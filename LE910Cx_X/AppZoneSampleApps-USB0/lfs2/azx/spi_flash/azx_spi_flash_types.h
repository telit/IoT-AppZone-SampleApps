/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  azx_spi_flash_types.h

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


#ifndef AZX_SPI_FLASH_TYPES_H
#define AZX_SPI_FLASH_TYPES_H

/********JSC TYPE DEFINITION************/
typedef unsigned char 		JSC_uint8;
typedef unsigned short 		JSC_uint16;
typedef unsigned int 		JSC_uint32;
typedef unsigned long long 	JSC_uint64;

typedef signed int			JSC_int32;
typedef char  				JSC_char;
typedef int 				JSC_bool;

typedef long long 			JSC_loff_t;
typedef unsigned int 		JSC_size_t;

typedef void                *JSC_HANDLE;

#ifndef NULL
	#define NULL  0
#endif

#ifndef true
	#define true  1
#endif

#ifndef false
	#define false 0
#endif

#endif /* AZX_SPI_FLASH_TYPES_H */
