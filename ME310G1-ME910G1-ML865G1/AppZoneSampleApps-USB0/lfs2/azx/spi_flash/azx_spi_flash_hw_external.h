/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  azx_spi_flash_hw_external.h

  @brief
  	  Project: SPI data flash

  @details

  @version
  	  1.0.0

  @note

  @author
  	  Norman Argiolas
  	  Fabio Pintus

  @dte
  	  File created on: Mar 14, 2020
*/

#ifndef AZX_SPI_FLASH_HW_EXTERNAL_H_
#define AZX_SPI_FLASH_HW_EXTERNAL_H_

#include "azx_spi_flash_types.h"

/* Global defines ================================================================================*/

#define __CS1467G__
/*=============== SPI FLASH JSC Pin Configuration (8-LGA) ===============*/
#if defined(__CS1467G__)
	#define SPI_FLASH_JS28P1GQSCAHG_CS_GPIO 2  //pin 2 -> 2@pl302
#endif


typedef enum
{
	SPI_FLASH_LOG_LEVEL_NONE 	= 0,        /**<Do not print any message*/
	SPI_FLASH_LOG_LEVEL_ERROR 	= 1,        /**<Print an error message*/
	SPI_FLASH_LOG_LEVEL_INFO 	= 2,        /**<Print an information message*/
	SPI_FLASH_LOG_LEVEL_DEBUG 	= 3			/**<Print a debug message*/
} SPI_FLASH_LOG_HOOK_LEVELS_E;

typedef enum SPI_FLASH_CODE_RESULT
{
	SPI_FLASH_CODE_PASS 					=  1,  /* the operation on the NAND was successfully completed*/
	SPI_FLASH_CODE_FAIL   					= -1,  /* the operation on the nand failed */
	SPI_FLASH_CODE_MALLOC_FAIL				= -13 /* Error alloc memory*/
} SPI_FLASH_CODE_RESULT_E;


/* Global function prototypes ====================================================================*/

void SPI_FLASH_logFormatted(SPI_FLASH_LOG_HOOK_LEVELS_E level,
							const char* function,
							const char* file,
							int line, const char *fmt, ... );

/****************************************************************************
 	 	 	 	 Return Codes
 ****************************************************************************/
typedef enum AZX_SPI_FLASH_CODE_RESULT
{
	AZX_SPI_FLASH_CODE_PASS 						=  1,  /* the operation on the NAND was successfully completed*/
	AZX_SPI_FLASH_CODE_FAIL   						= -1,  /* the operation on the nand failed */
	AZX_SPI_FLASH_CODE_FLASH_SIZE_OVERFLOW 			= -2,  /* the address is not within the device*/
	AZX_SPI_FLASH_CODE_WRONG_ADDRESS       			= -3,  /* the address is not */
	AZX_SPI_FLASH_CODE_DIFFERENT_PAGES     			= -4,  /* page different in datainput command*/
	AZX_SPI_FLASH_CODE_WRITE_PROTECTED	 			= -5,  /* device is wite protected */
	AZX_SPI_FLASH_CODE_UNLOCKED_BLOCK		 		= -6,  /* block unlocked*/
	AZX_SPI_FLASH_CODE_LOCKED_BLOCK		 			= -7,  /* block locked*/
	AZX_SPI_FLASH_CODE_LOCK_DOWN		     		= -8,  /* block lock-down*/
	AZX_SPI_FLASH_CODE_CACHE_READ_NOT_POSSIBLE  	= -9,  /* required cache read dataoutput with  cache read not pending */
	AZX_SPI_FLASH_CODE_OPERATION_NOT_ALLOWED    	= -10, /* Operation is not permitted on this device*/
	AZX_SPI_FLASH_CODE_INVALID_ARGUMENT  			= -11, /* Invalid argument */
	AZX_SPI_FLASH_CODE_TIMEDOUT						= -12, /* Connection timed out */
	AZX_SPI_FLASH_CODE_MALLOC_FAIL					= -13, /* Error alloc memory*/
	AZX_SPI_FLASH_CODE_INVALID_COMMAND				= -14,  /* Invalid command*/
	AZX_SPI_FLASH_CODE_SPI_WRITE_FAIL				= -15,
	AZX_SPI_FLASH_CODE_SPI_READ_FAIL				= -16,
	AZX_SPI_FLASH_CODE_EIO							= -17, /* I/O error */
	AZX_SPI_FLASH_CODE_ENOMEM						= -18, /* Out of memory */
	AZX_SPI_FLASH_CODE_ENODEV						= -19, /* No such device */
	AZX_SPI_FLASH_CODE_EINVAL						= -20, /* Invalid argument */
	AZX_SPI_FLASH_CODE_EBADMSG						= -21, /* Not a data message */
	AZX_SPI_FLASH_CODE_ETIMEDOT						= -22, /* Connection timed out */
	AZX_SPI_FLASH_CODE_EUCLEAN						= -23, /* Chip needs cleaning */
	AZX_SPI_FLASH_CODE_ENOTSUPP						= -24,  /* Operation is not supported */
	AZX_SPI_FLASH_CODE_SPI_INIT_FAIL				= -25,
	AZX_SPI_FLASH_CODE_GPIO_INIT_FAIL				= -26,
	AZX_SPI_FLASH_CODE_INVALID_MEMORY_MODEL 		= -27,
	AZX_SPI_FLASH_CODE_NOT_INITIALIZED				= -28,
	AZX_SPI_FLASH_CODE_RESET_FAIL					= -29,
	AZX_SPI_FLASH_CODE_ECC_FAIL						= -30,
	AZX_SPI_FLASH_CODE_QUAD_FAIL					= -31,
	AZX_SPI_FLASH_CODE_FREE_FAIL					= -32,
	AZX_SPI_FLASH_CODE_BAD_UID						= -33,
	AZX_SPI_FLASH_CODE_PARAMETER					= -34,
	AZX_SPI_FLASH_CODE_ONFI							= -35
} AZX_SPI_FLASH_CODE_RESULT_E;

