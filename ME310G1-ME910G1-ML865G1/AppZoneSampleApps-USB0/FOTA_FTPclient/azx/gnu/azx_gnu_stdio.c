/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
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
#include <errno.h>

#include "m2mb_types.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_fs_posix.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_fs_errno.h"
#include "azx_gnu_sys_types.h"

#include "azx_log.h"
#include "azx_gnu_stdlib.h"

/* Function prototypes and Local defines ========================================================*/
#include "azx_gnu_stdio.h"

/* Function definition ==========================================================================*/

static int _fprintf(void * f, char *buf)
{
  int chars_written = 0;

  if (stdout == f)
  {
    AZX_LOG_INFO("%s", buf);
    chars_written = strlen(buf);
  }
  else if (stderr == f)
  {
    AZX_LOG_INFO("%s", buf);
    chars_written = strlen(buf);
  }
  else
  {
    chars_written = (int) m2mb_fs_fwrite(buf,sizeof(CHAR), strlen(buf), ((M2MB_FILE_T*)f));
  }

  return chars_written;
}


int azx_gnu_fprintf(void *f, const char *format, ...)
{
  va_list arg;
  CHAR *buf = (CHAR*)malloc(2048);
  memset(buf, 0, 2048);
  va_start(arg, format);
  vsprintf(buf, format, arg);
  va_end(arg);


  int chars_written = _fprintf(f, buf);
  free(buf);
  return chars_written;
}

int azx_gnu_vfprintf(void *f, const char *format, va_list ap)
{

  CHAR *buf = (CHAR*)malloc(2048);
  memset(buf, 0, 2048);

  vsprintf(buf, format, ap);

  int chars_written = _fprintf(f, buf);

  free(buf);
  return chars_written;
}


int azx_gnu_truncate (const char* path, off_t length)
{
  return (int) m2mb_fs_truncate(path, length);
}


int azx_gnu_ungetc(void *f, int c)
{
	if (-1 == m2mb_fs_fseek((M2MB_FILE_T*)f, -1, SEEK_CUR))
	{
		return EOF;
	}
	return c;
}

int azx_gnu_ferror(void* f)
{
  	UNUSED_1(f);
	return m2mb_fs_get_errno_value();
}

FILE* azx_gnu_fdopen(int fd, const char *name)
{
  	UNUSED_2(fd, name);
	return NULL;
}

void azx_gnu_clearerr(FILE* fd)
{
  	UNUSED_1(fd);
	return;
}




int azx_gnu_feof(void *f)
{
	int ret = 0;

	if(f == NULL)
	{
		errno = EINVAL;
	}
	else
	{
		INT32 current, end;
		current = m2mb_fs_ftell((M2MB_FILE_T*)f);
		if(current == -1)
		{
			errno = EINVAL;
		}
		else
		{
			if(m2mb_fs_fseek((M2MB_FILE_T*)f, 0, M2MB_SEEK_END) == -1)
			{
				errno = EINVAL;
			}
			else
			{
				end = m2mb_fs_ftell((M2MB_FILE_T*)f);
				if(end == -1)
				{
					errno = EINVAL;
				}
				else
				{
					if(current == end)
					{
						ret = 1;
					}
					else
					{
						if(m2mb_fs_fseek((M2MB_FILE_T*)f, current, M2MB_SEEK_SET) == -1)
						{
							errno = EINVAL;
						}
					}
				}
			}
		}
	}
	return ret;
}



