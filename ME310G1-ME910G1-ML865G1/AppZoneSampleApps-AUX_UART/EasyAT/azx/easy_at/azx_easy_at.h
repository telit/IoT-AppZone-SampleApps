/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
  azx_easy_at.h

  @brief
  atp utility

  @details
  This header file provides ATP (EASY AT) utilities to ease custom at commands usage

  @note
    Dependencies:
      m2mb_types.h
      m2mb_atp.h

  @version 1.0.1
  @dependencies azx_log azx_utils

  @author
   Fabio Pintus

  @date
  13/03/2020
*/

#ifndef HDR_AZX_EASY_AT_H_
#define HDR_AZX_EASY_AT_H_


/** @defgroup azx_easy_at Documentation for azx_easy_at functionalities
   @{
   This group includes all the information about azx_easy_at configuration and usage

   @}
*/


/* Global declarations ==========================================================================*/

#define AZX_EASY_AT_INVALID_AT_INSTANCE 0xFFFF

/**
  @brief
    This is the custom command easy at main callback function signature

  @details
    This callback function is executed by ATP when a custom AT command is executed.
    This will contain user code to manage the command

  @param[in] h
    the ATP handle associated with the command
  @param[in] atpI
    the atp instance where the command was received. Can be used internally with m2mb_atp_get_input_data
    to retrieve the command information (name, parameters list and so on)

  @return
        None

  <b>Refer to</b>
    azx_easy_at_init() AZX_EASY_AT_ATCOMMAND_T list

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
typedef void ( *azx_easy_at_taskCallback )( M2MB_ATP_HANDLE h, UINT16 atpI );

/**
  @brief
    This is the custom command easy at delegation callback function signature

  @details
    This callback function is executed by ATP when a custom AT command is in execution, and user data input
    is expected. This will contain user code to manage the command

  @param[in] h
    the ATP handle associated with the command
  @param[in] atpI
    the atp instance where the command was received. Can be used internally with m2mb_atp_get_input_data
    to retrieve the command information (name, parameters list and so on)
  @param[in] delegationEvent
    the delegation event (can be DATA or ESCAPE)
  @param[in] delegationMsgSize
    additional delegation callback data size in bytes
  @param[in] delegationMsg
    additional delegation callback data pointer


  @return
        None

  <b>Refer to</b>
    azx_easy_at_init() M2MB_EASY_AT_ATCOMMAND_T list

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
typedef void ( *azx_easy_at_taskDelegation )( M2MB_ATP_HANDLE h,  UINT16 atpI,
                                              M2MB_ATP_DELEGATION_IND_E delegationEvent, UINT16 msg_size, void *delegationMsg );


/**
  @struct AZX_EASY_AT_ATCOMMAND_TAG

  @brief Single custom command utility struct

  @details This structure holds the parameters to register a single AT command

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at

*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_EASY_AT_ATCOMMAND_TAG
{
  const CHAR *cmd;                        /**<The command name*/
  UINT16 atpFlags;                        /**<The ATP flags to be used for the command, e.g. M2MB_ATP_NORML|M2MB_ATP_NOPIN|M2MB_ATP_NOSIM */
  azx_easy_at_taskCallback Callback;              /**<Main command callback*/
  azx_easy_at_taskDelegation Delegation;  /**<Command delegation callback*/
  UINT8 isHidden;                         /**<Command is hidden will not be notified when registered*/
} AZX_EASY_AT_ATCOMMAND_T;            /**< Typedef of struct AZX_EASY_AT_ATCOMMAND_TAG*/


/**
  @struct EASY_AT_MODULE_TAG

  @brief The module structure retrieved by azx_easy_at_init()

  @details This structure holds useful info for the registered group of commands
  (intended as a software module) that will be used internally

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at

*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_EASY_AT_MODULE_TAG
{
  CHAR module_name[32];                 /**<the software module name (e.g. "NTP_AT" ). will be filled by azx_easy_at_init*/
  UINT16 last_AT_Instance;              /**<the last AT instance received for this module*/
  M2MB_ATP_HANDLE last_ATP_Handle;      /**<the last ATP handle received for this module*/
} AZX_EASY_AT_MODULE_T;                     /**< Typedef of struct AZX_EASY_AT_MODULE_TAG*/


