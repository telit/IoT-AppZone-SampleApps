/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 	 azx_gnu_stdlib.c

 @brief
 	 gnu definition

 @details
 	 Porting from gnu to azx

 @note
 	 Dependencies:
 	 	 m2mb_types.h
 	 	 m2mb_os_api.h

 @author Moreno Floris
 @author Norman Argiolas

 @date
 	 11/02/2020
*/

/* Include files ================================================================================*/
#ifndef _STDIO_H_
#include <stdio.h>
#endif

#include <stddef.h>
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_appMng.h"

#include "azx_log.h"

#include "azx_gnu_stdio.h"
#include "azx_gnu_stdlib.h"
/* Function prototypes and Local defines ========================================================*/


/* Function definition ==========================================================================*/

void *azx_gnu_malloc(size_t size)
{
	return m2mb_os_malloc(size);
}

void *azx_gnu_calloc(size_t nitems, size_t size)
{
#ifdef APP_NAME /*Emulator*/
  void* p = m2mb_os_calloc(nitems * size);
  if(p)
  {
    memset(p, 0, nitems * size);
  }
  return p;
#else
	return m2mb_os_calloc(nitems * size);
#endif
}

void azx_gnu_free(void *ptr)
{
	m2mb_os_free(ptr);
}

void * azx_gnu_realloc(void * ptr, size_t size)
{
	if (0 == size)
	{
		if (NULL != ptr)
			m2mb_os_free(ptr); // the 'if' should not be necessary by C standard but just in case
		return NULL;
	} else if (NULL != ptr)
	{
		void * returnpointer = NULL;

		returnpointer = m2mb_os_malloc((UINT32) size);

		if (returnpointer != NULL)
		{
			memcpy(returnpointer, ptr, (UINT32) size);
		}

		AZX_LOG_TRACE("reallocated %u bytes from %p ", size, ptr);

		m2mb_os_free(ptr);
		ptr = NULL;

		return returnpointer;
	} else
	{
		void *res = m2mb_os_malloc((UINT32) size);
		if (res)
		{
			AZX_LOG_TRACE("(re)allocated %u bytes in %p \r\n", size, res);
			return res;
		} else
		{
			AZX_LOG_ERROR("unable to reallocate %u bytes \r\n", size);
			return NULL;
		}
	}
}

void azx_gnu_exit(int status)
{
  M2MB_APPMNG_HANDLE myappHandle = NULL;
  M2MB_APPMNG_RESULT_E appRes;
  myappHandle = m2mb_appMng_getMyHandle();

  AZX_LOG_TRACE("Exit with status %d\r\n", status);
  appRes = m2mb_appMng_stop(myappHandle);
  if( appRes != M2MB_APPMNG_RESULT_SUCCESS )
  {
    AZX_LOG_CRITICAL("m2mb_appMng_stop failure, error %d\r\n", appRes);
  }

}
