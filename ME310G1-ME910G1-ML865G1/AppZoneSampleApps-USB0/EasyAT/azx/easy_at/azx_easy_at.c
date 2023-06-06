/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
  azx_at_utils.c

  @brief
  atp utilities lib

  @details
  This header file provides ATP (AZX EASY AT) utilities to ease custom at commands usage

  @note
    Dependencies:
      m2mb_types.h
      m2mb_atp.h


  @author
   Fabio Pintus

  @date
  13/03/2020
*/
/* Include files ================================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include "m2mb_types.h"

#include "m2mb_os_api.h"
#include "m2mb_atp.h"

#include "azx_log.h"
#include "app_cfg.h"

#include "azx_easy_at.h"


/* Local defines ================================================================================*/

#ifdef __ARMCLIB_VERSION  /*Using RVCT ARM compiler*/
  #define ULONG_LONG_MAX ULLONG_MAX
#endif

#ifndef ULONG_LONG_MAX
  #define ULONG_LONG_MAX ULLONG_MAX
#endif


#ifndef ATP_THREAD_PRIORITY

#define ATP_THREAD_PRIORITY 231

#endif

#ifndef ATP_THREAD_STACK_SIZE

#define ATP_THREAD_STACK_SIZE (16*1024)

#endif

/* Local typedefs ===============================================================================*/

/**
  @struct AZX_EASY_AT_TASK_HANDLE_TAG

  @brief Task handle internal structure

  @details This structure holds the required info to manage the task

  <b>Refer to</b>
    azx_easy_at_init()

  @ingroup  azx_easy_at

*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_EASY_AT_TASK_HANDLE_TAG
{
  M2MB_OS_TASK_HANDLE azx_easy_at_taskHandle;     /**< The task handle*/
  M2MB_OS_Q_HANDLE azx_easy_at_taskQueue;         /**< The task associated queue handle*/
  AZX_EASY_AT_MODULE_T *module;                   /**< The easy at module pointer (used for prints)*/
} AZX_EASY_AT_TASK_HANDLE_T;                      /**< Typedef of struct AZX_EASY_AT_TASK_HANDLE_TAG*/


/**
  @struct AZX_EASY_AT_TASK_USERDATA_TAG

  @brief Structure holding all the parameters for a single command execution

  @details This structure holds the required info to manage the command

  <b>Refer to</b>
    azx_easy_at_init(_init()

  @ingroup  azx_easy_at

*/
/*-----------------------------------------------------------------------------------------------*/
typedef struct AZX_EASY_AT_TASK_USERDATA_TAG
{
  M2MB_OS_TASK_HANDLE azx_easy_at_taskHandle;      /**< The task handle*/
  M2MB_OS_Q_HANDLE azx_easy_at_taskQueue;          /**< The task associated queue handle*/
  azx_easy_at_taskCallback taskCallback;       /**< The easy at command main callback*/
  azx_easy_at_taskDelegation taskDelegation;   /**< The easy at command delegation callback*/
  AZX_EASY_AT_MODULE_T *module;               /**< The easy at module pointer (used for prints)*/
} AZX_EASY_AT_TASK_USERDATA_T;                 /**< Typedef of struct AZX_EASY_AT_TASK_USERDATA_TAG*/




/* Local statics ================================================================================*/


static const CHAR *azx_easy_at_cmd_type[] =
{
  "SET",    /* 0  */
  "READ",   /* 1  */
  "TEST",   /* 2  */
  "EXEC",   /* 3  */
  "NOPAR"   /* 4  */
};

static const CHAR *azx_easy_at_task_msg_event_type[] =
{
  "CMD",    /* 0  */
  "DELEG",   /* 1  */
};

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/



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
static void _azx_easy_at_task_group_callback( M2MB_ATP_HANDLE atpHandle,
                                              M2MB_ATP_CMD_IND_E atpEvent, UINT16 resp_size, void *resp_struct, void *azx_easy_at_taskUserdata );


