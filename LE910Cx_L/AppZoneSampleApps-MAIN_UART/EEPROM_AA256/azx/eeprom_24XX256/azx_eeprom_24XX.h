/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

#ifndef HDR_AZX_EEPROM_24XX_H_
#define HDR_AZX_EEPROM_24XX_H_

/**
  @file azx_eeprom_24XX.h

  @brief EEPROM communication functions

  This file defines EEPROM related functions (read, write, erase) for Microchip 24XX256 EEPROM memories.

  Supports any memory address (refer to AZX_EEPROM_24XX256T_SLAVE_ADDR macro)

  Supported read modes:
  1) Current address read byte
  2) Random read (byte or stream)
  3) Sequential read (byte or stream)

  Refer to https://ww1.microchip.com/downloads/en/DeviceDoc/24AA256-24LC256-24FC256-Data-Sheet-20001203W.pdf for further details

  @version 1.0.3
  @dependencies azx_log

  @note
    Dependencies:
    m2mb_types.h
    i2c_utils.h

  @author
    Fabio Pintus
   
  @date
    29/10/2020
 */

/** @defgroup azx_eeprom EEPROM utilities allowing communication with EEPROM chips of 24xx256 family.
 */


/* Global declarations ==========================================================================*/
/**  @{ */
/** \ingroup azx_eeprom
 */
/**< EEPROM control code (1010 by default)*/
#define AZX_EEPROM_24XX256T_CONTROL_CODE 0x50

/**< 24XX256 EEPROM total size in bytes */
#define AZX_EEPROM_24XX256T_MEM_SIZE     32768

/**< 24XX256 EEPROM page size in bytes */
#define AZX_EEPROM_24XX256T_PAGE_SIZE    64



/**/
/**
 * @brief creates the slave address
 *
 * Given the three chip select bits, create the final slave address for the eeprom chip.
 *
 * @param a2__ most significant chip select bit (pin 3 on EEPROM Chip)
 * @param a1__ middle chip select bit (pin 2 on EEPROM Chip)
 * @param a0__ least significant chip select bit (pin 1 on EEPROM Chip)
 *
 * @return the slave address
 */
#define AZX_EEPROM_24XX256T_SLAVE_ADDR(a2__, a1__, a0__)     (UINT16) ((AZX_EEPROM_24XX256T_CONTROL_CODE + ((a2__ << 2) + (a1__ << 1) + (a0__) )) << 1)

/** @} */

/* Global typedefs ==============================================================================*/



/**
 * @brief return codes
 * \ingroup azx_eeprom
 */
typedef enum
{
  AZX_EEPROM_RES_OK = 0,              /**< Success*/                                                //!< AZX_EEPROM_RES_OK
  AZX_EEPROM_RES_CANNOT_INIT,         /**< Cannot initialize the eeprom handle for internal errors*///!< AZX_EEPROM_RES_CANNOT_INIT
  AZX_EEPROM_RES_INVALID_HANDLE,      /**< Invalid handle passed to function*/                      //!< AZX_EEPROM_RES_INVALID_HANDLE
  AZX_EEPROM_RES_OVERFLOW,            /**< Read or Write operation overflows EEPROM size*/          //!< AZX_EEPROM_RES_OVERFLOW
  AZX_EEPROM_RES_WRITE_ERR,           /**< Error in write operation */                              //!< AZX_EEPROM_RES_WRITE_ERR
  AZX_EEPROM_RES_READ_ERR,            /**< Error in read operation*/                                //!< AZX_EEPROM_RES_READ_ERR
  AZX_EEPROM_RES_CANNOT_ALLOCATE,     /**< Cannot allocate dynamic buffer required internally*/     //!< AZX_EEPROM_RES_CANNOT_ALLOCATE

  AZX_EEPROM_RES_MAX                  /**< End of range, do not use */                              //!< AZX_EEPROM_RES_MAX
} AZX_EEPROM_RES_E;

