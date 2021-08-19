/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    md5_utils.c

  @brief
    The file contains MD5 utilities

  @details

  @description
    MD5 wrappers for m2mb_crypto APIs

  @note

  @date
    2020/02/05
 */

/* Include files ================================================================================*/

#include "m2mb_types.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_fs_errno.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_crypto.h"


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>



#define MD5_DIGEST_LENGTH 16 /**< Default MD5 digest size in bytes */


#define MD5_FILE_CHUNK 5000  /**< Default file chunk size in bytes */

/**
 * @brief Converts a hash into a printable string
 *
 * This function converts the input hash into a printable string in hex format.
 *
 *
 * @param[in] hash  Pointer to the hash binary buffer
 * @param[out] out  Pointer to the allocated buffer that will be used to store the conversion.
 *  *
 * @return out pointer
 *
 *
 * @see md5_computeSum md5_computeFromFile
 */
CHAR *md5_hashToString(UINT8 *hash, CHAR *out)
{
  int i,j;
  j=0;
  for(i=0; i < MD5_DIGEST_LENGTH; i++)
  {
    sprintf(out+j, "%02x", hash[i]);
    j+=2;
  }
  return out;
}


/**
 * @brief Compares a hash with a string containing a hash
 *
 * This function checks if the input hash is equal to the input string
 *
 *
 * @param[in] hash      Pointer to the hash binary buffer
 * @param[in] hash_size Size in bytes of hash buffer (it should be MD5_DIGEST_LENGTH )
 * @param[in] string    Pointer to the string containing the hash
 *
 * @return 1 if hashes are the same
 * @return 0 if they are different
 *
 *
 * @see md5_computeSum md5_computeFromFile
 */
INT32 md5_compareHashWithString(UINT8 *hash, UINT16 hash_size, CHAR *string)
{
  int i;
  const int str_len = strlen(string);
  if(str_len > (hash_size * 2) )
  {
    return 0;
  }
  UINT8 byte = 0;

  for (i = 0; i < (str_len / 2); i++)
  {
    if(1 != sscanf((string + 2 * i), "%02hhx", &byte))
    {
      return 0;
    }

    if (byte != hash[i])
    {
      return 0;
    }
  }

  return 1;
}

/**
 * @brief Compares two binary hashes
 *
 * This function checks if the input hashes are equal
 *
 *
 * @param[in] hash_a    Pointer to the first hash binary buffer
 * @param[in] hash_b    Pointer to the second hash binary buffer
 *
 * @return 1 if hashes are the same
 * @return 0 if they are different
 *
 *
 * @see md5_computeSum md5_computeFromFile
 */
INT32 md5_compareHashes(UINT8 *hash_a, UINT8 *hash_b)
{
  int i;

  for (i = 0; i < MD5_DIGEST_LENGTH ; i++)
  {
    if (hash_a[i] != hash_b[i])
    {
      return 0;
    }
  }
  return 1;
}

/**
 * @brief Checks if a string is in MD5 format
 *
 * This function checks if the input string is in MD5 format
 *
 *
 * @param[in] md5string    Pointer to string to be checked
 *
 * @return 1 if hashes are the same
 * @return -1 string length is 33 and last char is not <CR>
 * @return -2 string length is 34 and last characters are not <CRLF>
 * @return -3 string is not a possible hash
 *
 *
 * @see md5_computeSum md5_computeFromFile
 */
INT32 md5_isMD5(CHAR *md5string)
{
  int i;

  switch(strlen(md5string))
  {
  case 33:
    if('\r' != md5string[32])
    {
      return -1;
    }
    break;
  case 34:
    if('\r' != md5string[32] && '\n' != md5string[33])
    {
      return -2;
    }
  }

  for(i = 0; i < 32; i++)
  {
    if(!isxdigit((unsigned char) md5string[i]))
    {
      return -3;
    }
  }

  return 1;

}