/**
  @brief
    static - the ATP commands managing task entry function

  @details
    This is the entry function of the task that will handle all the registered commands.
    Inside, it will wait for messages on the task queue and execute the callback registered for each command
    during azx_easy_at_init().

  @param[in] pArg
    a pointer to the AZX_EASY_AT_TASK_HANDLE_T structure holding the task info, casted as void *.

  @return None

  <b>Refer to</b>
    _azx_easy_at_task_init()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
static void _azx_easy_at_task_entry( void *pArg );


/**
  @brief
    static - creates the task that will handle the command for this module.

  @details
    This is the function that creates the internal task to be used to manage custom commands.

  @param[in] module
    a pointer to the module created with azx_easy_at__init
  @param[out] handle
    pointer to the task structure handle that will be created.
  @param[in] priority
    task priority, 1 to 231. if 0 the minimum is set (231).
  @param[in] stackSize
    size of the reserved stack for the task, in bytes.

  @return None

  <b>Refer to</b>
    azx_easy_at__init()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
static M2MB_RESULT_E _azx_easy_at_task_init( AZX_EASY_AT_MODULE_T *module,
                                             AZX_EASY_AT_TASK_HANDLE *handle, UINT16 priority, UINT32 stackSize );


/**
  @brief
    static - registers a single command for the provided module

  @details
    This is the function that registers a custom command.

  @param[in] module
    a pointer to the module created with easy_at_init
  @param[in] atpHandle
    the atpHandle created with m2mb_atp_init
  @param[in] atpCmdString
    the command string
  @param[in] atpFlags
    Flags to be activated for the specific command, e.g. M2MB_ATP_NORML. May be ORed
  @param[in] atpTaskCallback
    The main command callback, executed inside the dedicated task. This is the callback that handles
    the command behavior.
  @param[in] atpTaskDelegation
    The delegation callback, executed inside the dedicated task, that will be executed to manage user input data.
  @param[in] azx_easy_at_taskHandle
    The handle of the created task that will execute the command

  @return
   M2MB_RESULT_E value

  <b>Refer to</b>
    azx_easy_at_init() _easy_at_registerAllCommands()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
static M2MB_RESULT_E _azx_easy_at_cmdRegister( AZX_EASY_AT_MODULE_T *module,
                                               M2MB_ATP_HANDLE atpHandle,
                                               CHAR *atpCmdString, UINT16 atpFlags, azx_easy_at_taskCallback azx_easy_at_taskCallback,
                                               azx_easy_at_taskDelegation azx_easy_at_taskDelegation,
                                               AZX_EASY_AT_TASK_HANDLE azx_easy_at_taskHandle );


/**
  @brief
    static - registers a list of commands for the provided module

  @details
    This is the function that registers the list of commands passed to
    easy_at_init

  @param[in] module
    a pointer to the module created with easy_at_init
  @param[in] atpH
    the atpHandle created with m2mb_atp_init
  @param[in] list
    The list of commands
  @param[in] list_size
    number of commands in the list
  @param[in] appAtptaskH
    The handle of the created task that will execute the commands

  @return
   M2MB_RESULT_E value

  <b>Refer to</b>
    azx_easy_at_init() _easy_at_cmdRegister()

  @ingroup  azx_easy_at
*/
/*-----------------------------------------------------------------------------------------------*/
static M2MB_RESULT_E _azx_easy_at_registerAllCommands( AZX_EASY_AT_MODULE_T *module,
                                                       M2MB_ATP_HANDLE atpH,
                                                       AZX_EASY_AT_ATCOMMAND_T *list, INT32 list_size, AZX_EASY_AT_TASK_HANDLE appAtptaskH );




/* Global functions =============================================================================*/


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
const CHAR *azx_easy_at_cmdTypeAsString( UINT16 type )
{
  if( type >= M2MB_NUM_ATP_CMDTYP )
  {
    return NULL;
  }
  else
  {
    return azx_easy_at_cmd_type[type];
  }
}


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
M2MB_RESULT_E azx_easy_at_sendUnsolicited( AZX_EASY_AT_MODULE_T *module, CHAR *message )
{
  if( ( module == NULL ) || ( message == NULL ) )
  {
    return M2MB_RESULT_INVALID_ARG;
  }

  AZX_EASY_AT_TRACE_INFO( "Sending AT URC <%s>\r\n", message );

  if( AZX_EASY_AT_INVALID_AT_INSTANCE ==
      module->last_AT_Instance ) //at instance was never used, we cannot reply to a single instance -> broadcast
  {
    return m2mb_atp_unsolicited_broadcast( module->last_ATP_Handle, message,
                                           M2MB_ATP_UNS_BUFFER_IF_BUSY );
  }
  else
  {
    /*Print the URC message over the AT instance*/
    return m2mb_atp_unsolicited_instance( module->last_ATP_Handle, module->last_AT_Instance, message,
                                          M2MB_ATP_UNS_BUFFER_IF_BUSY );
  }
}


