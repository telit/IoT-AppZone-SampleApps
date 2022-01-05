/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to communicate with an I2C slave device with I2C raw mode. Debug prints on MAIN UART
  @version 
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    05/05/2020
 */

/*
 * This Application shows how to use I2C raw mode APIs. a Kionix KXTF9-4100 accelerometer is used in this example
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



/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/**
 * @brief Compute the value of one accelerometer's axis from the 2 registers' value
 *
 * @param[in]   msB the most significant byte value
 * @param[in]   lsB the least significant byte value
 * @return the axis reading value
 */
UINT16 axisValue(UINT8 msB, UINT8 lsB)
{
  return ( msB << 4) + (lsB >> 4);
}



/**
 * @brief Perform a two's complement operation on the input number
 *
 * @param[in]   bynaryValue the input value to be complemented  *
 * @return the floating point conversion of the input parameter
 */
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



/**
 * @brief Perform a single READ operation on I2C bus (no data is written)
 *
 * This function will try perform a read operation on the I2C bus. the sequence will be
 * Start Sequence - Slave Address with Read bit flag - data input - Stop Sequence
 *
 *
 * @param[in]   fd The already opened and configured file descriptor for the I2C device
 * @param[out]  in_buf the buffer where the incoming data will be stored
 * @param[in]   bytesToRead the amount of bytes to be read from I2C Bus
 *
 * @return Number of read bytes, -1 in case of failure
 */
INT32 read_only_I2C(INT32 fd, UINT8 *in_buf, UINT8 bytesToRead)
{
  INT32 i2c_res;
  M2MB_I2C_CFG_T i2c_data = {0};

  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2]; /* one message for write, one for read */

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&i2c_data);
  if (i2c_res != 0)
  {
    AZX_LOG_ERROR("cannot get I2C channel configuration\r\n");
    return -1;
  }

  msgs[0].flags = I2C_M_WR;
  msgs[0].buf = NULL;
  msgs[0].len = 0;

  memset( in_buf, 0x00, bytesToRead );

  msgs[1].flags = I2C_M_RD;   /* Set read flag*/
  msgs[1].len   = bytesToRead;  /* Read 1 bytes from register (whose address was written with msgs[0] )*/
  msgs[1].buf   = in_buf;       /* Assign read buffer to message struct [1]*/


  /* Set i2c data struct rw parameters messages pointer to msgs */
  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 1; /* both write and read, so size is 2 x message structs */

  /* Assign rw param address to cfg struct */
  i2c_data.rw_param = &rdrw_data;

  AZX_LOG_DEBUG("Trying to read %u bytes\r\n", i2c_data.rw_param->msgs[1].len);

  i2c_res =  m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_RDWR, (void *)&i2c_data);
  if (i2c_res != bytesToRead) /*expected read bytes*/
  {
    AZX_LOG_ERROR("Cannot perform read on i2c channel! %d\r\n", i2c_res);
  }

  return i2c_res;
}



/**
 * @brief Perform a single READ operation on I2C bus from the specified register address
 *
 * This function will try perform a read operation on the I2C bus. The sequence will be
 * Start Sequence - Slave Address with Write bit flag - register address is written -
 *    Repeated Start Sequence - Slave Address with Read bit flag - data input - Stop Sequence
 *
 *
 * @param[in]   fd The already opened and configured file descriptor for the I2C device
 * @param[in]   regAddr The register address from where data will be read
 * @param[out]  in_buf the buffer where the incoming data will be stored
 * @param[in]   bytesToRead the amount of bytes to be read from I2C Bus
 *
 * @return Number of read bytes, -1 in case of failure
 */
