/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    at_common.h

  @brief
    The file contains common functions for custom commands

  @author


  @date
    13/03/2020
*/

#ifndef HDR_AT_COMMON_H_
#define HDR_AT_COMMON_H_


#define CMD_MODULE_NAME "M2M_AT"
#define EASY_AT_INVALID_AT_INSTANCE 0xFFFF

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
    easy_at_init() EASY_AT_ATCOMMAND_T list


*/
/*-----------------------------------------------------------------------------------------------*/

typedef void ( *easy_at_taskCallback )( M2MB_ATP_HANDLE h, UINT16 atpI );



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
    easy_at_init() M2MB_EASY_AT_ATCOMMAND_T list

*/
/*-----------------------------------------------------------------------------------------------*/
typedef void ( *easy_at_taskDelegation )( M2MB_ATP_HANDLE h,  UINT16 atpI,
                                              M2MB_ATP_DELEGATION_IND_E delegationEvent, UINT16 msg_size, void *delegationMsg );


/**
  @struct EASY_AT_ATCOMMAND_TAG

  @brief Single custom command utility struct

  @details This structure holds the parameters to register a single AT command

  <b>Refer to</b>
    easy_at_init()

 */
/*-----------------------------------------------------------------------------------------------*/
typedef struct EASY_AT_ATCOMMAND_TAG
{
  const CHAR *cmd;                        /**<The command name*/
  UINT16 atpFlags;                        /**<The ATP flags to be used for the command, e.g. M2MB_ATP_NORML|M2MB_ATP_NOPIN|M2MB_ATP_NOSIM */
  easy_at_taskCallback Callback;              /**<Main command callback*/
  easy_at_taskDelegation Delegation;  /**<Command delegation callback*/
  UINT8 isHidden;                         /**<Command is hidden will not be notified when registered*/
} EASY_AT_ATCOMMAND_T;



/**
  @struct EASY_AT_TASK_HANDLE_TAG

  @brief Task handle internal structure

  @details This structure holds the required info to manage the task

  <b>Refer to</b>
    easy_at_init()


*/
/*-----------------------------------------------------------------------------------------------*/

typedef struct EASY_AT_MODULE_TAG
{
  CHAR module_name[32];                 /**<the software module name (e.g. "NTP_AT" ). will be filled by azx_easy_at_init*/
  UINT16 last_AT_Instance;              /**<the last AT instance received for this module*/
  M2MB_ATP_HANDLE last_ATP_Handle;      /**<the last ATP handle received for this module*/
} EASY_AT_MODULE_T;



typedef struct EASY_AT_TASK_HANDLE_TAG
{
  M2MB_OS_TASK_HANDLE easy_at_taskHandle;     /**< The task handle*/
  M2MB_OS_Q_HANDLE easy_at_taskQueue;         /**< The task associated queue handle*/
  EASY_AT_MODULE_T *module;                   /**< The easy at module pointer (used for prints)*/
} EASY_AT_TASK_HANDLE_T;

typedef HANDLE EASY_AT_TASK_HANDLE;   /**<ATP task Handle*/

/**
  @struct AZX_EASY_AT_TASK_USERDATA_TAG

  @brief Structure holding all the parameters for a single command execution

  @details This structure holds the required info to manage the command

  <b>Refer to</b>
    azx_easy_at_init(_init()

  @ingroup  azx_easy_at

*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct EASY_AT_TASK_USERDATA_TAG
{
  M2MB_OS_TASK_HANDLE easy_at_taskHandle;      /**< The task handle*/
  M2MB_OS_Q_HANDLE easy_at_taskQueue;          /**< The task associated queue handle*/
  easy_at_taskCallback taskCallback;       /**< The easy at command main callback*/
  easy_at_taskDelegation taskDelegation;   /**< The easy at command delegation callback*/
  EASY_AT_MODULE_T *module;               /**< The easy at module pointer (used for prints)*/
} EASY_AT_TASK_USERDATA_T;                 /**< Typedef of struct AZX_EASY_AT_TASK_USERDATA_TAG*/



/**
  @brief
    static - group callback used when a new registered command is executed.

  @details
    This function is the global callback for all new registered commands.
    When a command is called from the AT interface, this callback is executed.

    refer to m2mb_atp_cmd_ind_callback for parameters description.

  @return None

  <b>Refer to</b>
    azx_easy_at_init() _azx_easy_at_cmdRegister

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
void _easy_at_task_group_callback( M2MB_ATP_HANDLE atpHandle,
                                              M2MB_ATP_CMD_IND_E atpEvent, UINT16 resp_size, void *resp_struct, void *easy_at_taskUserdata );

void set_at_module(EASY_AT_MODULE_T *module );
EASY_AT_MODULE_T *get_at_module( void );

INT32 my_cmds_at_init( void );
CHAR *TrimAndRemoveQuotes( CHAR *pStr );
INT32 strToUL( CHAR *str, UINT32 *output );


#endif /* HDR_AT_COMMON_H_ */