/**
  @brief
    function to output an error message over ATP interface.

  @details
    This function will output an error message on ATP interface.
    The format of the message depends on the cmee level for that
    interface. Custom cme code and message can be provided.
    It is internally called with the macro EASY_AT_RELEASE_WITH_CMEE

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
                                CHAR *cust_cmee_message, ... )
{
  M2MB_RESULT_E res;
  CHAR  buf[200];
  int   bufSize = sizeof( buf );

  if( ! hdls )
  {
    return M2MB_RESULT_INVALID_ARG;
  }

  memset( buf, 0, bufSize );

  if( cust_cmee_message )
  {
    va_list arg;
    va_start( arg, cust_cmee_message );
    vsnprintf( buf, bufSize, cust_cmee_message, arg );
    va_end( arg );
    AZX_EASY_AT_TRACE_DEBUG( "CMEE message: <%s>\r\n", buf );
    res = m2mb_atp_release( hdls->handle, hdls->atpI, M2MB_ATP_FRC_CME_ERROR, cmee_code, buf );
  }
  else
  {
    res = m2mb_atp_release( hdls->handle, hdls->atpI, M2MB_ATP_FRC_CME_ERROR, cmee_code, NULL );
  }

  return res;
}


static void _azx_easy_at_task_group_callback( M2MB_ATP_HANDLE atpHandle,
                                              M2MB_ATP_CMD_IND_E atpEvent, UINT16 resp_size, void *resp_struct, void *azx_easy_at_taskUserdata )
{
  M2MB_OS_RESULT_E osRes = M2MB_OS_QUEUE_FULL;
  AZX_EASY_AT_TASK_USERDATA_T *tmp_azx_easy_at_taskUserdata;
  M2MB_ATP_REQ_T Message;

  if( azx_easy_at_taskUserdata != NULL )
  {
    tmp_azx_easy_at_taskUserdata = ( AZX_EASY_AT_TASK_USERDATA_T * )azx_easy_at_taskUserdata;
    Message.atpHandle = atpHandle;
    Message.atpEvent = atpEvent;
    Message.resp_size = resp_size;
    /*copy the message resp struct*/
    Message.resp_struct = m2mb_os_malloc( resp_size );

    if( Message.resp_struct != NULL )
    {
      memcpy( Message.resp_struct, resp_struct, resp_size );
      Message.atptaskUserdata = azx_easy_at_taskUserdata;
      osRes = m2mb_os_q_tx( tmp_azx_easy_at_taskUserdata->azx_easy_at_taskQueue, ( void * )&Message,
                            M2MB_OS_NO_WAIT, 0 );

      if( osRes != M2MB_OS_SUCCESS )
      {
        AZX_EASY_AT_TRACE_ERROR( "Send message to ATP queue failed; error %d", osRes );
      }
    }
  }
}