/**
  @struct EASY_AT_HANDLES_TAG

  @brief Structure containing atpHandle and atpI info for a specific command

  @details This structure holds useful info for command in execution, allowing to pass it to other functions like macros
  It can be created inside the AT command main callback function

    <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at


  @code
  void MY_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI )
  {
    M2MB_ATP_PARAM_T *atpParam;
    m2mb_atp_get_input_data( atpHandle, atpI, &atpParam );
    EASY_AT_HANDLES_T hdls = {atpHandle, atpI};
    ...
    EASY_AT_MODULE_T *module;

    EASY_AT_RELEASE_WITH_SUCCESS(module, (p_hdls));
  }
  @endcode

*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_EASY_AT_HANDLES_TAG
{
  M2MB_ATP_HANDLE handle;           /**<ATP handle*/
  UINT16 atpI;                      /**<ATP Instance*/
} AZX_EASY_AT_HANDLES_T;                /**< Typedef of struct AZX_EASY_AT_HANDLES_TAG*/
/* Global typedefs ==============================================================================*/

/// @private
typedef HANDLE AZX_EASY_AT_TASK_HANDLE;   /**<ATP task Handle*/


/* Global functions =============================================================================*/



/*
   MACROS
 * */

#define AZX_EASY_AT_TRIM_LEADING_QUOTES(a) if('\"'==a[0]){a++;}
#define AZX_EASY_AT_TRIM_TRAILING_QUOTES(a) if('\"'==a[strlen(a)-1]){a[strlen(a)-1]=0;}
#define AZX_EASY_AT_TRIM_LEADING_AND_TRAILING_QUOTES(a) AZX_EASY_AT_TRIM_LEADING_QUOTES(a)AZX_EASY_AT_TRIM_TRAILING_QUOTES(a)


#ifndef AZX_EASY_AT_ARRAY_SIZE
  #define AZX_EASY_AT_ARRAY_SIZE(a) ( sizeof(a)/sizeof(a[0]) )
#endif

#define AZX_EASY_AT_TRACE_FATAL(a,...)    AZX_LOG_CRITICAL( a, ##__VA_ARGS__)
#define AZX_EASY_AT_TRACE_ERROR(a,...)    AZX_LOG_ERROR( a, ##__VA_ARGS__)
#define AZX_EASY_AT_TRACE_WARNING(a,...)  AZX_LOG_WARN( a, ##__VA_ARGS__)
#define AZX_EASY_AT_TRACE_INFO(a,...)     AZX_LOG_INFO( a, ##__VA_ARGS__)
#define AZX_EASY_AT_TRACE_DEBUG(a,...)    AZX_LOG_DEBUG( a, ##__VA_ARGS__)
#define AZX_EASY_AT_TRACE_DETAIL(a,...)   AZX_LOG_TRACE( a, ##__VA_ARGS__)



#define AZX_EASY_AT_ASSERT_REGISTER( result)\
  {\
    M2MB_RESULT_E _r = result;\
    if(_r == M2MB_RESULT_SUCCESS)\
      AZX_EASY_AT_TRACE_DETAIL("Command Registered!\r\n");\
    else {\
      AZX_EASY_AT_TRACE_ERROR("Cannot register! error: %d\r\n", result);\
      return _r;\
    }\
  }


#define AZX_EASY_AT_ASSERT_MSG( result)\
  {\
    if(result == M2MB_RESULT_SUCCESS)\
      AZX_EASY_AT_TRACE_DEBUG("Message sent out to parser!\r\n");\
    else \
      AZX_EASY_AT_TRACE_ERROR("Cannot send message out! error: %d\r\n", result);\
  }

#define AZX_EASY_AT_RELEASE_WITH_SUCCESS( h)\
  {\
    int ret;\
    if((ret = m2mb_atp_release((h)->handle, (h)->atpI, M2MB_ATP_FRC_OK, -1, NULL)) == M2MB_RESULT_SUCCESS){\
      AZX_EASY_AT_TRACE_DEBUG("Release SUCCESS was OK\r\n");\
    }\
    else {\
      AZX_EASY_AT_TRACE_ERROR("Cannot release! error: %d\r\n", ret);\
    }\
  }

#define AZX_EASY_AT_RELEASE_WITH_ERROR(h, e)\
  {\
    int ret;\
    if((ret = m2mb_atp_release((h)->handle, (h)->atpI, M2MB_ATP_FRC_ERROR, e, NULL)) == M2MB_RESULT_SUCCESS){\
      AZX_EASY_AT_TRACE_DEBUG("Release FAILURE was OK\r\n");\
    }\
    else {\
      AZX_EASY_AT_TRACE_ERROR("Cannot release! error: %d\r\n", ret);\
    }\
  }


