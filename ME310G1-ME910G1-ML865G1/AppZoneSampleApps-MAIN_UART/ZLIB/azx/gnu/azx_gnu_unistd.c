/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 	 azx_gnu_unistd.c

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

#include "azx_gnu_stdio.h"

/* Function prototypes and Local defines ========================================================*/
#include "azx_gnu_unistd.h"

/* Function definition ==========================================================================*/

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    chown

  @details
    chown not supported

  @param[in]

  @return

  @note
    chown not supported

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
int azx_gnu_chown(const char *__path, uid_t __owner, gid_t __group )
{
	(void) __path;
	(void) __owner;
	(void) __group;

	return 0; /*chown not supported. */
}

