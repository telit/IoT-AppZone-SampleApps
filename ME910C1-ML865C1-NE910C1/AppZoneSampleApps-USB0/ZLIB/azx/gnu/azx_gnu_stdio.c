/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
    azx_gnu_stdio.c

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

#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "m2mb_types.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_fs_posix.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "azx_gnu_sys_types.h"

#include "azx_log.h"
#include "azx_gnu_stdlib.h"

/* Function prototypes and Local defines ========================================================*/
#include "azx_gnu_stdio.h"

/* Function definition ==========================================================================*/

int azx_gnu_fprintf(void *f, const char *format, ...)
{
  va_list arg;
  CHAR *buf = (CHAR*)malloc(2048);
  memset(buf, 0, 2048);
  va_start(arg, format);
  vsprintf(buf, format, arg);
  va_end(arg);

  int chars_written = 0;

#if 1
  if (stdout == f)
  {
    AZX_LOG_INFO("%s", buf);
    chars_written = strlen(buf);
  }
  else if (stderr == f)
  {
    //chars_written = AZX_LOG_ERROR(buf); //LOG_ERROR is not really useful because it refers to this line in this file
    AZX_LOG_INFO("%s", buf);
    chars_written = strlen(buf);
  }
  else
  {
    chars_written = (int) m2mb_fs_fwrite(buf,sizeof(CHAR), strlen(buf), ((M2MB_FILE_T*)f));
  }
#endif
  free(buf);
  return chars_written;
}


int azx_gnu_truncate (const char* path, off_t length)
{
  return (int) m2mb_fs_truncate(path, length);
}

