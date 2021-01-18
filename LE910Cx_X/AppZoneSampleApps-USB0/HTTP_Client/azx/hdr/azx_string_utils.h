/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */


#ifndef HDR_AZX_STRING_UTILS_H_
#define HDR_AZX_STRING_UTILS_H_
/**
  @file
    azx_string_utils.h
  @version 1.0.1
  @dependencies 

  @brief String related utilities

  Utitities to manage strings (e.g. convert to numbers, trim)


  @note
    Dependencies:
       m2mb_types.h

  @author
    Fabio Pintus

  @date
     11/08/2020
*/


/** \defgroup stringUsage String utilities usage
    Functions that can be used to manipulate strings
*/

/** \defgroup arrayUsage Array utilities usage
    Functions that can be used to manage arrays
*/

/** \addtogroup  stringUsage
  @{ */

/** \name Trim Macros
  \brief These macros can be used to trim strings removing quotes
   @{ */

/* Global declarations =======================================================*/


#ifndef AZX_TRIM_LEADING_QUOTES
  #define AZX_TRIM_LEADING_QUOTES(a) if('\"'==a[0]){a++;}  /**<Trim quotes at the beginning of the string*/
#endif
#ifndef AZX_TRIM_TRAILING_QUOTES
  #define AZX_TRIM_TRAILING_QUOTES(a) if('\"'==a[strlen(a)-1]){a[strlen(a)-1]=0;} /**<Trim quotes at the end of the string*/
#endif

#ifndef AZX_TRIM_LEADING_AND_TRAILING_QUOTES
  #define AZX_TRIM_LEADING_AND_TRAILING_QUOTES(a) TRIM_LEADING_QUOTES(a)TRIM_TRAILING_QUOTES(a) /**<Trim quotes at the beginning AND at the end of the string*/
#endif


/** @} */
/** @} */  //close addtogroup


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Returns the number of elements of an array
  @details

  @param[in] a
    array of elements

  @return
    An integer with the number of elements in the provided array

  <b>Sample usage</b>
  @code
    int my_array[] ={ 1,2,1,2};
    int array_elems = M2M_ARRAY_ELEMENTS(my_array); //this will be 4
  @endcode

  \ingroup arrayUsage
*/
/*-----------------------------------------------------------------------------------------------*/
#ifndef AZX_GET_ARRAY_ELEMENTS
  #define AZX_GET_ARRAY_ELEMENTS(a) ( sizeof(a)/sizeof(a[0]) )
#endif

/* Global typedefs ===========================================================*/

/* Global functions ==========================================================*/

/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a string to a signed long integer

  @details
    This function converts the input string in decimal format to a signed long integer

  @param[in] str
    input string containing the number to be converted
  @param[out] output
    variable where the converted number will be stored

  @return
    0    Ok
  @return
    -1    Out of range value
  @return
    -2    No digits were found
  @return
    -3    Invalid number input (e.g. an hex number was passed)

  <b>Sample usage</b>
  @code
    INT32 var;
    char string[] = "+12398";
    INT32 ret = azx_str_to_l(string, &var);
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_str_to_l( char *str, INT32 *output );


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a string to an unsigned long integer

  @details
    This function converts the input string in decimal format to an unsigned long integer

  @param[in] str
    input string containing the number to be converted
  @param[out] output
    variable where the converted number will be stored


  @return
    0    Ok
  @return
    -1    Out of range value
  @return
    -2    No digits were found.
  @return
    -3    Invalid number input (e.g. an hex number was passed)
  @return
    -4    unexpected error

  <b>Sample usage</b>
  @code
    UINT32 var;
    char string[] = "3147483642";
    INT32 ret = azx_str_to_ul(string, &var);
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_str_to_ul( char *str, UINT32 *output );


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a string to an unsigned long long integer

  @details
    This function converts the input string in decimal format to an unsigned long long integer (64 bits)

  @param[in] str
    input string containing the number to be converted
  @param[out] output
    variable where the converted number will be stored


  @return
    0    Ok
  @return
    -1    Out of range value
  @return
    -2    No digits were found.
  @return
    -3    Invalid number input (e.g. an hex number was passed)
  @return
    -4    unexpected error

  <b>Sample usage</b>
  @code
    UINT64 var;
    char string[] = "9223372036854775807";
    INT32 ret = azx_str_to_ull(string, &var);
    PRINT("%llu", var);  //NOTE: PRINT is a generic print function like printf
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_str_to_ull( char *str, UINT64 *output );


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a string in hex format to an unsigned long integer

  @details
    This function converts the input string in hex format to an unsigned long integer

  @param[in] str
    input string containing the number to be converted
  @param[out] output
    variable where the converted number will be stored

  @return
    0    Ok
   @return
    -1    Out of range value
  @return
    -2    No digits were found.
  @return
    -3    Invalid number input (e.g. unexpected characters were passed)
  @return
    -4    unexpected error

  <b>Sample usage</b>
  @code
    UINT32 var;
    char string[] = "3F4b";
    INT32 ret = azx_str_to_ul_hex(string, &var);
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_str_to_ul_hex( char *str, UINT32 *output );



