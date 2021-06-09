/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to communicate with an I2C slave device. Debug prints on USB0
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    05/09/2018
*/

/*
 * This Application shows how to use I2C APIs. a Kionix KXTF9-4100 accelerometer is used in this example
 *
 * */

/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_i2c.h"


#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/*
 * Kionix device
 */
#define I2C_SDA       (UINT8) 2
#define I2C_SCL       (UINT8) 3
#define KX_I2C_ADDR      (UINT16) 0x0F

#define DEVICE_ADDR     (UINT16) KX_I2C_ADDR << 1

#define KX_WHOAMI        (UINT8) 0x0F
#define KX_CTRL_REG1     (UINT8) 0x1B
#define KX_CTRL_REG3  (UINT8) 0x1D  //4D     01001101

#define KX_X_OUT_LSB   (UINT8) 0x06
#define KX_X_OUT_MSB   (UINT8) 0x07
#define KX_Y_OUT_LSB   (UINT8) 0x08
#define KX_Y_OUT_MSB   (UINT8) 0x09
#define KX_Z_OUT_LSB   (UINT8) 0x0A
#define KX_Z_OUT_MSB   (UINT8) 0x0B


void configRegister(INT32 fd, UINT8 regAddr, const char* regName, UINT8 byteToWrite, char* message)
{
  INT32 i2c_res;
  UINT8 buf[1];

  M2MB_I2C_CFG_T config;

  i2c_res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&config);
  if (i2c_res != 0)
  {
    AZX_LOG_ERROR("cannot get I2C channel configuration\r\n");
    return;
  }

  config.registerId = regAddr;

  i2c_res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_SET_CFG, (void *)&config);
  if (i2c_res != 0)
  {
    AZX_LOG_ERROR("cannot set I2C channel configuration\r\n");
    return;
  }


  buf[0]= byteToWrite;

  AZX_LOG_INFO( "Configuring I2C Registers - Writing 0x%02X into 0x%02X register (%s)...\r\n", (UINT8) *buf, regAddr, regName);

   //Write 1 byte from WHOAMI register
  i2c_res = m2mb_i2c_write(fd, buf, 1);
  if (i2c_res != 1)
  {
    AZX_LOG_ERROR("cannot write data! error: %d\r\n", i2c_res);
    return;
  }
  else
  {
    AZX_LOG_INFO("Write: success\r\n");
  }

  buf[0] = 0;

  AZX_LOG_INFO( "\nI2C reading data from 0x%02X register (%s)...\r\n", regAddr, regName );

  //Read 1 byte from register
  i2c_res = m2mb_i2c_read(fd, buf, 1);
  if (i2c_res != 1)
  {
    AZX_LOG_ERROR("cannot read data! error: %d\r\n", i2c_res);
    return;
  }
  else
  {
    AZX_LOG_INFO("Read: success.\r\n");
  }

  if (buf[0] == byteToWrite)
  {
    AZX_LOG_INFO("%s\r\n", message);
  }
  else
  {
    AZX_LOG_ERROR("%s: 0x%02X - ERROR!: Expected 0x%02X\r\n",regName, buf[0], byteToWrite);
  }

}


UINT16 axisValue(UINT8 msB, UINT8 lsB)
{
  return ( msB << 4) + (lsB >> 4);
}


float twoscompl(INT16 binaryValue)
{
  UINT8 isNegative;
  INT16 temp;
  float result;

  isNegative = (binaryValue & (1 << 11)) != 0; //if MSB == 1 -> negative number

  if (isNegative)
  {
      temp = binaryValue | ~((1 << 12) - 1);
      result = (float)(~temp + 1 ) * -1999/2047000.0;  //returns values with sign
  }
  else
      result = binaryValue * 1999/2047000.0;

  return result;
}




int test_I2C(void)
{
  INT32 res;
  INT32 fd;
  UINT8 buf[256];
  CHAR dev_ID[64];
  UINT8 i;

  M2MB_I2C_CFG_T config;


  UINT16 xAxis12bit, yAxis12bit, zAxis12bit;
  /**************
   *  Configuring the IIC device.
   **************/
  AZX_LOG_INFO( "\nConfiguring the Kionix device...\r\n" );

  //Create device name using device address in decimal base, left shifted by 1 bit
  sprintf(dev_ID, "/dev/I2C-%d", DEVICE_ADDR);
  AZX_LOG_INFO("opening channel %s\r\n", dev_ID);

  fd = m2mb_i2c_open( dev_ID, 0 );

  if (-1 == fd)
  {
    AZX_LOG_ERROR("cannot open I2C channel!\r\n");
    return -1;
  }

  config.sclPin = I2C_SCL;
  config.sdaPin = I2C_SDA;

  config.registerId = KX_WHOAMI;

  res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_SET_CFG, (void *)&config);
  if (res != 0)
  {
    AZX_LOG_ERROR("cannot configure I2C channel\r\n");
    return -1;
  }

  //Read 1 byte from WHOAMI register
  res = m2mb_i2c_read(fd, buf, 1);
  if (res != 1)
  {
    AZX_LOG_ERROR("cannot read data! error: %d\r\n", res);
    return -1;
  }
  if (buf[0] == 0x01)
  {
    AZX_LOG_DEBUG("\nWHOAMI content: 0x01\r\n");
  }
  else
  {
    AZX_LOG_ERROR("\nWHOAMI content: 0x%02X - ERROR!: Expected 0x01\r\n",buf[0]);
  }


  /**********
   *  WRITING PROCEDURE
   **********/

  /* CTRL_REG3 */

  configRegister( fd, KX_CTRL_REG3, "CTRL_REG3", 0x4D, (char*) "Accelerometer Enabled. ODR tilt: 12.5Hz, ODR directional tap: 400Hz, ORD Motion Wakeup: 50Hz" );

  /* CTRL_REG1 */

  configRegister( fd, KX_CTRL_REG1, "CTRL_REG1", 0xC0, (char*) "Accelerometer Enabled. Operative mode, 12bit resolution" );


  /***************
    *  READING AXES VALUES
    **************/


   AZX_LOG_INFO( "I2C read axes registers\r\n" );
   AZX_LOG_INFO( "------------\r\n" );

   res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&config);
   if (res != 0)
   {
     AZX_LOG_ERROR("Cannot get I2C channel configuration\r\n");
     return -1;
   }

  config.registerId = KX_X_OUT_LSB;

  res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_SET_CFG, (void *)&config);
  if (res != 0)
  {
    AZX_LOG_ERROR("Cannot set I2C channel configuration\r\n");
    return -1;
  }

   for(i=0; i<100; i++)
   {
       memset(buf, 0, 6);
       res = m2mb_i2c_read(fd, buf, 6);  //reading 6 bytes
      if ( res == 6 )
      {
        AZX_LOG_INFO("Reading Success.\r\n");
      }
      else
      {
        AZX_LOG_ERROR("Reading FAIL! - Exit Value: %d.\r\n",res );
        return -1;
      }
      xAxis12bit = axisValue( buf[1], buf[0] );  //msb, lsb
      yAxis12bit = axisValue( buf[3], buf[2] );
      zAxis12bit = axisValue( buf[5], buf[4] );

      AZX_LOG_INFO( "\r\nX: %.3f g\r\nY: %.3f g\r\nZ: %.3f g\r\n", twoscompl(xAxis12bit), twoscompl(yAxis12bit), twoscompl(zAxis12bit) );

      azx_sleep_ms(2000);
   }

   return 0;
}



/* Static functions =============================================================================*/
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
  AZX_LOG_INFO("Starting I2C demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);
        

  test_I2C();

}