INT32 readI2CData(INT32 fd, UINT8 regAddr, UINT8 *in_buf, UINT8 bytesToRead)
{
  INT32 i2c_res;
  UINT8 buf_wr[4] = {0}; /*To write register address*/

  M2MB_I2C_CFG_T i2c_data = {0};

  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[2]; /* one message for write, one for read */

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&i2c_data);
  if (i2c_res != 0)
  {
    AZX_LOG_ERROR("cannot get I2C channel configuration\r\n");
    return -1;
  }

  i2c_data.registerId = regAddr;

  memset( buf_wr, 0x00, sizeof(buf_wr) );
  memset( in_buf, 0x00, bytesToRead );

  buf_wr[0] = i2c_data.registerId;  /* the only byte to be written will be register id */

  msgs[0].flags = I2C_M_WR;   /* Set write flag*/
  msgs[0].len   = 1;            /* How many bytes to be written: register address only*/
  msgs[0].buf   = buf_wr;       /* Assign write buffer to message struct [0]*/

  msgs[1].flags = I2C_M_RD;   /* Set read flag*/
  msgs[1].len   = bytesToRead;  /* Read 1 bytes from register (whose address was written with msgs[0] )*/
  msgs[1].buf   = in_buf;       /* Assign read buffer to message struct [1]*/



  /* Set i2c data struct rw parameters messages pointer to msgs */
  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 2; /* both write and read, so size is 2 x message structs */

  /* Assign rw param address to cfg struct */
  i2c_data.rw_param = &rdrw_data;

  i2c_res =  m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_RDWR, (void *)&i2c_data);
  if (i2c_res != bytesToRead) /*expected read bytes*/
  {
    AZX_LOG_ERROR("cannot perform read on i2c channel! %d\r\n", i2c_res);
  }

  return i2c_res;

}



/**
 * @brief Configure a register on I2C slave device
 *
 * This function will perform a write operation on I2C bus. The sequence will be
 * Start Sequence - Slave Address with Write bit flag - register address is written -
 *    data output - Stop Sequence
 * The function will also call a read to verify if the required value has been properly
 * stored in the device.
 *
 *
 * @param[in]   fd The already opened and configured file descriptor for the I2C device
 * @param[in]   regAddr The register address where data will be stored
 * @param[in]   regName The string with register name (for debu purposes)
 * @param[in]   bytesToWrite the amount of bytes to be written on I2C Bus
 * @param[in]   message the debug message to be printed in case of success.
 *
 * @return 0 in case of success, -1 in case of failure
 */
INT32 configI2CRegister(INT32 fd, UINT8 regAddr, const char* regName, UINT8 byteToWrite, const char* message)
{
  INT32 i2c_res;
  UINT8 i2cbuf_wr[4];
  UINT8 i2cbuf_rd[4];

  M2MB_I2C_CFG_T i2c_data = {0};

  M2MB_I2C_RDWR_IOCTL_DATA rdrw_data = {0};
  M2MB_I2C_MSG msgs[1];

  /* Retrieve current channel config */
  i2c_res = m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_GET_CFG, (void *)&i2c_data);
  if (i2c_res != 0)
  {
    AZX_LOG_ERROR("Cannot get I2C channel configuration\r\n");
    return -1;
  }

  i2c_data.registerId = regAddr;

  memset( i2cbuf_wr, 0x00, sizeof(i2cbuf_wr) );
  memset( i2cbuf_rd, 0x00, sizeof(i2cbuf_rd) );


  /*First, configure the write buffer with content: */
  i2cbuf_wr[0] = i2c_data.registerId;  /* first byte to be written will be register id */
  i2cbuf_wr[1] = byteToWrite;          /* then data to write in register */


  msgs[0].flags = I2C_M_WR; /* Set write flag*/
  msgs[0].len   = 2;          /* How many bytes to be written: register + data*/
  msgs[0].buf   = i2cbuf_wr;  /* Assign write buffer to message struct [0]*/



  /* Set i2c data struct rw parameters messages pointer to msgs */
  rdrw_data.msgs = &msgs[0];
  rdrw_data.nmsgs = 1;       /* writing only: 1 message struct size */

  /* Assign rw param address to cfg struct */
  i2c_data.rw_param = &rdrw_data;

  i2c_res =  m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_RDWR, (void *)&i2c_data);
  if (i2c_res != msgs[0].len) /* expected msgs[0].len written bytes */
  {
    AZX_LOG_ERROR("Cannot perform Write on i2c channel! Received %d, expected %d\r\n", i2c_res, msgs[0].len);
    return i2c_res;
  }


  /*Check if register current value is the expected*/
  readI2CData(fd, regAddr, i2cbuf_rd, 1);

  if (i2cbuf_rd[0] == byteToWrite)
  {
    AZX_LOG_INFO("%s\r\n", message);
  }
  else
  {
    AZX_LOG_ERROR("%s: Received 0x%02X, Expected 0x%02X !!\r\n", regName, i2cbuf_rd[0], byteToWrite);
  }
  return 0;
}