typedef enum
{
	AZX_SPI_FLASH_LOG_LEVEL_NONE 	= 0,        /**<Do not print any message*/
	AZX_SPI_FLASH_LOG_LEVEL_ERROR 	= 1,        /**<Print an error message*/
	AZX_SPI_FLASH_LOG_LEVEL_INFO 	= 2,        /**<Print an information message*/
	AZX_SPI_FLASH_LOG_LEVEL_DEBUG 	= 3			/**<Print a debug message*/
} AZX_SPI_FLASH_LOG_HOOK_LEVELS_E;


/* Ability of SPI Controllers */
#define SPI_OPM_RX_SING             (0)         /* Support 1 pin RX */
#define SPI_OPM_RX_DUAL             (1 << 1)    /* Support 2 pin RX */
#define SPI_OPM_RX_QUAD             (1 << 3)    /* Support 4 pin RX */

#define SPI_OPM_TX_SING             (0)         /* Support 1 pin TX */
#define SPI_OPM_TX_QUAD             (1 << 0)    /* Support 4 pin TX */

/* Flags of SPI Transfer */
#define SPI_XFER_BEGIN              (1 << 0)    /* Assert CS before transfer */
#define SPI_XFER_END                (1 << 1)    /* Deassert CS after transfer */
#define SPI_XFER_DUAL               (1 << 29)   /* 2 pin data transfer */
#define SPI_XFER_QUAD               (1 << 30)   /* 4 pin data transfer */


/* Global typedef ==============================================================================*/

typedef enum
{
  LOW_VALUE = 0,     /**< Drive the output LOW. */
  HIGH_VALUE = 1     /**< Drive the output HIGH. */
} AZX_SPI_FLASH_GPIO_VALUE_E;

/*
* SPI controller device structure
* @hspi: spi device handle
* @op_mode_rx: spi operation mode for rx
* @op_mode_tx: spi operation mode for tx
*/
typedef struct {
	void		*hspi;
	JSC_uint8   op_mode_rx;
	JSC_uint8   op_mode_tx;
} AZX_SPI_FLASH_SPI_SLAVE;

typedef struct {
	JSC_int32 gpio;
	JSC_int32 spi;
} AZX_SPI_FLASH_DESCRIPTOR;

#endif /* AZX_SPI_FLASH_HW_EXTERNAL_H_ */