static void _azx_easy_at_task_entry( void *pArg )
{
  AZX_EASY_AT_TASK_HANDLE_T *atpTaskHandle = ( AZX_EASY_AT_TASK_HANDLE_T * )pArg;
  AZX_EASY_AT_TASK_USERDATA_T *azx_easy_at_taskUserdata;
  M2MB_ATP_REQ_T Message;

  while( 1 )
  {
    m2mb_os_q_rx( atpTaskHandle->azx_easy_at_taskQueue, &Message, M2MB_OS_WAIT_FOREVER );
    azx_easy_at_taskUserdata = ( AZX_EASY_AT_TASK_USERDATA_T * )Message.atptaskUserdata;
    //atpTaskHandle->module
    AZX_EASY_AT_TRACE_DEBUG( "task entry, message event type: %s\r\n",
                             azx_easy_at_task_msg_event_type[Message.atpEvent] );
    //M2M_LOG_DEBUG_MORE("event type: %s\r\n", atp_task_msg_event_type[Message.atpEvent] );
    atpTaskHandle->module->last_ATP_Handle = Message.atpHandle;

    switch( Message.atpEvent )
    {
      case M2MB_ATP_CMD_CALLBACK_IND:
      {
        M2MB_ATP_CALLBACK_IND_T *tmp_resp = ( M2MB_ATP_CALLBACK_IND_T * )Message.resp_struct;

        if( ( tmp_resp != NULL ) && ( azx_easy_at_taskUserdata != NULL ) )
        {
          atpTaskHandle->module->last_AT_Instance = tmp_resp->instanceNumber;
          azx_easy_at_taskUserdata->taskCallback( Message.atpHandle, tmp_resp->instanceNumber );
        }

        break;
      }

      case M2MB_ATP_CMD_DELEGATION_IND:
      {
        //FIXME check if atpTaskDelegation is ! NULL, use a generic "doing"
        M2MB_ATP_DELEGATION_IND_T *tmp_resp = ( M2MB_ATP_DELEGATION_IND_T * )Message.resp_struct;

        if( ( tmp_resp != NULL ) && ( azx_easy_at_taskUserdata != NULL ) )
        {
          atpTaskHandle->module->last_AT_Instance = tmp_resp->instanceNumber;

          if( azx_easy_at_taskUserdata->taskDelegation )
          {
            azx_easy_at_taskUserdata->taskDelegation( Message.atpHandle, tmp_resp->instanceNumber,
                                                      tmp_resp->event, tmp_resp->msg_size, tmp_resp->msg );
          }
        }

        break;
      }

      default:
        break;
    }

    if( Message.resp_struct )
    {
      m2mb_os_free( Message.resp_struct );
    }
  }
}


static M2MB_RESULT_E _azx_easy_at_task_init( AZX_EASY_AT_MODULE_T *module,
                                             AZX_EASY_AT_TASK_HANDLE *handle, UINT16 priority, UINT32 stackSize )
{
  M2MB_OS_RESULT_E osRes;
  M2MB_OS_TASK_ATTR_HANDLE taskAttr;
  M2MB_OS_Q_ATTR_HANDLE qAttrHandle;
  AZX_EASY_AT_TASK_HANDLE_T *newHandler;
  UINT32 msgSize = sizeof( M2MB_ATP_REQ_T );
  static UINT8 QueueArea[100 * 4 * 4];
  UINT16 prio;
  UINT32 stack;
  AZX_EASY_AT_TRACE_DETAIL( "ATPTASK : _azx_easy_at_task_init %d\r\n", msgSize );
  msgSize = ( msgSize / 4 );
  AZX_EASY_AT_TRACE_DETAIL( "msgSize %d\r\n", msgSize );
  osRes = m2mb_os_q_setAttrItem( &qAttrHandle, 1, M2MB_OS_Q_SEL_CMD_CREATE_ATTR,  NULL );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_EASY_AT_TRACE_DEBUG( "ATPTASK : Create queue attributes failed; error %d\r\n", osRes );
    return M2MB_RESULT_FAIL;
  }

  //QueueArea = m2mb_os_malloc( 100 * msgSize * 4);
  osRes = m2mb_os_q_setAttrItem( &qAttrHandle,
                                 CMDS_ARGS
                                 (
                                   M2MB_OS_Q_SEL_CMD_NAME, "ATPTASKQueue",
                                   M2MB_OS_Q_SEL_CMD_QSTART, &QueueArea,
                                   M2MB_OS_Q_SEL_CMD_MSG_SIZE, msgSize,
                                   M2MB_OS_Q_SEL_CMD_QSIZE, sizeof( QueueArea )
                                 )
                               );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_EASY_AT_TRACE_DEBUG( "ATPTASK : Set queue attributes failed; error %d\r\n", osRes );
    return M2MB_RESULT_FAIL;
  }

  newHandler = ( AZX_EASY_AT_TASK_HANDLE_T * )m2mb_os_malloc( sizeof( AZX_EASY_AT_TASK_HANDLE_T ) );

  if( newHandler == NULL )
  {
    return M2MB_RESULT_FAIL;
  }

  osRes = m2mb_os_q_init( &( newHandler->azx_easy_at_taskQueue ), &qAttrHandle );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_EASY_AT_TRACE_DEBUG( "ATPTASK : Queue init failed; error %d", osRes );
    m2mb_os_free( newHandler );
    return M2MB_RESULT_FAIL;
  }

  AZX_EASY_AT_TRACE_DETAIL( "ATPTASK : Thread creation\r\n" );

  if( priority == 0 )
  {
    prio = ATP_THREAD_PRIORITY;
  }
  else
  {
    prio = priority;
  }

  if( stackSize == 0 )
  {
    stack = ATP_THREAD_STACK_SIZE;
  }
  else
  {
    stack = stackSize;
  }

  m2mb_os_taskSetAttrItem( &taskAttr,
                           CMDS_ARGS(
                             M2MB_OS_TASK_SEL_CMD_CREATE_ATTR, NULL,
                             M2MB_OS_TASK_SEL_CMD_STACK_SIZE,  stack,
                             M2MB_OS_TASK_SEL_CMD_NAME, module->module_name,
                             M2MB_OS_TASK_SEL_CMD_PRIORITY, prio,
                             M2MB_OS_TASK_SEL_CMD_PREEMPTIONTH, 231,
                             M2MB_OS_TASK_SEL_CMD_AUTOSTART, M2MB_OS_TASK_AUTOSTART,
                             M2MB_OS_TASK_SEL_CMD_USRNAME, module->module_name
                           )
                         );
  newHandler->module = module;
  osRes = m2mb_os_taskCreate( &( newHandler->azx_easy_at_taskHandle ), &taskAttr,
                              _azx_easy_at_task_entry, newHandler );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_EASY_AT_TRACE_DEBUG( "ATPTASK : Thread creation failed; error %d\r\n", osRes );
    m2mb_os_free( newHandler );
    return M2MB_RESULT_FAIL;
  }

  *handle = ( AZX_EASY_AT_TASK_HANDLE )newHandler;
  return M2MB_RESULT_SUCCESS;
}


