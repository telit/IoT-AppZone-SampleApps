/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/* Include files =============================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include <errno.h>

#include "m2mb_types.h"

/* Local defines =============================================================*/
#ifdef __ARMCLIB_VERSION  /*Using RVCT ARM compiler*/
  #define ULONG_LONG_MAX ULLONG_MAX
#endif

#ifndef ULONG_LONG_MAX
  #define ULONG_LONG_MAX ULLONG_MAX
#endif

/* Local typedefs ============================================================*/
/* Local statics =============================================================*/
/* Local function prototypes =================================================*/
/* Static functions ==========================================================*/

/* Global functions ==========================================================*/
INT32 azx_str_to_l( char *str, INT32 *output )
{
  char *endptr;
  INT32 tmp;
  errno = 0;

  if( str[strspn( str, "0123456789-+" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -3;
  }

  tmp = strtol( str, &endptr, 10 );

  if( ( errno == ERANGE && ( ( tmp == LONG_MAX ) || ( tmp == LONG_MIN ) ) ) || ( errno != 0 &&
                                                                                 tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( endptr == ( char * ) str )
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}

INT32 azx_str_to_ul( char *str, UINT32 *output )
{
  char *endptr;
  unsigned long tmp;
  errno = 0;

  if( str[strspn( str, "0123456789" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -3;
  }

  tmp = strtoul( str, &endptr, 10 );

  if( ( errno == ERANGE && ( tmp == ULONG_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( endptr == ( char * ) str )
  {
    /*no digits found*/
    return -2;
  }

  *output = ( UINT32 )tmp;
  return 0;
}

INT32 azx_str_to_ull( char *str, UINT64 *output )
{
  char *endptr;
  UINT64 tmp;
  errno = 0;

  if( str[strspn( str, "0123456789" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -3;
  }

  tmp = strtoull( str, &endptr, 10 );

  if( ( errno == ERANGE && ( tmp == ULONG_LONG_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( endptr == ( char * ) str )
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}

INT32 azx_str_to_ul_hex( char *str, UINT32 *output )
{
  char *endptr;
  UINT32 tmp;
  errno = 0;

  if( str[strspn( str, "0123456789abcdefABCDEF" )] !=
      0 ) //check if string is composed of HEX digits only
  {
    /*not valid hex input*/
    return -3;
  }

  tmp = strtoul( str, &endptr, 16 );

  if( ( errno == ERANGE && ( tmp == ULONG_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    return -1;
  }

  if( endptr == ( char * ) str )
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}

INT8 azx_str_to_uc( char *str, UINT8 *output )
{
  char *endptr;
  UINT32 tmp;
  errno = 0;

  if( str[strspn( str, "0123456789" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -3;
  }

  tmp = strtoul( str, &endptr, 10 );

  if( ( errno == ERANGE && ( tmp == UCHAR_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( endptr == ( char * ) str )
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}

INT8 azx_str_to_d( char *str, FLOAT64 *output )
{
  char *endptr;
  FLOAT64 tmp;
  errno = 0;

  if( str[strspn( str, "0123456789.-+" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -3;
  }

  tmp = strtod( str, &endptr );

  if( ( errno == ERANGE && ( tmp == DBL_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( endptr == ( char * ) str )
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}

INT8 azx_str_to_f( char *str, FLOAT32 *output )
{
  char *endptr;
  FLOAT32 tmp;
  errno = 0;

  if( str[strspn( str, "0123456789.-+" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -3;
  }

  tmp = strtof( str, &endptr );

  if( ( errno == ERANGE && ( tmp == FLT_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( endptr == ( char * ) str )
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}

UINT8 azx_asc_to_dec( char ch )
{
  if( ch >= '0' && ch <= '9' )
  {
    return ch - '0';
  }

  if( ch >= 'a' && ch <= 'f' )
  {
    return ch - 'a' + 10;
  }

  if( ch >= 'A' && ch <= 'F' )
  {
    return ch - 'A' + 10;
  }

  return 0;
}



UINT8 azx_str_rem_ch( char *str, char chr )
{
  int i, j = 0;

  for( i = 0; str[i] != '\0'; i++ )  /* 'i' moves through all of original 's' */
  {
    if( str[i] != chr )
    {
      str[j++] = str[i]; /* 'j' only moves after we write a non-'chr' */
    }
  }

  str[j] = '\0'; /* re-null-terminate */
  return 0;
}


UINT8 azx_str_l_trim( char *str )
{
  char *p = str;

  while( *p && *p == ' ' )
  {
    p++;
  }

  int f_lenght = strlen( p );
  strncpy( str, p, f_lenght );
  str[f_lenght] = 0;
  return 0;
}

UINT8 azx_str_r_trim( char *str )
{
  char *p = &str[strlen( str ) - 1];

  while( p != str && *p == ' ' )
  {
    p--;
  }

  if( p == str && *p == ' ' ) // case str is with spaces only
  {
    *p = 0;
  }

  *( p + 1 ) = 0;
  return 0;
}


