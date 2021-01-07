/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  azx_spi_flash_strings.h

  @brief
  	  Project: SPI data flash

  @details

  @version
 	 1.0.0

  @note

  @author
  	  Norman Argiolas

  @note
  	  File created on: Apr 08, 2020

*/

#ifndef AZX_SPI_FLASH_STRINGS_H_
#define AZX_SPI_FLASH_STRINGS_H_

#include "azx_spi_flash_hw_external.h"

static const char * getMessage(AZX_SPI_FLASH_CODE_RESULT_E code)
{
    switch(code)
    {
    case AZX_SPI_FLASH_CODE_PASS: 					 return "OK";
    case AZX_SPI_FLASH_CODE_FAIL: 					 return "FAIL";
    case AZX_SPI_FLASH_CODE_FLASH_SIZE_OVERFLOW: 	 return "";
    case AZX_SPI_FLASH_CODE_WRONG_ADDRESS: 			 return "";
    case AZX_SPI_FLASH_CODE_DIFFERENT_PAGES: 		 return "";
    case AZX_SPI_FLASH_CODE_WRITE_PROTECTED: 		 return "";
    case AZX_SPI_FLASH_CODE_UNLOCKED_BLOCK: 		 return "";
    case AZX_SPI_FLASH_CODE_LOCKED_BLOCK: 			 return "";
    case AZX_SPI_FLASH_CODE_LOCK_DOWN: 				 return "";
    case AZX_SPI_FLASH_CODE_CACHE_READ_NOT_POSSIBLE: return "";
    case AZX_SPI_FLASH_CODE_OPERATION_NOT_ALLOWED: 	 return "";
    case AZX_SPI_FLASH_CODE_INVALID_ARGUMENT: 		 return "Invalid Argument - Issue Command fail!"; //TODO Duplicate
    case AZX_SPI_FLASH_CODE_TIMEDOUT: 				 return "Connection timed out!"; //TODO Duplicate
    case AZX_SPI_FLASH_CODE_MALLOC_FAIL: 			 return "Memory error - Malloc fail!";
    case AZX_SPI_FLASH_CODE_INVALID_COMMAND: 		 return "";
    case AZX_SPI_FLASH_CODE_SPI_WRITE_FAIL: 		 return "SPI Write fail!";
    case AZX_SPI_FLASH_CODE_SPI_READ_FAIL: 			 return "SPI Read fail!";
    case AZX_SPI_FLASH_CODE_EIO: 					 return "I/O error!";
    case AZX_SPI_FLASH_CODE_ENOMEM: 				 return "Out of memory!";
    case AZX_SPI_FLASH_CODE_ENODEV: 				 return "No such device!";
    case AZX_SPI_FLASH_CODE_EINVAL: 				 return "Invalid argument!"; //TODO Duplicate
    case AZX_SPI_FLASH_CODE_EBADMSG: 				 return "Internal ECC error reading page - Bad Block faund!";
    case AZX_SPI_FLASH_CODE_ETIMEDOT: 				 return "Connection timed out!"; //TODO duplicate
    case AZX_SPI_FLASH_CODE_EUCLEAN: 				 return "Chip needs cleaning!";
    case AZX_SPI_FLASH_CODE_ENOTSUPP: 				 return "Operation is not supported!";
    case AZX_SPI_FLASH_CODE_SPI_INIT_FAIL: 			 return "SPI initialization error!";
    case AZX_SPI_FLASH_CODE_GPIO_INIT_FAIL: 		 return "GPIO initialization error!";
    case AZX_SPI_FLASH_CODE_INVALID_MEMORY_MODEL:	 return "Invalid nand flash memory model!";
    case AZX_SPI_FLASH_CODE_NOT_INITIALIZED: 		 return "Memory not initialized yet!";
    case AZX_SPI_FLASH_CODE_RESET_FAIL: 			 return "Spi Reset fail!";
    case AZX_SPI_FLASH_CODE_ECC_FAIL: 				 return "ECC initialization error!";
    case AZX_SPI_FLASH_CODE_QUAD_FAIL: 				 return "QUAD initialization error!";
    case AZX_SPI_FLASH_CODE_FREE_FAIL: 				 return "Release memory error - Free fails!";
    case AZX_SPI_FLASH_CODE_BAD_UID: 				 return "Error reading UID!";
    case AZX_SPI_FLASH_CODE_PARAMETER: 				 return "Error reading parameter page to cache!";
    case AZX_SPI_FLASH_CODE_ONFI: 					 return "Could not find valid ONFI parameter page!";
    default:
    	return NULL;
    }
}

#endif /* AZX_SPI_FLASH_STRINGS_H_ */
