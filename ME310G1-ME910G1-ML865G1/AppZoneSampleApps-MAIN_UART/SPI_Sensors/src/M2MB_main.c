/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing SPI usage, configuring two ST devices: a magnetometer \(ST LIS3MDL\) and a gyroscope \(ST L3G4200D\). The application will read values from both devices using GPIO4 and 3 \(respectively\) as magnetometer CS and gyro CS. Debug prints on MAIN UART
  @version 
    1.0.3
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    02/03/2017
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_spi.h"

#include "m2mb_gpio.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
#define SPI_BUF_LEN 8

/*
 * ST_LIS3MDL magnetometer and ST_L3G4200D gyroscope
 * REQUEST BYTE FORMAT:
 *  |RW|MS|ADDR|
 *  where
 *  RW: when 0, data in MOSI(7:0) is written into the device. when 1, data MISO(7:0) is read from the device.
 *  In the latter case, the device will drive MISO at the start of bit 8
 *  MS: when 0, the address will remain unchanged in multiple read/write commands. When 1, it is auto-incremented
 *  ADDR (5:0) the address of the indexed register
 */
#define ST_RW_BIT 1<<7
#define ST_MS_BIT 1<<6 //multiple send

//ST Magnetometer defines
#define ST_LIS3MDL_CS_PIN 2  //gpio 2
#define ST_LIS3MDL_WHOAMI 0x0F

#define ST_LIS3MDL_OUT_X_L 0x28
#define ST_LIS3MDL_OUT_X_H 0x29
#define ST_LIS3MDL_OUT_Y_L 0x2A
#define ST_LIS3MDL_OUT_Y_H 0x2B
#define ST_LIS3MDL_OUT_Z_L 0x2C
#define ST_LIS3MDL_OUT_Z_H 0x2D

#define ST_LIS3MDL_CTRL_REG1 0x20
#define ST_LIS3MDL_CTRL_REG2 0x21
#define ST_LIS3MDL_CTRL_REG3 0x22
#define ST_LIS3MDL_CTRL_REG4 0x23
#define ST_LIS3MDL_CTRL_REG5 0x24

//ST Gyroscope defines
#define ST_L3G4200D_CS_PIN 3  //gpio 3
#define ST_L3G4200D_WHOAMI 0x0F

#define ST_L3G4200D_OUT_X_L 0x28
#define ST_L3G4200D_OUT_X_H 0x29
#define ST_L3G4200D_OUT_Y_L 0x2A
#define ST_L3G4200D_OUT_Y_H 0x2B
#define ST_L3G4200D_OUT_Z_L 0x2C
#define ST_L3G4200D_OUT_Z_H 0x2D

#define ST_L3G4200D_CTRL_REG1 0x20
#define ST_L3G4200D_CTRL_REG2 0x21
#define ST_L3G4200D_CTRL_REG3 0x22
#define ST_L3G4200D_CTRL_REG4 0x23
#define ST_L3G4200D_CTRL_REG5 0x24



/*
 *  Connection scheme
  #####################################################################
                     | CS1467g-A       |  ST
  TX_AUX/MOSI        | PL303/1         | SDI
  RX_AUX/MISO         | PL303/2         | SDO
  SPI_CLK            | PL303/3         | SCK
  GPIO2              | PL302/2         | CS for Magnetometer
  GPIO3              | PL302/3         | CS for Gyroscope
  GND                | PL303/10        | BLACK
  V3.8               | PL101/9         | RED
  #####################################################################

  PL101/9 actually provides a 3.9xxV voltage, above the 3.8V max (according to specifications).
  USIF1 (aux UART) will be occupied since the pins are shared with SPI.
*/



/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
INT32 gpio_MAGN_CS_fd;
INT32 gpio_GYRO_CS_fd;



/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/

