/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file az_hw.c

  @brief Utilities for lfs2 in AppZone
  
  @note File created on: Oct 21, 2020

  @author NormanAr

*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "azx_log.h"

#include "az_hw.h"

static UINT32 get_uptime (void);


/*******************************************************************************
                  get_uptime

Function:       UINT32 get_uptime(void)
Arguments:
Description:    get system current tick(unit: 10ms) function ;

*******************************************************************************/
static UINT32 get_uptime(void)
{

  UINT32 sysTicks = m2mb_os_getSysTicks();

  FLOAT32 ms_per_tick = m2mb_os_getSysTickDuration_ms();

  return (UINT32) (sysTicks * ms_per_tick); //milliseconds
}
/***************** get_uptime ***********************************************/


/*******************************************************************************
                  LFS2_logFormatted

Function:
Arguments:
Description:
*******************************************************************************/
void LFS2_logFormatted(LFS2_LOG_HOOK_LEVELS_E level,
					   const char* function,
					   const char* file,
					   int line, const char *fmt, ... )
{
	char buf[512];
	int bufSize = sizeof(buf);
	va_list arg;
	INT32   offset = 0;
	unsigned int now = get_uptime();

	//CHAR taskName[32];

	memset(buf,0,bufSize);

	switch(level)
	{
	case LFS2_LOG_LEVEL_ERROR:
		offset = sprintf(buf, "%s %u.%03u %8s:%d - %8s: ",
				"[ERROR]",
				now / 1000, now % 1000,
				file,
				line,
				function
		);
		break;
	case LFS2_LOG_LEVEL_INFO:
		break;
	case LFS2_LOG_LEVEL_DEBUG:
		offset = sprintf(buf, "%s %u.%03u %8s:%d - %8s: ",
				"[DEBUG]",
				now / 1000, now % 1000,
				file,
				line,
				function
		);

		break;
	default:
		break;
	}
	va_start(arg, fmt);
	vsnprintf(buf + offset, bufSize-offset, fmt, arg);
	va_end(arg);

	AZX_LOG_INFO("%s\r\n", buf);  //This is a generic function, like printf

}
/***************** SPI_FLASH_logFormatted ***********************************************/

M2MB_OS_RESULT_E az_free(void *buf)
{
	return  m2mb_os_free(buf);
}

void *az_malloc(UINT32 size)
{
	return m2mb_os_malloc(size);
}