#define AZX_EASY_AT_RELEASE_SILENT( h)\
  {\
    int ret;\
    if((ret = m2mb_atp_release((h)->handle, (h)->atpI, M2MB_ATP_FRC_SILENT, -1, NULL)) == M2MB_RESULT_SUCCESS){\
      AZX_EASY_AT_TRACE_DEBUG("Release silent was OK\r\n");\
    }\
    else {\
      AZX_EASY_AT_TRACE_ERROR("Cannot release! error: %d\r\n", ret);\
    }\
  }



#define AZX_EASY_AT_FORWARD_TO_PARSER(h,m)\
  {\
    int ret;\
    if((ret = m2mb_atp_forward_parser((h)->handle, (h)->atpI, (CHAR*) m)) == M2MB_RESULT_SUCCESS){\
      EASY_AT_TRACE_DEBUG("Forward return was OK\r\n");\
    }\
    else {\
      EASY_AT_TRACE_ERROR("Forward failed! error: %d\r\n", ret);\
    }\
    EASY_AT_TRACE_DEBUG("Releasing instance..\r\n");\
    RELEASE_SILENT(h);\
  }



/**
  @brief
    function to output an error message over ATP interface.

  @details
    This function will output an error message on ATP interface.
    The format of the message depends on the cmee level for that
    interface. Custom cme code and message can be provided.
    It is internally called with the macro AZX_EASY_AT_RELEASE_WITH_CMEE


  @param[in] hdls
     pointer to the structure with atp handle and atp instance info
  @param[in] cmee_code
     the numeric value of the error. can be one of the standard
  @param[in] level
     the custom message, if needed. Leave to NULL to use a standard message.
     It Supports variadic parameters


  @return
    M2MB_RESULT_E value

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
M2MB_RESULT_E azx_easy_at_CMEE( AZX_EASY_AT_HANDLES_T *hdls, INT16 cmee_code,
                                CHAR *cust_cmee_message, ... );

//h: pointer to handles struct
//c: cmee code
//m: custom message for cmee code and for app logging(if required)
#define AZX_EASY_AT_RELEASE_WITH_CMEE( h, c, m, ...)\
  {\
    M2MB_RESULT_E _int_res;\
    _int_res = azx_easy_at_CMEE(h, c, m, ##__VA_ARGS__); \
    if( _int_res != M2MB_RESULT_SUCCESS ){ /* failed */ AZX_EASY_AT_TRACE_ERROR( "Failed sending the CMEE message. _int_res: %d\r\n", _int_res); }\
  } /**<This macro will release the ATP instance for the called handle (h), using the cmee code (c)
and passing an optional custom message(m) with its parameters. See azx_easy_at_CMEE*/


/**
  @brief
    Provides the command type as a string.

  @details
    This function will output a string that describes the current command
    type. refer to M2MB_ATP_CMDTYPE_E enum. It is called by macro AZX_EASY_AT_CMD_INFO

  @param[in] type
     the command type as a number (retrieved by m2mb_atp_get_input_data returned structure)

  @return
    M2MB_RESULT_E value

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
const CHAR *azx_easy_at_cmdTypeAsString( UINT16 type );


#define AZX_EASY_AT_CMD_INFO(h)\
  {\
    M2MB_ATP_PARAM_T *atpParam;\
    m2mb_atp_get_input_data( (h)->handle, (h)->atpI, &atpParam );\
    AZX_EASY_AT_TRACE_INFO( "%s: type %s; itemnum: %d; instance: %d\r\n",\
                            atpParam->atpCmdString,\
                            azx_easy_at_cmdTypeAsString(atpParam->type), \
                            atpParam->itemNum, \
                            (h)->atpI);\
  } /**<This macro prints information about the currently running command, given the hdls structure pointer (h).
_module is the AZX_EASY_AT_MODULE pointer. See also azx_easy_at_cmdTypeAsString() */