INT32 M2M_open_gpio_output(int pin)
{
  INT32 ret;
  INT32 gpio_fd;
  char path[16];

  memset(path, 0, sizeof(path));
  snprintf(path, sizeof(path), "/dev/GPIO%d", pin);

  gpio_fd = m2mb_gpio_open( path, 0 );
  if( gpio_fd != -1 )
  {
    /* SET GPIO as output */
    ret = m2mb_gpio_ioctl( gpio_fd, M2MB_GPIO_IOCTL_SET_DIR, M2MB_GPIO_MODE_OUTPUT );
    if ( ret == -1 )
    {
      AZX_LOG_CRITICAL("cannot set as output\r\n");
      return -1;
    }

    ret = m2mb_gpio_ioctl( gpio_fd, M2MB_GPIO_IOCTL_SET_DRIVE, M2MB_GPIO_LOW_DRIVE );
     if ( ret == -1 )
     {
       AZX_LOG_CRITICAL("cannot set medium drive\r\n");
       return -1;
     }

    ret = m2mb_gpio_ioctl( gpio_fd, M2MB_GPIO_IOCTL_SET_PULL, M2MB_GPIO_PULL_UP );
    if ( ret == -1 )
    {
      AZX_LOG_CRITICAL("cannot set pullup\r\n");
      return -1;
    }

    return gpio_fd;
  }
  else
  {
    AZX_LOG_CRITICAL("cannot open path\r\n");
    return -1;
  }

}


int M2M_gpio_set(INT32 fd, int value)
{
  if(fd == -1)
  {
    AZX_LOG_ERROR("fd invalid\r\n");
    return -1;
  }
  return m2mb_gpio_write( fd, (M2MB_GPIO_VALUE_E) value );

}


UINT16 axisValue(UINT8 msB, UINT8 lsB)
{
  return ( msB << 8) + (lsB);
}

float twoscompl(INT16 binaryValue)
{
  UINT8 isNegative;
  INT16 temp;
  float result;

  isNegative = (binaryValue & (1 << 15)) != 0; //if MSB == 1 -> negative number

  if (isNegative)
  {
    temp = binaryValue | ~((1 << 16) - 1);
    result = (float)(~temp + 1 ) * -4.0/32768;  //returns values with sign
  }
  else
    result = binaryValue * 4.0/32768;

  return result;
}




UINT8 m2m_read_register_byte(INT32 fd, INT32 cs_pin_fd, INT32 regaddr)
{
  INT32 ret_w = -1;
  UINT8 tx_buf[2];
  UINT8 rx_buf[2];

  memset(tx_buf, 0xFF, 2);
  memset(rx_buf, 0, 2);

  tx_buf[0] = regaddr | ST_RW_BIT; //add RW bit since this is a read request

  /*
   * ST REQUEST BYTE FORMAT:
   *  |RW|MS|ADDR|
   *  where
   *  RW: when 0, data in MOSI(7:0) is written into the device. when 1, data MISO(7:0) is read from the device.
   *  In the latter case, the device will drive MISO at the start of bit 8
   *  MS: when 0, the address will remain unchanged in multiple read/write commands. When 1, it is auto-incremented
   *  ADDR (5:0) the address of the indexed register
   */

  AZX_LOG_TRACE("Sending: %02X %02X \r\n",tx_buf[0], tx_buf[1] );

  M2M_gpio_set(cs_pin_fd,0); //set CS to output LOW
  ret_w = m2mb_spi_write_read(fd, tx_buf, rx_buf, 2);
  M2M_gpio_set(cs_pin_fd,1); //set CS to output HIGH

  if( (ret_w > 0) )
  {
    AZX_LOG_TRACE( "Test SPI ret_w = %d \r\n", ret_w );
    AZX_LOG_TRACE("SPI RAW recv: %02X %02X\r\n", rx_buf[0], rx_buf[1]);
  }


  return rx_buf[1];
}

