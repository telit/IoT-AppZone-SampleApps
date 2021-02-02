/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "i2c_utils.h"
#include "azx_eeprom_24XX.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

/* Local function prototypes ====================================================================*/
/*----------------------------------------------------------------------------*/
/**
  @brief sleeps for a really short amount of time
  @return None

 */
/*----------------------------------------------------------------------------*/
static void fast_sleep(void);

/* Static functions =============================================================================*/
static void fast_sleep(void)
{
  m2mb_os_taskSleep(1);
}


/* Global functions =============================================================================*/


INT32 azx_eeprom_init(AZX_EEPROM_T *pEeprom, EEPROM_I2C_T* pI2CHandle, UINT8 devAddr,
    UINT16 memSize, UINT16 pageSize)
{

  if ( !pI2CHandle || !pEeprom )
  {
    return -1;
  }

  pEeprom->i2c_handle.scl_pin = pI2CHandle->scl_pin;
  pEeprom->i2c_handle.sda_pin = pI2CHandle->sda_pin;
  pEeprom->i2c_handle.dev_addr = devAddr;

  if(0 != EEPROM_I2C_init(&pEeprom->i2c_handle))
  {
    return AZX_EEPROM_RES_CANNOT_INIT;
  }
  pEeprom->mem_size = memSize;
  pEeprom->page_size = pageSize;

  return AZX_EEPROM_RES_OK;
}


/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_deinit(AZX_EEPROM_T *pEeprom)
{

  if ( !pEeprom || !EEPROM_I2C_isInit(&pEeprom->i2c_handle) )
  {
    return -1;
  }

  if(0 != EEPROM_I2C_deinit(&pEeprom->i2c_handle))
  {
    return -1;
  }

  pEeprom->mem_size = 0;
  pEeprom->page_size = 0;
  return AZX_EEPROM_RES_OK;
}

/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_eraseAll(AZX_EEPROM_T *pEeprom)
{
  UINT32 written = 0;
  UINT8 *data;

  if ( !pEeprom )
  {
    return AZX_EEPROM_RES_INVALID_HANDLE;
  }

  data = (UINT8 *) m2mb_os_malloc(sizeof(UINT8) * pEeprom->page_size);
  if(!data)
  {
    return AZX_EEPROM_RES_CANNOT_ALLOCATE;
  }
  /*"Empty" pages have 0xFF ... 0xFF stored*/
  memset(data, 0xFF, pEeprom->page_size);

  while ( written < pEeprom->mem_size )
  {

    if( 0 != EEPROM_i2c_write(&pEeprom->i2c_handle, 0x00 + written, data, pEeprom->page_size ))
    {
      m2mb_os_free(data);
      return AZX_EEPROM_RES_WRITE_ERR;
    }
    else
    {
      //m2mb_os_taskSleep(1); //sleep one tick
      fast_sleep();
      written += pEeprom->page_size;
    }

  }
  m2mb_os_free(data);

  return AZX_EEPROM_RES_OK;
}


/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_writePages(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData,
    UINT16 size)
{
  UINT32 written = 0;
  UINT8 chunkSize = 0;

  if ( !pEeprom )
  {
    return AZX_EEPROM_RES_INVALID_HANDLE;
  }

  if(mem_address + size > pEeprom->mem_size)
  {
    return AZX_EEPROM_RES_OVERFLOW;
  }

  while ( written < size )
  {
    /*Define the chunk size. if bigger than a eeprom page, use the page as chunk size. Use the remaining data amount otherwise. */
    chunkSize = (size - written > pEeprom->page_size)? pEeprom->page_size: size - written;

    if( 0 != EEPROM_i2c_write(&(pEeprom->i2c_handle), mem_address + written, &pData[written], chunkSize ))
    {
      return AZX_EEPROM_RES_WRITE_ERR;
    }
    else
    {
      //m2mb_os_taskSleep(1); //sleep one tick
      fast_sleep();
      written += chunkSize;
    }

  }
  return AZX_EEPROM_RES_OK;
}


/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_writeByte(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 data)
{
  if ( !pEeprom )
  {
    return AZX_EEPROM_RES_INVALID_HANDLE;
  }

  if(mem_address >= pEeprom->mem_size)
  {
    return AZX_EEPROM_RES_OVERFLOW;
  }


  if( 0 != EEPROM_i2c_write_byte(&pEeprom->i2c_handle, mem_address, data))
  {
    return AZX_EEPROM_RES_WRITE_ERR;
  }

  return AZX_EEPROM_RES_OK;
}

/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_readPages(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData,
    UINT16 size)
{

  UINT32 read = 0;
  UINT8 chunkSize = 0;

  if ( !pEeprom )
  {
    return AZX_EEPROM_RES_INVALID_HANDLE;
  }

  /*Read in chunks, as it improves the reliability*/

  if(mem_address + size > pEeprom->mem_size)
  {
    return AZX_EEPROM_RES_OVERFLOW;
  }

  while ( read < size )
  {
    /*Define the chunk size. if bigger than a eeprom page, use the page as chunk size. Use the remaining data amount otherwise. */
    chunkSize = (size - read > pEeprom->page_size)? pEeprom->page_size: size - read;

    if( 0 != EEPROM_i2c_read(&pEeprom->i2c_handle, mem_address + read, &pData[read], chunkSize ))
    {
      return AZX_EEPROM_RES_READ_ERR;
    }
    else
    {
      //m2mb_os_taskSleep(1); //sleep one tick
      fast_sleep();

      read += chunkSize;
    }

  }


  return AZX_EEPROM_RES_OK;
}


/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_readSequentially(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData,
    UINT16 size)
{

  if ( !pEeprom )
  {
    return AZX_EEPROM_RES_INVALID_HANDLE;
  }

  if(mem_address + size > pEeprom->mem_size)
  {
    return AZX_EEPROM_RES_OVERFLOW;
  }

  if( 0 != EEPROM_i2c_read_delayed_chunked(&pEeprom->i2c_handle, mem_address, pData, size, pEeprom->page_size, 1))
  {
    return AZX_EEPROM_RES_READ_ERR;
  }

  return AZX_EEPROM_RES_OK;
}


/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_readByte(AZX_EEPROM_T *pEeprom,
    UINT16 mem_address, UINT8 *pData)
{

  if ( !pEeprom )
  {
    return AZX_EEPROM_RES_INVALID_HANDLE;
  }

  if(mem_address >= pEeprom->mem_size)
  {
    return AZX_EEPROM_RES_OVERFLOW;
  }

  if( 0 != EEPROM_i2c_read_byte_from_address(&pEeprom->i2c_handle, mem_address, pData ))
  {
    return AZX_EEPROM_RES_READ_ERR;
  }

  return AZX_EEPROM_RES_OK;
}


/*----------------------------------------------------------------------------*/
INT32 azx_eeprom_readByteFromCurrentAddress(AZX_EEPROM_T *pEeprom, UINT8 *pData)
{

  if ( !pEeprom )
  {
    return AZX_EEPROM_RES_INVALID_HANDLE;
  }


  if( 0 != EEPROM_i2c_read_byte(&pEeprom->i2c_handle, pData ))
  {
    return AZX_EEPROM_RES_READ_ERR;
  }

  return AZX_EEPROM_RES_OK;
}