/**
  @brief
    Send an URC message.

  @details
    This function will queue an URC message (buffering it) and send it either as a broadcast (no AT instance was
    ever used by this module (e.g. no AT commands were received yet) or to the last used AT instance.

  @param[in] module
     the module structure pointer
  @param[in] message
     the URC message.

  @return
    M2MB_RESULT_E value

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
M2MB_RESULT_E azx_easy_at_sendUnsolicited( AZX_EASY_AT_MODULE_T *module, CHAR *message );



/**
  @brief
    Initialize the easy AT module

  @details
    This function will create a module structure (with the provided module_name) and register the list of
    custom commands.

  @param[in] module_name
     the module name to be used. It will be showed in all trace prints for that module.
  @param[in] list
      pointer to the array of commands info
  @param[in] list_size
      size of the list (number of commands)

  @return
    structure pointer in case of success, NULL in case of failure


  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
AZX_EASY_AT_MODULE_T *azx_easy_at_init( CHAR *module_name,  AZX_EASY_AT_ATCOMMAND_T *list,
                                        INT32 list_size );


/**
  @brief
    Gets the ATE setting for the provided instance

  @details
    This function retrieve the ATE value for the instance passed with h

  @param[in] h
     the AZX_EASY_AT_HANDLES_T pointer, carrying the ATP instance info
  @param[out] ate
     pointer to the variable that will be filled with the ATE value

  @return
    M2MB_RESULT_E value

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
M2MB_RESULT_E azx_easy_at_getAte( AZX_EASY_AT_HANDLES_T *h, INT32 *ate );


/**
  @brief
    Gets the CME setting for the provided instance

  @details
    This function retrieve the CME value for the instance passed with h

  @param[in] h
     the AZX_EASY_AT_HANDLES_T pointer, carrying the ATP instance info
  @param[out] cmee
     pointer to the variable that will be filled with the CME value

  @return
    M2MB_RESULT_E value

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
M2MB_RESULT_E azx_easy_at_getCmee( AZX_EASY_AT_HANDLES_T *h, INT32 *cmee );



/*Utility functions*/


/**
  @brief
    modifies a string removing quotes in it.

  @details
    This function will modify the input string, removing all quotes it may
    contain. the string length is modified accordingly.

  @param[in] pStr
     pointer to the string

  @return
    the pointer to the string.

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
char *azx_easy_at_TrimAndRemoveQuotes( CHAR *pStr );


/**
  @brief
    Converts a string into a signed long.

  @details
    This function convert an input string into an INT32 number.

  @param[in] str
     pointer to the string containing the number
  @param[out] output
     pointer to the variable that will be filled with the value.

  @return 0 if success
  @return -1 if the string number is out of range for an INT32
  @return -2 if no digits were found
  @return -3 invalid parameters
  @return -4 if non numeric characters were found (different from 0-9 and +-)


  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_easy_at_strToL( CHAR *str, INT32 *output );

/**
  @brief
    Converts a string into an unsigned signed long.

  @details
    This function convert an input string into an UINT32 number.

  @param[in] str
     pointer to the string containing the number
  @param[out] output
     pointer to the variable that will be filled with the value.

  @return 0 if success
  @return -1 if the string number is out of range for an UINT32
  @return -2 if no digits were found
  @return -3 invalid parameters
  @return -4 if non numeric characters were found (different from 0-9 and -)


  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_easy_at_strToUL( CHAR *str, UINT32 *output );

/**
  @brief
    Converts a string into an unsigned long long.

  @details
    This function convert an input string into an UINT64 number.

  @param[in] str
     pointer to the string containing the number
  @param[out] output
     pointer to the variable that will be filled with the value.

  @return 0 if success
  @return -1 if the string number is out of range for an UINT64
  @return -2 if no digits were found
  @return -3 invalid parameters
  @return -4 if non numeric characters were found (different from 0-9 and -)


  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_easy_at_strToULL( CHAR *str, UINT64 *output );

/**
  @brief
    Converts a string in HEX format into an unsigned signed long.

  @details
    This function convert an input string in HEX format into an UINT32 number.

  @param[in] str
     pointer to the string containing the number
  @param[out] output
     pointer to the variable that will be filled with the value.

  @return 0 if success
  @return -1 if the string number is out of range for an UINT32
  @return -2 if no digits were found
  @return -3 invalid parameters
  @return -4 if non numeric characters were found (different from 0-9 and A-F )

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
INT32 azx_easy_at_strToULHex( CHAR *str, UINT32 *output );


/**
  @brief
    Converts a string into an unsigned short.

  @details
    This function convert an input string into an UINT8 number.

  @param[in] str
     pointer to the string containing the number
  @param[out] output
     pointer to the variable that will be filled with the value.

  @return 0 if success
  @return -1 if the string number is out of range for an UINT8
  @return -2 if no digits were found
  @return -3 invalid parameters
  @return -4 if non numeric characters were found (different from 0-9 and -)


  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
INT8 azx_easy_at_strToUS( CHAR *str, UINT8 *output );


/**
  @brief
   converts an ASCII hex character (0-9A-F) to its corresponding value.

  @details
    This function converts a single ASCII hex character (0-9A-F)
    to the corresponding value. E.g. 'F' -> 0x0F

  @param[in] ch
     the numeric value

  @return the converted value


  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
UINT8 azx_easy_at_ASCtoDEC( CHAR ch );



#endif /* HDR_EASYAT_UTILS_H_ */
