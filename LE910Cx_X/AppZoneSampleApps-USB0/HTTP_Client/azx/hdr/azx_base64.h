/**
  @file
    azx_base64.h
  @version 1.1.1
  @dependencies 

  @brief Base64 utilities
  @details
    Porting of base64 de/encoder from https://sources.debian.org/src/fetchmail/6.4.16-1/base64.c
    refer to COPYING-base64 file in this folder for attributions

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
