/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_string.c

 @brief
 gnu definition

 @details
 Porting from gnu to azx

 @note
 <note>

 @author Fabio Pintus
 @author Norman Argiolas

 @date
 11/02/2020
 */

/* Include files ================================================================================*/
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"

/* Function prototypes and Local defines ========================================================*/
#include "azx_gnu_string.h"

/* Function definition ==========================================================================*/

char * azx_gnu_strdup(const char * s)
{
	char * newp = (char *) m2mb_os_malloc(strlen(s) + 1);
	if (!newp)
	{
		return NULL;
	}
	else
	{
		strcpy(newp, s);
		return newp;
	}
}

char * azx_gnu_strndup(const char * s, size_t n)
{
	char * newp = (char *) m2mb_os_malloc(n + 1);
	if (!newp)
	{
		return NULL;
	}
	else
	{
		strncpy(newp, s, n);
		newp[n] = '\0';
		return newp;
	}
}