int run_I2CRawDemo(void)
{

  INT32 res;
  INT32 fd = 0;
  int i;
  UINT8 i2cbuf_rd[32];
  CHAR dev_ID[64];
  M2MB_I2C_CFG_T config;

  UINT16 xAxis12bit, yAxis12bit, zAxis12bit;

  /**************
   *  Configuring the IIC device.
   **************/
  AZX_LOG_INFO( "\nConfiguring the I2C device...\r\n" );

  //Create device name using device address in decimal base, left shifted by 1 bit
  sprintf(dev_ID, "/dev/I2C-%d", DEVICE_ADDR);
  AZX_LOG_INFO("Opening I2C channel %s ( device address is 0x%02X << 1 )\r\n", dev_ID,  DEVICE_ADDR >> 1);

  fd = m2mb_i2c_open( dev_ID, 0 );

  if (-1 == fd)
  {
    AZX_LOG_ERROR("cannot open I2C channel!\r\n");
    return -1;
  }

  config.sclPin = I2C_SCL;
  config.sdaPin = I2C_SDA;

  config.registerId = 0x00; /*placeholder*/


  if (m2mb_i2c_ioctl(fd, M2MB_I2C_IOCTL_SET_CFG, (void *)&config) != 0)
  {
    AZX_LOG_ERROR("cannot set I2C channel configuration\r\n");
    return -1;
  }


  /*Read Device WhoAmI register*/
  res = readI2CData(fd, KX_WHOAMI, i2cbuf_rd, 1);
  if (res != 1)
  {
    AZX_LOG_ERROR("cannot read data! error: %d\r\n", res);
    return -1;
  }
  if (i2cbuf_rd[0] == 0x01)
  {
    AZX_LOG_DEBUG("\nWHOAMI content correct.\r\n");
  }
  else
  {
    AZX_LOG_ERROR("\nWHOAMI content: 0x%02X - ERROR!: Expected 0x01\r\n",i2cbuf_rd[0]);
  }

  /**********
   *  CONFIGURATION PROCEDURE
   **********/

  /* CTRL_REG3 */
  res = configI2CRegister(fd, KX_CTRL_REG3, "CTRL_REG3", 0x4D,
      "Accelerometer Enabled. ODR tilt: 12.5Hz, ODR directional tap: 400Hz, ORD Motion Wakeup: 50Hz");
  if (res != 0)
  {
    return -1;
  }


  /* CTRL_REG1 */
  res = configI2CRegister(fd, KX_CTRL_REG1, "CTRL_REG1", 0xC0,
      "Accelerometer Enabled. Operative mode, 12bit resolution" );
  if (res != 0)
  {
    return -1;
  }
  

  /***************
   *  READING AXES VALUES
   **************/
  AZX_LOG_INFO( "I2C read axes registers\r\n"
      "------------\r\n");
  for(i=0; i<20; i++)
  {
    memset(i2cbuf_rd, 0, 6);

    /*Read Device Axes registers*/
    res = readI2CData(fd, KX_X_OUT_LSB, i2cbuf_rd, 6); /*Read 6 bytes: 6 registers in a row*/
    if ( res != 6 )
    {
      AZX_LOG_ERROR("Reading FAIL! - Exit Value: %d.\r\n", res );
      return -1;
    }

    xAxis12bit = axisValue( i2cbuf_rd[1], i2cbuf_rd[0] ); /* MSB, LSB */
    yAxis12bit = axisValue( i2cbuf_rd[3], i2cbuf_rd[2] ); /* MSB, LSB */
    zAxis12bit = axisValue( i2cbuf_rd[5], i2cbuf_rd[4] ); /* MSB, LSB */

    AZX_LOG_INFO( "\r\nX: %.3f g\r\nY: %.3f g\r\nZ: %.3f g\r\n",
        twoscompl(xAxis12bit), /*Convert raw data in readable values*/
        twoscompl(yAxis12bit), /*Convert raw data in readable values*/
        twoscompl(zAxis12bit)  /*Convert raw data in readable values*/
    );
    azx_sleep_ms(2000);
  }

   return 0;
}
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


  azx_sleep_ms(2000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting I2C raw demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  run_I2CRawDemo();



}

