/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
 azx_gnu_string.h

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

#ifndef HDR_AZX_GNU_STRING_H_
#define HDR_AZX_GNU_STRING_H_

#undef strdup
#undef strndup

/* Global defines ================================================================================*/

#define strdup azx_gnu_strdup
#define strndup azx_gnu_strndup

/* Function prototypes ====================================================================*/

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Copies the s string pointed by source into the array, including the terminating null character (and stopping at that point).

  @details


  @param[in] s
  	string pointed

  @return
	Returns the array source
  @note
    See strcpy

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
char * azx_gnu_strdup(const char * s);

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Copies the first n charactersand returns the source to destination.

  @details

  @param[in] s
  	string pointed

  @param[in] n
    number of character to copy

  @return
	Returns the source to destination.
  @note
    See strncpy

  @b
    Example
  @code
    <C code example>
  @endcode
*/
/*-----------------------------------------------------------------------------------------------*/
char * azx_gnu_strndup(const char * s, size_t n);

#endif /* HDR_AZX_GNU_STRING_H_ */
