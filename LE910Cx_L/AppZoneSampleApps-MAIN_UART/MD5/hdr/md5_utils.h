/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    md5_utils.h

  @brief
    The file contains MD5 utilities

  @details

  @note
    Dependencies:
    m2mb_types.h
    <stdio.h>
  @note


  @date
    2020/02/05
 */


#ifndef HDR_MD5_UTILS_H_
#define HDR_MD5_UTILS_H_


CHAR *md5_hashToString(UINT8 *hash, CHAR *out);

INT32 md5_isMD5(CHAR *md5string);

INT32 md5_compareHashes(UINT8 *hash_a, UINT8 *hash_b);

INT32 md5_compareHashWithString(UINT8 *hash, UINT16 hash_size, CHAR *string);

INT32 md5_computeFromFile(const CHAR *input_filename, UINT8 *md5_sum);

INT32 md5_computeSum(UINT8 *output_data, UINT8 *input_data, size_t data_size);


#endif /* HDR_MD5_UTILS_H_ */
