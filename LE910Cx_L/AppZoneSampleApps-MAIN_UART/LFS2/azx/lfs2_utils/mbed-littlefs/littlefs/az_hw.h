/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file az_hw.h

  @brief Utilities for lfs in AppZone
  
  @note File created on: Oct 21, 2020

  @author NormanAr

*/

#ifndef MBED_LITTLEFS_LITTLEFS_AZ_HW_H_
#define MBED_LITTLEFS_LITTLEFS_AZ_HW_H_

#include <stdio.h>
#include <string.h>
#include "m2mb_os_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**\name LFS2_LOG_HOOK_LEVELS enum
 * \brief Debug lfs2 levels.
 *  @{ */
typedef enum
{
	LFS2_LOG_LEVEL_NONE 	= 0,        /**<Do not print any message*/
	LFS2_LOG_LEVEL_ERROR 	= 1,        /**<Print an error message*/
	LFS2_LOG_LEVEL_INFO 	= 2,        /**<Print an information message*/
	LFS2_LOG_LEVEL_DEBUG 	= 3			/**<Print a debug message*/
} LFS2_LOG_HOOK_LEVELS_E;
/** @} */


/*******************************************************************************
                  LFS2_logFormatted

Function:
Arguments:
Description:
*******************************************************************************/
void LFS2_logFormatted(LFS2_LOG_HOOK_LEVELS_E level,
					   const char* function,
					   const char* file,
					   int line, const char *fmt, ... );


M2MB_OS_RESULT_E az_free(void *buf);
void *az_malloc(UINT32 size);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MBED_LITTLEFS_LITTLEFS_AZ_HW_H_ */