static M2MB_RESULT_E _azx_easy_at_cmdRegister( AZX_EASY_AT_MODULE_T *module,
                                               M2MB_ATP_HANDLE atpHandle,
                                               CHAR *atpCmdString, UINT16 atpFlags, azx_easy_at_taskCallback azx_easy_at_taskCallback,
                                               azx_easy_at_taskDelegation azx_easy_at_taskDelegation,
                                               AZX_EASY_AT_TASK_HANDLE azx_easy_at_taskHandle )
{
  AZX_EASY_AT_TASK_HANDLE_T *tmp_azx_easy_at_taskHandle = ( AZX_EASY_AT_TASK_HANDLE_T * )
                                                          azx_easy_at_taskHandle;
  AZX_EASY_AT_TASK_USERDATA_T *azx_easy_at_taskUserdata = ( AZX_EASY_AT_TASK_USERDATA_T * )
                                                          m2mb_os_malloc( sizeof( AZX_EASY_AT_TASK_USERDATA_T ) );

  if( azx_easy_at_taskUserdata == NULL )
  {
    return M2MB_RESULT_FAIL;
  }

  azx_easy_at_taskUserdata->azx_easy_at_taskHandle =
    tmp_azx_easy_at_taskHandle->azx_easy_at_taskHandle;
  azx_easy_at_taskUserdata->azx_easy_at_taskQueue = tmp_azx_easy_at_taskHandle->azx_easy_at_taskQueue;
  azx_easy_at_taskUserdata->taskCallback = azx_easy_at_taskCallback;
  azx_easy_at_taskUserdata->taskDelegation = azx_easy_at_taskDelegation;
  azx_easy_at_taskUserdata->module = module;
  return m2mb_atp_register( atpHandle, atpCmdString, atpFlags, _azx_easy_at_task_group_callback,
                            ( void * )azx_easy_at_taskUserdata );
}


