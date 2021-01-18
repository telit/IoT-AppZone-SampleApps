/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#ifndef HDR_EEPROM_I2C_UTILS_H_
#define HDR_EEPROM_I2C_UTILS_H_

/**
  @file
    i2c_utils.h

  @brief
    i2c utilities for EEPROM usage
  @details
    this file defines utility functions to be used with EEPROM driver
  @note
    Dependencies:
    m2mb_types.h

  @author
		FabioPi
  @date
    28/10/2020
 */

/* Global declarations ==========================================================================*/
/* Global typedefs ==============================================================================*/

typedef enum
{
  EEPROM_I2C_RES_OK = 0,
  EEPROM_I2C_RES_CANNOT_INIT,
  EEPROM_I2C_RES_INTERNAL_ERROR,
  EEPROM_I2C_RES_INVALID_HANDLE,
  EEPROM_I2C_RES_WRITE_ERR,
  EEPROM_I2C_RES_READ_ERR,
  EEPROM_I2CRES_CANNOT_ALLOCATE,
  EEPROM_I2C_RES_NOT_INIT
} EEPROM_I2C_RES_E;

typedef struct
{
  UINT8 sda_pin;
  UINT8 scl_pin;
  INT32 dev_addr;
  INT32 fd;
  UINT8 init;
} EEPROM_I2C_T;


/* Global functions =============================================================================*/
INT32 EEPROM_I2C_init(EEPROM_I2C_T* h);
INT32 EEPROM_I2C_isInit(EEPROM_I2C_T* h);
INT32 EEPROM_I2C_deinit(EEPROM_I2C_T* h);

INT32 EEPROM_i2c_write(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData, UINT16 size);
INT32 EEPROM_i2c_write_byte(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 data);

INT32 EEPROM_i2c_read(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData, UINT16 size);
INT32 EEPROM_i2c_read_delayed_chunked(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData, UINT16 size, UINT16 chunksize, UINT16 delay);
INT32 EEPROM_i2c_read_byte_from_address(EEPROM_I2C_T *hi2c, UINT16 memAddress, UINT8 *pData);
INT32 EEPROM_i2c_read_byte(EEPROM_I2C_T *hi2c, UINT8 *pData);

#endif /* HDR_EEPROM_I2C_UTILS_H_ */