/**
 * @brief EEPROM handle structure
 * This structure holds the parameters of the EEPROM handle, used by all communication functions
 *
 * \ingroup azx_eeprom
 */
typedef struct
{
  EEPROM_I2C_T i2c_handle;    /**< EEPROM I2C low level handle*/
  UINT16 mem_size;            /**< EEPROM memory total size in bytes, e.g. 32kB for 24XX256 */
  UINT16 page_size;           /**< EEPROM single page in bytes, e.g. 64 for 24XX256 */
} AZX_EEPROM_T;


/* Global functions =============================================================================*/

/**
 * @brief Initializes the EEPROM handle
 *
 * This function initializes the EEPROM handle with the provided I2C structure, address and sizes.
 * Opens the I2C channel for the required address.
 * Call it before any `azx_eeprom_*` is used.
 *
 *
 * @param[in] pEeprom Pointer to the structure that will be initialized
 * @param[in] pI2CHandle Pointer to the structure containing the I2C configuration (SDA, SCL pins)
 * @param[in] devAddr EEPROM slave address (refer to AZX_EEPROM_24XX256T_SLAVE_ADDR )
 * @param[in] memSize the full EEPROM memory size in bytes to be set
 * @param[in] pageSize the EEPROM page size in bytes to be set
 *
 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *
    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_init(AZX_EEPROM_T *pEeprom, EEPROM_I2C_T* pI2CHandle, UINT8 devAddr,
    UINT16 memSize, UINT16 pageSize);


/**
 * @brief De-initializes the EEPROM handle
 *
 * This function de-initializes the EEPROM handle and closes the I2C channel linked to the provided handle
 *
 *
 * @param[in] pEeprom Pointer to the structure that will be de-initialized
 *
 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *
    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_init(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }


    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }
 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_deinit(AZX_EEPROM_T *pEeprom);


/**
 * @brief Erases the EEPROM
 *
 * This function erases all the EEPROM memory, by writing 0xFF in every byte.
 *
 *
 * @param[in] pEeprom Pointer to the structure that will be erased
 *
 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *
    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_eraseAll(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }
 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_eraseAll(AZX_EEPROM_T *pEeprom);



/**
 * @brief Writes data in the EEPROM page by page
 *
 * This function will write input data into the EEPROM. Data will be sent to the memory chip with multiple write operations, each the size of the memory page size.
 * The function will wait 1 system tick between each page write operation
 *
 *
 * @param[in] pEeprom Pointer to the EEPROM structure
 * @param[in] mem_address Starting memory address as an unsigned 16bit integer.
 * @param[in] pData pointer to the data to be stored
 * @param[in] size size of the data to be stored, in bytes

 *
 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *
    #include <string.h>

    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;
    UINT16 randomAddr = 0x0213;
    char data[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789............................abcdefghijklmnopqrstuvwxyz0123456789............................"

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_writePages(&eepromH, randomAddr, (UINT8*) data, (UINT16) strlen(data));
    if(AZX_EEPROM_RES_OK != res)
    {
      //handle error
    }

    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_writePages(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData,
    UINT16 size);



/**
 * @brief Writes one byte of data in the EEPROM
 *
 * This function will write input byte into the EEPROM.
 *
 *
 * @param[in] pEeprom Pointer to the EEPROM structure
 * @param[in] mem_address Starting memory address as an unsigned 16bit integer.
 * @param[in] data value to be stored
 *
 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *
    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_writeByte(&eepromH, 0x00, 'A');
    if(AZX_EEPROM_RES_OK != res)
    {
      //handle error
    }

    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_writeByte(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 data);


/**
 * @brief Reads data in the EEPROM page by page
 *
 * This function will read data from the EEPROM, starting from the given address.
 * Data will be read from the memory chip with multiple "write start address + read page"
 * operations, using the memory page size.
 * The function will wait 1 system tick between each page read operation
 *
 *
 * @param[in] pEeprom Pointer to the EEPROM structure
 * @param[in] mem_address Starting memory address as an unsigned 16bit integer.
 * @param[in] pData pointer to the buffer to be filled with data from the flash
 * @param[in] size size of the data to be read, in bytes

 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *
    #include <stdio.h>
    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;
    UINT16 randomAddr = 0x0213;
    char data[128];

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_readPages(&eepromH, randomAddr, (UINT8*) data, sizeof(data));
    if(AZX_EEPROM_RES_OK != res)
    {
      //handle error
    }
    else
    {
      //manage retrieved data
    }

    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_readPages(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData,
    UINT16 size);


/**
 * @brief Reads data in the EEPROM sequentially
 *
 * This function will read data from the EEPROM, starting from the given address.
 * Data will be read from the memory chip with multiple "read page" operations,
 * using the memory page size, but with a single "write address" operation at the
 * very beginning.
 * The function will wait 1 system tick between each page read operation
 *
 * @param[in] pEeprom Pointer to the EEPROM structure
 * @param[in] mem_address Starting memory address as an unsigned 16bit integer.
 * @param[in] pData pointer to the buffer to be filled with data from the flash
 * @param[in] size size of the data to be read, in bytes

 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *
    #include <stdio.h>
    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;

    char data[128];

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_readSequentially(&eepromH, 0x00, (UINT8*) data, sizeof(data));
    if(AZX_EEPROM_RES_OK != res)
    {
      //handle error
    }
    else
    {
      //manage retrieved data
    }

    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_readSequentially(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData,
    UINT16 size);



/**
 * @brief Reads one byte of data from the EEPROM
 *
 * This function will read a single byte from the EEPROM, starting from the given address.
 *
 * @param[in] pEeprom Pointer to the EEPROM structure
 * @param[in] mem_address Starting memory address as an unsigned 16bit integer.
 * @param[in] pData pointer to the allocated variable to store the value.

 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *

    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;


    UINT8 data;

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_readByte(&eepromH, 0x00, &data);
    if(AZX_EEPROM_RES_OK != res)
    {
      //handle error
    }
    else
    {
      //manage retrieved data
    }


    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_readByte(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData);


/**
 * @brief Reads one byte of data from the EEPROM
 *
 * This function will read a single byte from the EEPROM, starting from current address.
 *
 * @param[in] pEeprom Pointer to the EEPROM structure
 * @param[in] pData pointer to the allocated variable to store the value.

 * @return AZX_EEPROM_RES_E value
 *
 * **Sample usage**
 *

    INT32 res;
    AZX_EEPROM_T eepromH = {0};
    EEPROM_I2C_T i2c_h = {0};
    UINT8 a2 = 0, a1 = 0, a0 = 0;


    UINT8 data;

    i2c_h.scl_pin = 2;
    i2c_h.sda_pin = 3;
    res = azx_eeprom_init(&eepromH, &i2c_h, AZX_EEPROM_24XX256T_SLAVE_ADDR(a2, a1, a0),
                        AZX_EEPROM_24XX256T_MEM_SIZE, AZX_EEPROM_24XX256T_PAGE_SIZE);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

    res = azx_eeprom_readByte(&eepromH, 0x00, &data);
    if(AZX_EEPROM_RES_OK != res)
    {
      //handle error
    }
    else
    {
      //manage retrieved data
    }

    res = azx_eeprom_readByteFromCurrentAddress(&eepromH, &data);
    if(AZX_EEPROM_RES_OK != res)
    {
      //handle error
    }
    else
    {
      //manage retrieved data
    }

    res = azx_eeprom_deinit(&eepromH);
    if(AZX_EEPROM_RES_OK != res)
    {
      return -1;
    }

 *
 *
 * \ingroup azx_eeprom
 */
INT32 azx_eeprom_readByteFromCurrentAddress(AZX_EEPROM_T *pEeprom, UINT8 *pData);

#endif /* HDR_AZX_EEPROM_24XX_H_ */