UINT8 m2m_read_register_stream(INT32 fd, INT32 cs_pin_fd, INT32 regaddr, UINT8 *rx_buf, UINT8 bytes)
{
  int i;
  INT32 ret_w = -1;
  UINT8 tx_buf[SPI_BUF_LEN];
  UINT8 *int_rx_buf = NULL;

  memset(tx_buf, 0xFF, SPI_BUF_LEN);

  int_rx_buf = (UINT8*) m2mb_os_malloc(bytes + 1);
  if (! int_rx_buf)
  {
    AZX_LOG_ERROR("cannot allocate.\r\n");
    return 0;
  }
  memset(int_rx_buf, 0, bytes + 1);

  memset(rx_buf, 0, bytes);


  tx_buf[0] = regaddr | ST_RW_BIT | ST_MS_BIT; //add RW bit since this is a read request

  /*
   * ST REQUEST BYTE FORMAT:
   *  |RW|MS|ADDR|
   *  where
   *  RW: when 0, data in MOSI(7:0) is written into the device. when 1, data MISO(7:0) is read from the device.
   *  In the latter case, the device will drive MISO at the start of bit 8
   *  MS: when 0, the address will remain unchanged in multiple read/write commands. When 1, it is auto-incremented
   *  ADDR (5:0) the address of the indexed register
   */

  AZX_LOG_TRACE("Sending: %02X %02X \r\n",tx_buf[0], tx_buf[1] );

  M2M_gpio_set(cs_pin_fd,0); //set CS to output LOW
  ret_w = m2mb_spi_write_read(fd, tx_buf, int_rx_buf, bytes + 1);
  M2M_gpio_set(cs_pin_fd,1); //set CS to output HIGH

  if( (ret_w == bytes + 1) )
  {

    AZX_LOG_TRACE( "Test SPI ret_w = %d \r\n", ret_w );

    //AZX_LOG_DEBUG("raw data: ");
    for(i=0; i < ret_w; i++)
    {
      rx_buf[i] = int_rx_buf[i+1];
      //AZX_LOG_INFO("%02X ", int_rx_buf[i+1]);
    }
    m2mb_os_free(int_rx_buf);
    //AZX_LOG_INFO("SPI RAW recv: %02X %02X\r\n", rx_buf[0], rx_buf[1]);
    return ret_w - 1;
  }
  else
  {
    m2mb_os_free(int_rx_buf);
    return 0;
  }

}



UINT8 m2m_write_register_byte(INT32 fd, INT32 cs_pin_fd, INT32 regaddr, UINT8 val)
{
  INT32 ret_w = -1;
  UINT8 tx_buf[2];
  UINT8 rx_buf[2];

  memset(tx_buf, 0, 2);
  memset(rx_buf, 0, 2);
  //&= (~(1<< L))
  tx_buf[0] = regaddr & (~(ST_RW_BIT | ST_MS_BIT)); //remove RW bit since this is a write request
  tx_buf[1] = val;
  /*
   * ST REQUEST BYTE FORMAT:
   *  |RW|MS|ADDR|
   *  where
   *  RW: when 0, data in MOSI(7:0) is written into the device. when 1, data MISO(7:0) is read from the device.
   *  In the latter case, the device will drive MISO at the start of bit 8
   *  MS: when 0, the address will remain unchanged in multiple read/write commands. When 1, it is auto-incremented
   *  ADDR (5:0) the address of the indexed register
   */

  AZX_LOG_TRACE("Sending: %02X %02X \r\n",tx_buf[0], tx_buf[1] );

  M2M_gpio_set(cs_pin_fd,0); //set CS to output LOW
  ret_w = m2mb_spi_write_read(fd, tx_buf, rx_buf, 2);
  M2M_gpio_set(cs_pin_fd,1); //set CS to output HIGH

  if( (ret_w == 2) )
  {
    //AZX_LOG_INFO( "Test SPI ret_w = %d \r\n", ret_w );
    //AZX_LOG_INFO("SPI RAW recv: %02X %02X\r\n", rx_buf[0], rx_buf[1]);
    return 0; //success
  }
  else
  {
    AZX_LOG_ERROR("unexpected return: %d\r\n", ret_w);
    return ret_w;
  }
}


void spi_cb_fn( UINT32 status, void *callback_ctxt )
{
  (void)status;
  (void)callback_ctxt;
  AZX_LOG_INFO( "Test SPI   callback \r\n" );
}