static M2MB_RESULT_E _azx_easy_at_registerAllCommands( AZX_EASY_AT_MODULE_T *module,
                                                       M2MB_ATP_HANDLE atpH, AZX_EASY_AT_ATCOMMAND_T *list, INT32 list_size,
                                                       AZX_EASY_AT_TASK_HANDLE appAtptaskH )
{
  int i;
  int numberOfCommands = list_size;

  for( i = 0; i < numberOfCommands; i++ )
  {
    if( list[i].isHidden )
    {
    }
    else
    {
      AZX_EASY_AT_TRACE_INFO( "register AT%s \r\n", list[i].cmd );
    }

    AZX_EASY_AT_ASSERT_REGISTER( _azx_easy_at_cmdRegister( module, atpH,
                                                           ( CHAR * )list[i].cmd,
                                                           list[i].atpFlags,
                                                           list[i].Callback,
                                                           list[i].Delegation,
                                                           appAtptaskH ) );
  }

  return M2MB_RESULT_SUCCESS;
}


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
M2MB_RESULT_E azx_easy_at_getAte( AZX_EASY_AT_HANDLES_T *h, INT32 *ate )
{
  M2MB_RESULT_E res;
  M2MB_ATP_AT_COMMAND_VAL_U ate_val;

  if( ( h == NULL ) || ( ate == NULL ) )
  {
    return M2MB_RESULT_INVALID_ARG;
  }

  res = m2mb_atp_at_command_conf( ( h )->handle, ( h )->atpI, ( CHAR * ) "E?", 1, &ate_val );

  if( M2MB_RESULT_SUCCESS != res )
  {
    *ate = 0;
    return res;
  }
  else
  {
    *ate = ( INT32 ) ate_val.get_integer;
    return M2MB_RESULT_SUCCESS;
  }
}


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
M2MB_RESULT_E azx_easy_at_getCmee( AZX_EASY_AT_HANDLES_T *h, INT32 *cmee )
{
  M2MB_RESULT_E res;
  M2MB_ATP_AT_COMMAND_VAL_U cme_val;

  if( ( h == NULL ) || ( cmee == NULL ) )
  {
    return M2MB_RESULT_INVALID_ARG;
  }

  res = m2mb_atp_at_command_conf( ( h )->handle, ( h )->atpI, ( CHAR * ) "+CMEE?", 1, &cme_val );

  if( M2MB_RESULT_SUCCESS != res )
  {
    *cmee = 0;
    return res;
  }
  else
  {
    *cmee = ( INT32 ) cme_val.get_integer;
    return M2MB_RESULT_SUCCESS;
  }
}


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
                                        INT32 list_size )
{
  AZX_EASY_AT_MODULE_T *module = NULL;
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  AZX_EASY_AT_TASK_HANDLE appAtptaskHandle;
  M2MB_ATP_HANDLE atpHandle;
  module = ( AZX_EASY_AT_MODULE_T * ) m2mb_os_malloc( sizeof( AZX_EASY_AT_MODULE_T ) );

  if( !module )
  {
    return NULL;
  }

  strncpy( module->module_name, module_name, sizeof( module->module_name ) );
  AZX_EASY_AT_TRACE_DETAIL( "azx_easy_at_init start\r\n" );
  retVal = m2mb_atp_init( &atpHandle, ( m2mb_atp_ind_callback )NULL, NULL );

  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_EASY_AT_TRACE_ERROR( "m2m_atp_init failed! Aborting.\r\n" );
    m2mb_os_free( module );
    return NULL;
  }
  else
    if( NULL == atpHandle )
    {
      AZX_EASY_AT_TRACE_ERROR( "atpHandle is NULL! Aborting.\r\n" );
      m2mb_os_free( module );
      return NULL;
    }
    else
    {
      AZX_EASY_AT_TRACE_DEBUG( "m2mb_atp_init succeeded\r\n" );
    }

  retVal = _azx_easy_at_task_init( module, &appAtptaskHandle, 0, 0 );

  if( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_EASY_AT_TRACE_DETAIL( "_azx_easy_at_task_init succeeded\r\n" );
  }
  else
  {
    AZX_EASY_AT_TRACE_ERROR( "aptask_init failed! Aborting.\r\n" );
    m2mb_os_free( module );
    return NULL;
  }

  retVal = _azx_easy_at_registerAllCommands( module, atpHandle, list, list_size, appAtptaskHandle );

  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_EASY_AT_TRACE_ERROR( "_azx_easy_at_registerAllCommands failed! Aborting.\r\n" );
    m2mb_os_free( module );
    return NULL;
  }

  module->last_AT_Instance = AZX_EASY_AT_INVALID_AT_INSTANCE;

  module->last_ATP_Handle = atpHandle;

  return module;
}



