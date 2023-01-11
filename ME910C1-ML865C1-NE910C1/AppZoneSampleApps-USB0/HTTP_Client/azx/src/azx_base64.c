/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/* Include files =============================================================*/
#include "m2mb_types.h"

#include <ctype.h>
#include "azx_base64.h"

int cust_isascii(int c) { return !!((c >= 0) && (c <= 127)); }

#ifndef isascii
#define isascii cust_isascii
#endif

/* Local defines =============================================================*/
// Base 64 Related
#define BAD     -1
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)

/* Local typedefs ============================================================*/
/* Local statics =============================================================*/



static const char base64digits[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const signed char base64val[] =
{
  BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,
  BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD,
  BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, BAD, 62, BAD, BAD, BAD, 63,
  52, 53, 54, 55,  56, 57, 58, 59,  60, 61, BAD, BAD, BAD, BAD, BAD, BAD,
  BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
  15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25, BAD, BAD, BAD, BAD, BAD,
  BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
  41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51, BAD, BAD, BAD, BAD, BAD
};
/** @endcond */
/* Static functions ==========================================================*/
/* Global functions ==========================================================*/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : azx_base64Encoder
// Purpose      : Converts a given string into a base64 encoded buffer.
// Last updated : 01/09/2005 15/05/2005
// Notes      :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void azx_base64Encoder( UINT8 *out, const UINT8 *in, int inlen )
{
  for( ; inlen >= 3; inlen -= 3 )
  {
    *out++ = base64digits[in[0] >> 2];
    *out++ = base64digits[( ( in[0] << 4 ) & 0x30 ) | ( in[1] >> 4 )];
    *out++ = base64digits[( ( in[1] << 2 ) & 0x3c ) | ( in[2] >> 6 )];
    *out++ = base64digits[in[2] & 0x3f];
    in += 3;
  }

  if( inlen > 0 )
  {
    unsigned char fragment;
    *out++ = base64digits[in[0] >> 2];
    fragment = ( in[0] << 4 ) & 0x30;

    if( inlen > 1 )
    {
      fragment |= in[1] >> 4;
    }

    *out++ = base64digits[fragment];
    *out++ = ( inlen < 2 ) ? '=' : base64digits[( in[1] << 2 ) & 0x3c];
    *out++ = '=';
  }

  *out = '\0';
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Function     : m2m_base64Decoder
// Purpose      : Converts a given base64 string into a bytes buffer.
// Notes      :
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int azx_base64Decoder( CHAR *out, const CHAR *in )
{
  int len = 0;
  register unsigned char digit1, digit2, digit3, digit4;

  if( in[0] == '+' && in[1] == ' ' )
  {
    in += 2;
  }

  if( *in == '\r' )
  {
    return( 0 );
  }

  do
  {
    digit1 = in[0];

    if( DECODE64( digit1 ) == BAD )
    {
      return( -1 );
    }

    digit2 = in[1];

    if( DECODE64( digit2 ) == BAD )
    {
      return( -1 );
    }

    digit3 = in[2];

    if( digit3 != '=' && DECODE64( digit3 ) == BAD )
    {
      return( -1 );
    }

    digit4 = in[3];

    if( digit4 != '=' && DECODE64( digit4 ) == BAD )
    {
      return( -1 );
    }

    in += 4;
    *out++ = ( DECODE64( digit1 ) << 2 ) | ( DECODE64( digit2 ) >> 4 );
    ++len;

    if( digit3 != '=' )
    {
      *out++ = ( ( DECODE64( digit2 ) << 4 ) & 0xf0 ) | ( DECODE64( digit3 ) >> 2 );
      ++len;

      if( digit4 != '=' )
      {
        *out++ = ( ( DECODE64( digit3 ) << 6 ) & 0xc0 ) | DECODE64( digit4 );
        ++len;
      }
    }
  }
  while( *in && *in != '\r' && digit4 != '=' );

  return ( len );
}