//Magnetometer
int ST_LIS3MDL_SPI_demo(INT32 fd)
{
  INT32 i;
  INT32 ret = -1;

  UINT8 multi_byte_buffer[SPI_BUF_LEN];
  UINT8 reg_data;

  UINT16 xAxis16bit;
  UINT16 yAxis16bit;
  UINT16 zAxis16bit;


  AZX_LOG_INFO( "\r\nMagnetometer SPI Demo start\r\n");


  ///   READ  ST Magnetometer WHOAMI
  AZX_LOG_INFO("Reading Magnetometer WHOAMI. Expected: 0x3D\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_WHOAMI);
  if (reg_data == 0x3D)
  {
    AZX_LOG_INFO("Expected response received!\r\n");
  }
  else
  {
    AZX_LOG_ERROR("unexpected response! %02X\r\n", reg_data);
  }

  AZX_LOG_TRACE("Magnetometer CTRL_REG3 at boot..\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG3);

  AZX_LOG_TRACE("Response: %02X\r\n", reg_data);


  AZX_LOG_INFO("Setting continuous conversion mode...\r\n");
  ret = m2m_write_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG3, 0x00); //continuous conversion mode
  if (ret != 0)
  {
    AZX_LOG_ERROR("failed setting register!\r\n");
    return -1;
  }

  AZX_LOG_TRACE("Reading again Magnetometer CTRL_REG3.\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG3);
  if (reg_data == 0x00)
  {
    AZX_LOG_INFO("Continuous conversion mode successfully set.\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Failed register configuration, expected 0x00, received %02X\r\n", reg_data);
    return -1;
  }

  AZX_LOG_TRACE("Magnetometer CTRL_REG1. at boot...\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG1);

  AZX_LOG_TRACE("Response: %02X\r\n", reg_data);

  AZX_LOG_INFO("Setting 10 Hz Output Data Rate, Medium performance mode X Y axis...\r\n");
  ret = m2m_write_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG1, 0x30); //10 Hz Output Data Rate, Medium performance mode X Y axis
  if (ret != 0)
  {
    AZX_LOG_ERROR("failed setting register!\r\n");
    return -1;
  }
  AZX_LOG_TRACE("Reading again Magnetometer CTRL_REG1.\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG1);
  if (reg_data == 0x30)
  {
    AZX_LOG_INFO("Magnetometer Enabled. 10Hz ODR, Medium Perf. Mode (X,Y).\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Failed register configuration, expected 0x30, received %02X\r\n", reg_data);
    return -1;
  }

  AZX_LOG_TRACE("Magnetometer CTRL_REG4. at boot...\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG4);

  AZX_LOG_TRACE("Response: %02X\r\n", reg_data);

  AZX_LOG_INFO("Setting Medium performance for Z axis, little endian...\r\n");
  ret = m2m_write_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG4, 0x04); //10 Hz Output Data Rate, Medium performance mode X Y axis
  if (ret != 0)
  {
    AZX_LOG_ERROR("failed setting register!\r\n");
    return -1;
  }
  AZX_LOG_TRACE("Reading again Magnetometer CTRL_REG1.\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_CTRL_REG4);
  if (reg_data == 0x04)
  {
    AZX_LOG_INFO("Medium Perf. Mode (Z), little endian.\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Failed register configuration, expected 0x04, received %02X\r\n", reg_data);
    return -1;
  }


  AZX_LOG_INFO("Setting complete, starting reading loop...\r\n");

  for (i=0; i < 10; i++)
  {

    memset(multi_byte_buffer, 0, SPI_BUF_LEN);

    ret = m2m_read_register_stream(fd, gpio_MAGN_CS_fd, ST_LIS3MDL_OUT_X_L, multi_byte_buffer, 6);  //reading 6 bytes
    if ( ret != 6 )
    {
      AZX_LOG_ERROR("Reading FAIL! - Exit Value: %d.\r\n", ret );
      return -1;
    }

    xAxis16bit = axisValue( multi_byte_buffer[1], multi_byte_buffer[0] );  //msb, lsb
    yAxis16bit = axisValue( multi_byte_buffer[3], multi_byte_buffer[2] );
    zAxis16bit = axisValue( multi_byte_buffer[5], multi_byte_buffer[4] );

    AZX_LOG_INFO( "\r\nX: %.3f gauss\r\nY: %.3f gauss\r\nZ: %.3f gauss\r\n", twoscompl(xAxis16bit), twoscompl(yAxis16bit), twoscompl(zAxis16bit) );

    azx_sleep_ms(2000);


  }

  AZX_LOG_INFO( "Reading complete.\r\n" );
  return 0;
}


//Gyroscope

int ST_L3G4200D_SPI_demo(INT32 fd, int scale)
{
  INT32 i;
  INT32 ret = -1;

  UINT8 multi_byte_buffer[SPI_BUF_LEN];
  UINT8 reg_data;

  UINT16 xAxis16bit;
  UINT16 yAxis16bit;
  UINT16 zAxis16bit;


  AZX_LOG_INFO( "\r\nGyroscope SPI Demo start\r\n");


  ///   READ  ST gyroscope WHOAMI
  AZX_LOG_INFO("Reading gyroscope WHOAMI. Expected: 0xD3\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_WHOAMI);
  if (reg_data == 0xD3)
  {
    AZX_LOG_INFO("Expected response received!\r\n");
  }
  else
  {
    AZX_LOG_ERROR("unexpected response! %02X\r\n", reg_data);
  }

  AZX_LOG_TRACE("Gyroscope CTRL_REG1 at boot...\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG1);

  AZX_LOG_TRACE("Response: %02X\r\n", reg_data);

  AZX_LOG_INFO("Setting 100 Hz Output Data Rate, enable X Y Z axes, disable power down mode...\r\n");
  ret = m2m_write_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG1, 0x0F);
  if (ret != 0)
  {
    AZX_LOG_ERROR("failed setting register!\r\n");
    return -1;
  }
  AZX_LOG_TRACE("Reading again Gyroscope CTRL_REG1.\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG1);
  if (reg_data == 0x0F)
  {
    AZX_LOG_INFO("Gyroscope Enabled. 100Hz ODR 12.5 Cut-Off.\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Failed register configuration, expected 0x0F, received %02X\r\n", reg_data);
    return -1;
  }



  AZX_LOG_TRACE("Gyroscope CTRL_REG4 at boot...\r\n");
  reg_data = m2m_read_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG4);

  AZX_LOG_TRACE("Response: %02X\r\n", reg_data);

  if (scale == 250)
  {
    AZX_LOG_INFO("Setting continuous update, little endian, 250 dps (degrees per second)..\r\n");
    ret = m2m_write_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG4, 0x00);
    if (ret != 0)
    {
      AZX_LOG_ERROR("failed setting register!\r\n");
      return -1;
    }
    AZX_LOG_TRACE("Reading again Gyroscope CTRL_REG4.\r\n");
    reg_data = m2m_read_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG4);
    if (reg_data == 0x00)
    {
      AZX_LOG_INFO("Continuous update, little endian, 250 dps.\r\n");
    }
    else
    {
      AZX_LOG_ERROR("Failed register configuration, expected 0x00, received %02X\r\n", reg_data);
      return -1;
    }
  }
  else if (scale == 500)
  {
    AZX_LOG_INFO("Setting continuous update, little endian, 500 dps (degrees per second)...\r\n");
    ret = m2m_write_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG4, 0x10);
    if (ret != 0)
    {
      AZX_LOG_ERROR("failed setting register!\r\n");
      return -1;
    }
    AZX_LOG_TRACE("Reading again Magnetometer CTRL_REG4.\r\n");
    reg_data = m2m_read_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG4);
    if (reg_data == 0x10)
    {
      AZX_LOG_INFO("Continuous update, little endian, 500 dps.\r\n");
    }
    else
    {
      AZX_LOG_ERROR("Failed register configuration, expected 0x10, received %02X\r\n", reg_data);
      return -1;
    }
  }
  else
  {
    AZX_LOG_INFO("Setting continuous update, little endian, 2000 dps (degrees per second)...\r\n");
    ret = m2m_write_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG4, 0x30);
    if (ret != 0)
    {
      AZX_LOG_ERROR("failed setting register!\r\n");
      return -1;
    }
    AZX_LOG_TRACE("Reading again Magnetometer CTRL_REG4.\r\n");
    reg_data = m2m_read_register_byte(fd, gpio_GYRO_CS_fd, ST_L3G4200D_CTRL_REG4);
    if (reg_data == 0x30)
    {
      AZX_LOG_INFO("Continuous update, little endian, 2000 dps.\r\n");
    }
    else
    {
      AZX_LOG_ERROR("Failed register configuration, expected 0x30, received %02X\r\n", reg_data);
      return -1;
    }
  }


  AZX_LOG_INFO("Setting complete, starting reading loop...\r\n");

  for (i=0; i < 10; i++)
  {

    memset(multi_byte_buffer, 0, SPI_BUF_LEN);

    ret = m2m_read_register_stream(fd, gpio_GYRO_CS_fd, ST_L3G4200D_OUT_X_L, multi_byte_buffer, 6);  //reading 6 bytes
    if ( ret != 6 )
    {
      AZX_LOG_ERROR("Reading FAIL! - Exit Value: %d.\r\n", ret );
      return -1;
    }

    xAxis16bit = axisValue( multi_byte_buffer[1], multi_byte_buffer[0] );  //msb, lsb
    yAxis16bit = axisValue( multi_byte_buffer[3], multi_byte_buffer[2] );
    zAxis16bit = axisValue( multi_byte_buffer[5], multi_byte_buffer[4] );

    AZX_LOG_INFO( "\r\nX: %.3f degrees per second\r\nY: %.3f degrees per second\r\nZ: %.3f degrees per second\r\n", twoscompl(xAxis16bit), twoscompl(yAxis16bit), twoscompl(zAxis16bit) );

    azx_sleep_ms(500);


  }

  AZX_LOG_INFO( "Reading complete.\r\n" );
  return 0;
}

