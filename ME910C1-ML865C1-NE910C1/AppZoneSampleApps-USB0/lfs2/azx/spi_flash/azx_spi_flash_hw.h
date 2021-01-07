/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
 @file
 	 azx_spi_flash_hw.h

 @brief
 	 Project: SPI data flash

 @details

 @version
 	 1.0.0

 @note

 @author
 	 Norman Argiolas
 	 Fabio Pintus

 @date
 	 File created on: Apr 01, 2020
 */

#ifndef AZX_SPI_FLASH_HW_H
#define AZX_SPI_FLASH_HW_H

/* Include files ================================================================================*/

//------------- include NAND JSC dependancies
#include "azx_spi_flash_types.h"
#include "azx_spi_flash_hw_external.h"
//-------------------------------------------

/* Global defines ================================================================================*/
//#define NAND_DEBUG
#ifdef NAND_DEBUG
	#define AZX_SPI_FLASH_LOG_DEBUG(a...)   	SPI_FLASH_logFormatted(SPI_FLASH_LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, a)
#else
	#define AZX_SPI_FLASH_LOG_DEBUG(a...)
#endif

#define AZX_SPI_FLASH_LOG_INFO(a...)      	SPI_FLASH_logFormatted(SPI_FLASH_LOG_LEVEL_INFO, "", "", 0, a)

#define AZX_SPI_FLASH_LOG_ERROR(a...)     	SPI_FLASH_logFormatted(SPI_FLASH_LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, a)



/* Global function prototypes ====================================================================*/

AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_spi_initialization  	(JSC_int32 *spi_fd);
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_spi_close				(JSC_int32 *spi_fd);
AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_gpio_close			(JSC_int32 *gpio_fd);


AZX_SPI_FLASH_CODE_RESULT_E azx_spi_flash_gpio_initialization(const JSC_uint8 CS_GPIO_pin,
															  JSC_int32 *gpio_fd);


AZX_SPI_FLASH_CODE_RESULT_E 	azx_spi_flash_spi_xfer(AZX_SPI_FLASH_SPI_SLAVE *slave,
													   JSC_uint32 bitlen,
													   void *dout, void *din,
													   JSC_uint64 flags);

AZX_SPI_FLASH_CODE_RESULT_E 	azx_spi_flash_free(void *buf);
AZX_SPI_FLASH_CODE_RESULT_E 	azx_spi_flash_memcpy(void* dst, const void* src, JSC_size_t size);
AZX_SPI_FLASH_CODE_RESULT_E 	azx_spi_flash_memset(void* buf, JSC_int32 val, JSC_size_t size);
JSC_int32 						azx_spi_flash_memcmp(const void* src1, const void* src2, JSC_size_t size);
void *							azx_spi_flash_malloc(JSC_size_t size);
void 							azx_spi_flash_delay(JSC_int32 delay);
JSC_uint32 						azx_spi_flash_get_uptime(void); //verify better
JSC_uint32 						azx_spi_flash_get_ticks(void);


#endif //AZX_SPI_FLASH_HW_H
