/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


/**
  @file
    azx_base64.h
  @version 1.0.0
  @dependencies 

  @brief Base64 utilities
  @details
    Porting of base64 de/encoder

  @note
    Dependencies:
       m2mb_types.h

  @author Fabio Pintus

  @date
     11/08/2020
*/

/** \defgroup b64Usage  Base64 functions usage
    Functions that can be used to manipulate Base64 hashes
*/

#ifndef HDR_AZX_BASE64_H_
#define HDR_AZX_BASE64_H_

/* Global declarations =======================================================*/
/** @cond PRIVATE */
// Base 64 Related
#define AZX_DECODE64(c)  (isascii(c) ? base64val[c] : AZX_B64_BAD)
#define AZX_B64_BAD     -1

static const char base64digits[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const signed char base64val[] =
{
  AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD,
  AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD,
  AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, 62, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, 63,
  52, 53, 54, 55,  56, 57, 58, 59,  60, 61, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD,
  AZX_B64_BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
  15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD,
  AZX_B64_BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
  41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD, AZX_B64_BAD
};
/** @endcond */


/* Global typedefs ===========================================================*/

/* Global functions ==========================================================*/


/**

  @brief
    Encode string to base64 hash

  @details
    This function allows user calculate the base64 hash of provided string

  @param[out] out
        Allocated buffer that will contain the hash output
  @param[in] in
        Buffer to be encoded
  @param[in] inlen
        Length of the input buffer
  @return
        None

  @ingroup b64Usage
*/
/*-----------------------------------------------------------------------------------------------*/
void azx_base64Encoder( UINT8 *out, const UINT8 *in, int inlen );
/** \example{lineno} azx_base64_example.c
    This is a detailed example of base64 functions usage.
*/
/**

  @brief
    Decode base64 hash to buffer

  @details
    This function allows user decode the base64 hash to the original string

  @param[out] out
        Allocated buffer that will contain decoded string
  @param[in] in
        Base64 hash to be decoded

  @return
        Length of the decoded buffer

  @ingroup b64Usage
*/
/*-----------------------------------------------------------------------------------------------*/
int  azx_base64Decoder( CHAR *out, const CHAR *in );


#endif /* HDR_AZX_BASE64_H_ */