int SPI_demo(void)
{

  INT32 fd;
  INT32 ret = -1;

  M2MB_SPI_CFG_T cfg;

  UINT8 userdata[3];
  //char dev_name[32];

  userdata[0] = 0x0F;


  cfg.spi_mode = M2MB_SPI_MODE_0; //clock idle LOW, data driven on falling edge and sampled on rising edge
  cfg.cs_polarity = M2MB_SPI_CS_ACTIVE_LOW;
  cfg.cs_mode = M2MB_SPI_CS_DEASSERT;
  cfg.endianness = M2MB_SPI_NATIVE; //M2MB_SPI_LITTLE_ENDIAN; //M2MB_SPI_BIG_ENDIAN;
  cfg.callback_fn = NULL;
  cfg.callback_ctxt = (void *)userdata; //NULL;
  cfg.clk_freq_Hz = 1000000;
  cfg.bits_per_word = 8;
  cfg.cs_clk_delay_cycles = 3;
  cfg.inter_word_delay_cycles = 0;
  cfg.loopback_mode = FALSE;


  AZX_LOG_INFO( "SPI start\r\n");

  gpio_MAGN_CS_fd = M2M_open_gpio_output(ST_LIS3MDL_CS_PIN);
  gpio_GYRO_CS_fd = M2M_open_gpio_output(ST_L3G4200D_CS_PIN);

  if (gpio_MAGN_CS_fd == -1 || gpio_GYRO_CS_fd == -1)
  {
    AZX_LOG_ERROR("cannot set GPIO descriptors!\r\n");
    return -1;
  }

  M2M_gpio_set(gpio_MAGN_CS_fd,1); //set magnetometer CS to output HIGH
  M2M_gpio_set(gpio_GYRO_CS_fd,1); //set giroscope CS to output HIGH

  fd = m2mb_spi_open("/dev/spidev5.0", 0);
  if(fd != -1)
  {
    ret = m2mb_spi_ioctl(fd, M2MB_SPI_IOCTL_SET_CFG, (void *)&cfg);
    if(ret == 0)
    {
      //Magnetometer
      ST_LIS3MDL_SPI_demo(fd);

      //Gyroscope
      ST_L3G4200D_SPI_demo(fd, 2000);
    }

    ret = m2mb_spi_close(fd);
    if(ret != 0)
    {
      AZX_LOG_ERROR( "Test SPI   m2mb_spi_close FAIL   ret = %d \r\n", ret );
    }

  }
  else
  {
    AZX_LOG_ERROR("cannot open SPI descriptor!\r\n");
  }
  return 0;
}



/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;

  azx_sleep_ms(5000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting SPI demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  SPI_demo();

}