/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a string in to an unsigned char integer

  @details
    This function converts the input stringto an unsigned char integer (8 bits)

  @param[in] str
    input string containing the number to be converted
  @param[out] output
    variable where the converted number will be stored


  @return
    0    Ok
   @return
    -1    Out of range value
  @return
    -2    No digits were found.
  @return
    -3    Invalid number input (e.g. unexpected characters were passed)
  @return
    -4    unexpected error

  <b>Sample usage</b>
  @code
    UINT8 var;
    char string[] = "129";
    INT32 ret = azx_str_to_uc(string, &var);
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT8 azx_str_to_uc( char *str, UINT8 *output );


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a hex ASCII character in to an unsigned char integer

  @details
    This function converts the input ASCII character (0-9 A-F) to the corresponding decimal value.
    E.g. 'F' -> 0x0F

  @param[in] ch
    input character containing the number to be converted

  @return
    0    failure
  @return
    converted value


  <b>Sample usage</b>
  @code
    UINT8 var;
    char c ='f';
    var = m2m_asc_to_dec(c);
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
UINT8 azx_asc_to_dec( char ch );
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a string in to a double

  @details
    This function converts the input stringto a double (64 bits)

  @param[in] str
    input string containing the number to be converted
  @param[out] output
    variable where the converted number will be stored


  @return
    0    Ok
   @return
    -1    Out of range value
  @return
    -2    No digits were found.
  @return
    -3    Invalid number input (e.g. unexpected characters were passed)
  @return
    -4    unexpected error


  <b>Sample usage</b>
  @code
    FLOAT64 var;
    char string[] = "686.97";
    INT32 ret = azx_str_to_d(string, &var);
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT8 azx_str_to_d( char *str, FLOAT64 *output );
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Converts a string in to a float

  @details
    This function converts the input stringto a float (32 bits)

  @param[in] str
    input string containing the number to be converted
  @param[out] output
    variable where the converted number will be stored

  @return
    0    Ok
   @return
    -1    Out of range value
  @return
    -2    No digits were found.
  @return
    -3    Invalid number input (e.g. unexpected characters were passed)
  @return
    -4    unexpected error


  <b>Sample usage</b>
  @code
    FLOAT32 var;
    char string[] = "686.97";
    INT32 ret = azx_str_to_f(string, &var);
  @endcode

  \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
INT8 azx_str_to_f( char *str, FLOAT32 *output );
/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Remove all the character recurrences from the given string

  @details
     Remove all the character recurrences from the given string

  @param[in] str
    input string
  @param[in] chr
    character to be removed

  @return
    0    Ok

    \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
UINT8 azx_str_rem_ch( char *str, char chr );


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Remove all the blank leading spaces

  @details
     Remove all the blank leading spaces. Resize the input string accordingly

  @param[in] str
    input string

  @return
    0    Ok

    \ingroup stringUsage
*/
/*-----------------------------------------------------------------------------------------------*/
UINT8 azx_str_l_trim( char *str );


/*-----------------------------------------------------------------------------------------------*/
/**
  @brief
    Remove all the trailing spaces

  @details
     Remove all the trailing spaces. Resize the input string accordingly

  @param[in] str
    input string

  @return
    0    Ok

*/
/*-----------------------------------------------------------------------------------------------*/
UINT8 azx_str_r_trim( char *str );


#endif /* HDR_AZX_STRING_UTILS_H_ */