/* =============================

   Utility functions

   =============================*/

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
CHAR *azx_easy_at_TrimAndRemoveQuotes( CHAR *pStr )
{
  int nLen = strlen( pStr );
  int i, j;
  CHAR *p = pStr;
  int nFirst = 0;

  for( j = 0; j < nLen; j++ )
  {
    nFirst = j;

    if( *( p + nFirst ) == '"' )
    {
      for( i = nFirst + 1; i < nLen; i++ )
      {
        if( pStr[i] != '"' )
        {
          *( p + i - 1 ) = pStr[i];
        }
        else
        {
          *( p + i - 1 ) = '\0';
        }
      }

      *( p + i - 1 ) = '\0';
    }
  }

  return p;
}


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
INT32 azx_easy_at_strToL( CHAR *str, INT32 *output )
{
  CHAR *endptr;
  INT32 tmp;

  if( ( str == NULL ) || ( output == NULL ) )
  {
    return -3;
  }

  errno = 0;

  if( str[strspn( str, "0123456789-+" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -4;
  }

  tmp = strtol( str, &endptr, 10 );

  if( ( errno == ERANGE && ( ( tmp == LONG_MAX ) || ( tmp == LONG_MIN ) ) ) || ( errno != 0 &&
                                                                                 tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( ( *endptr != '\0') || (  endptr == ( CHAR * ) str ))
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}


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
INT32 azx_easy_at_strToUL( CHAR *str, UINT32 *output )
{
  CHAR *endptr;
  UINT32 tmp;

  if( ( str == NULL ) || ( output == NULL ) )
  {
    return -3;
  }

  errno = 0;

  if( str[strspn( str, "0123456789" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -4;
  }

  tmp = strtoul( str, &endptr, 10 );

  if( ( errno == ERANGE && ( tmp == ULONG_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( ( *endptr != '\0') || (  endptr == ( CHAR * ) str ))
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}


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
INT32 azx_easy_at_strToULL( CHAR *str, UINT64 *output )
{
  CHAR *endptr;
  UINT64 tmp;

  if( ( str == NULL ) || ( output == NULL ) )
  {
    return -3;
  }

  errno = 0;

  if( str[strspn( str, "0123456789" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -4;
  }

  tmp = strtoull( str, &endptr, 10 );

  if( ( errno == ERANGE && ( tmp == ULONG_LONG_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( ( *endptr != '\0') || ( endptr == ( CHAR * ) str ))
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}


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
INT32 azx_easy_at_strToULHex( CHAR *str, UINT32 *output )
{
  CHAR *endptr;
  UINT32 tmp;

  if( ( str == NULL ) || ( output == NULL ) )
  {
    return -3;
  }

  errno = 0;

  if( str[strspn( str, "0123456789abcdefABCDEF" )] !=
      0 ) //check if string is composed of HEX digits only
  {
    /*not valid hex input*/
    return -4;
  }

  tmp = strtoul( str, &endptr, 16 );

  if( ( errno == ERANGE && ( tmp == ULONG_MAX ) ) || ( errno != 0 && tmp == 0 ) )
  {
    return -1;
  }

  if( ( *endptr != '\0') || (  endptr == ( CHAR * ) str ))
  {
    /*no digits found*/
    return -2;
  }

  *output = tmp;
  return 0;
}


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
INT8 azx_easy_at_strToUS( CHAR *str, UINT8 *output )
{
  CHAR *endptr;
  UINT32 tmp;

  if( ( str == NULL ) || ( output == NULL ) )
  {
    return -3;
  }

  errno = 0;

  if( str[strspn( str, "0123456789" )] != 0 ) //check if string is composed of base10 digits only
  {
    /*not valid number input*/
    return -4;
  }

  tmp = strtoul( str, &endptr, 10 );

  if( ( tmp >= UCHAR_MAX ) || ( errno != 0 && tmp == 0 ) )
  {
    /*Out of range parameter*/
    return -1;
  }

  if( ( *endptr != '\0') || (  endptr == ( CHAR * ) str ))
  {
    /*no digits found*/
    return -2;
  }

  *output = (UINT8) tmp;
  return 0;
}


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
UINT8 azx_easy_at_ASCtoDEC( CHAR ch )
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