/**
 * @brief Computes a MD5 hash from a local file
 *
 * This function computes the MD5 hash using a local file content as input
 *
 *
 * @param[in] input_filename    Input file name to be hashed
 * @param[out] md5_sum          Pointer to the allocated binary buffer that will contain the hash
 *
 * @return 0  if hash has been computed successfully
 * @return -1 if input filename is null
 * @return -2 if input filename is empty
 * @return -3 if it was not possible to allocate the chunk to read file content
 * @return -4 if it was not possible to init crypto md context
 * @return -5 if it was not possible to configure md context
 * @return -6 if it was not possible to open input file
 * @return -7 if if was not possible to close input file
 *
 * @see md5_computeSum
 */
INT32 md5_computeFromFile(const CHAR *input_filename, UINT8 *md5_sum)
{

  INT8 ret = 0;
  M2MB_CRYPTO_MD_CONTEXT MD5;
  unsigned char md5_digest[16];

  if(NULL == input_filename)
  {
    ret = -1;
  }
  else if(!strcmp(input_filename, "")) /* if filename is empty return */
  {
    ret = -2;
  }
  else
  {
    char *data = (char*) m2mb_os_malloc(MD5_FILE_CHUNK);
    if(NULL == data)
    {
      ret = -3;
    }
    else
    {
      if(M2MB_RESULT_SUCCESS !=  m2mb_crypto_md_init( &MD5 ))
      {
        ret = -4;
      }
      else
      {
        if( M2MB_RESULT_SUCCESS != m2mb_crypto_md_setup( MD5, M2MB_CRYPTO_MD_ALG_MD5))
        {
          ret = -5;
        }
        else
        {
          memset(data,0, MD5_FILE_CHUNK);

          M2MB_FILE_T *file_handle = m2mb_fs_fopen(input_filename, "rb");
          if(NULL == file_handle)
          {
            ret = -6;
          }
          else
          {
            UINT32 datalen;
            do
            {
              datalen = m2mb_fs_fread(data, sizeof(unsigned char), MD5_FILE_CHUNK, file_handle);
              m2mb_crypto_md_update(MD5, (const UINT8 *)data, datalen);
            }while (datalen != 0);


            if (0 != m2mb_fs_fclose(file_handle))
            {
              ret = -7;
            }
            else
            {
              m2mb_crypto_md_final(MD5, (UINT8 *) md5_digest);
              memcpy(md5_sum, md5_digest, MD5_DIGEST_LENGTH);
              /* SUCCESS */
            }

          }
        }
      }
      m2mb_os_free(data);
      data = NULL;
    }

  }
  return ret;
}


/**
 * @brief Computes a MD5 hash from a buffer
 *
 * This function computes the MD5 hash using a local buffer as input
 *
 *
 * @param[in] input_data        Input data to be hashed
 * @param[out] output_data      Pointer to the allocated binary buffer that will contain the hash
 * @param[in] data_size         Size of input buffer
 *
 * @return 0  if hash has been computed successfully
 * @return -1 if it was not possible to init crypto md context
 * @return -2 if it was not possible to configure md context
 *
 * @see md5_computeFromFile
 */
INT32 md5_computeSum(UINT8 *output_data, UINT8 *input_data, size_t data_size)
{
  UINT32 i;
  UINT8 MD5_res[MD5_DIGEST_LENGTH];

  UINT32 chunksize = MD5_FILE_CHUNK;

  UINT32 rest = data_size % chunksize;
  UINT32 chunks = (UINT32)data_size / (UINT32)chunksize;

  M2MB_CRYPTO_MD_CONTEXT ctx;


  if(M2MB_RESULT_SUCCESS !=  m2mb_crypto_md_init( &ctx ))
  {
    return -1;
  }
  if( M2MB_RESULT_SUCCESS != m2mb_crypto_md_setup( ctx, M2MB_CRYPTO_MD_ALG_MD5))
  {
    return -2;
  }

  /* split chunks */
  m2mb_crypto_md_update(ctx, (const UINT8 *)input_data, rest);
  input_data = input_data + rest;
  for(i = 0; i < chunks; i++)
  {
    m2mb_crypto_md_update(ctx, (const UINT8 *)input_data, chunksize);
    input_data = input_data + chunksize;
  }

  m2mb_crypto_md_final(ctx, MD5_res);

  memcpy(output_data, MD5_res, MD5_DIGEST_LENGTH);

  return 0; //success
}
