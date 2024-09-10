/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    at_common.c

  @brief
    The file contains the common functions for custom commands

  @author


  @date
    13/03/2020
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os.h"
#include "m2mb_os_sem.h"

#include "m2mb_pdp.h"
#include "m2mb_os_api.h"
#include "m2mb_atp.h"

#include "azx_log.h"


//#include "azx_easy_at.h"

#include "at_common.h"

#include "at_hash_M2MWRITE.h"
#include "at_hash_M2MADMIN.h"


static EASY_AT_MODULE_T *g_at_module = NULL;
HANDLE   appAtptaskHandle;

#ifndef ATP_THREAD_PRIORITY

#define ATP_THREAD_PRIORITY 231

#endif

#ifndef ATP_THREAD_STACK_SIZE

#define ATP_THREAD_STACK_SIZE (16*1024)

#endif


/* Local typedefs ===============================================================================*/





/* Local statics ================================================================================*/


static const CHAR *easy_at_task_msg_event_type[] =
{
  "CMD",    /* 0  */
  "DELEG",   /* 1  */
};

/* Local function prototypes ====================================================================*/


/* Static functions =============================================================================*/



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
static void _easy_at_task_entry( void *pArg );


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
static M2MB_RESULT_E _easy_at_task_init( EASY_AT_MODULE_T *module,
                                             EASY_AT_TASK_HANDLE *handle, UINT16 priority, UINT32 stackSize );



void set_at_module( EASY_AT_MODULE_T *module )
{
  g_at_module = module;
}

EASY_AT_MODULE_T *get_at_module( void )
{
  return g_at_module;
}


void _easy_at_task_group_callback( M2MB_ATP_HANDLE atpHandle,
                                              M2MB_ATP_CMD_IND_E atpEvent, UINT16 resp_size, void *resp_struct, void *easy_at_taskUserdata )
{
  M2MB_OS_RESULT_E osRes = M2MB_OS_QUEUE_FULL;
  EASY_AT_TASK_USERDATA_T *tmp_easy_at_taskUserdata;
  M2MB_ATP_REQ_T Message;

  if( easy_at_taskUserdata != NULL )
  {
    tmp_easy_at_taskUserdata = ( EASY_AT_TASK_USERDATA_T * )easy_at_taskUserdata;
    Message.atpHandle = atpHandle;
    Message.atpEvent = atpEvent;
    Message.resp_size = resp_size;
    /*copy the message resp struct*/
    Message.resp_struct = m2mb_os_malloc( resp_size );

    if( Message.resp_struct != NULL )
    {
      memcpy( Message.resp_struct, resp_struct, resp_size );
      Message.atptaskUserdata = easy_at_taskUserdata;
      osRes = m2mb_os_q_tx( tmp_easy_at_taskUserdata->easy_at_taskQueue, ( void * )&Message,
                            M2MB_OS_NO_WAIT, 0 );

      if( osRes != M2MB_OS_SUCCESS )
      {
        AZX_LOG_ERROR( "Send message to ATP queue failed; error %d", osRes );
      }
    }
  }
}


static void _easy_at_task_entry( void *pArg )
{
  EASY_AT_TASK_HANDLE_T *atpTaskHandle = ( EASY_AT_TASK_HANDLE_T * )pArg;
  EASY_AT_TASK_USERDATA_T *easy_at_taskUserdata;
  M2MB_ATP_REQ_T Message;

  while( 1 )
  {
    m2mb_os_q_rx( atpTaskHandle->easy_at_taskQueue, &Message, M2MB_OS_WAIT_FOREVER );
    easy_at_taskUserdata = ( EASY_AT_TASK_USERDATA_T * )Message.atptaskUserdata;
    //atpTaskHandle->module
    AZX_LOG_DEBUG( "task entry, message event type: %s\r\n",
                             easy_at_task_msg_event_type[Message.atpEvent] );
    //M2M_LOG_DEBUG_MORE("event type: %s\r\n", atp_task_msg_event_type[Message.atpEvent] );
    atpTaskHandle->module->last_ATP_Handle = Message.atpHandle;

    switch( Message.atpEvent )
    {
      case M2MB_ATP_CMD_CALLBACK_IND:
      {
        M2MB_ATP_CALLBACK_IND_T *tmp_resp = ( M2MB_ATP_CALLBACK_IND_T * )Message.resp_struct;

        if( ( tmp_resp != NULL ) && ( easy_at_taskUserdata != NULL ) )
        {
          atpTaskHandle->module->last_AT_Instance = tmp_resp->instanceNumber;
          easy_at_taskUserdata->taskCallback( Message.atpHandle, tmp_resp->instanceNumber );
        }

        break;
      }

      case M2MB_ATP_CMD_DELEGATION_IND:
      {
        //FIXME check if atpTaskDelegation is ! NULL, use a generic "doing"
        M2MB_ATP_DELEGATION_IND_T *tmp_resp = ( M2MB_ATP_DELEGATION_IND_T * )Message.resp_struct;

        if( ( tmp_resp != NULL ) && ( easy_at_taskUserdata != NULL ) )
        {
          atpTaskHandle->module->last_AT_Instance = tmp_resp->instanceNumber;

          if( easy_at_taskUserdata->taskDelegation )
          {
            easy_at_taskUserdata->taskDelegation( Message.atpHandle, tmp_resp->instanceNumber,
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

static M2MB_RESULT_E _easy_at_task_init( EASY_AT_MODULE_T *module,
                                             EASY_AT_TASK_HANDLE *handle, UINT16 priority, UINT32 stackSize )
{
  M2MB_OS_RESULT_E osRes;
  M2MB_OS_TASK_ATTR_HANDLE taskAttr;
  M2MB_OS_Q_ATTR_HANDLE qAttrHandle;
  EASY_AT_TASK_HANDLE_T *newHandler;
  UINT32 msgSize = sizeof( M2MB_ATP_REQ_T );
  static UINT8 QueueArea[100 * 4 * 4];
  UINT16 prio;
  UINT32 stack;
  AZX_LOG_TRACE( "ATPTASK : _easy_at_task_init %d\r\n", msgSize );
  msgSize = ( msgSize / 4 );
  AZX_LOG_TRACE( "msgSize %d\r\n", msgSize );
  osRes = m2mb_os_q_setAttrItem( &qAttrHandle, 1, M2MB_OS_Q_SEL_CMD_CREATE_ATTR,  NULL );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_LOG_DEBUG( "ATPTASK : Create queue attributes failed; error %d\r\n", osRes );
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
    AZX_LOG_DEBUG( "ATPTASK : Set queue attributes failed; error %d\r\n", osRes );
    return M2MB_RESULT_FAIL;
  }

  newHandler = ( EASY_AT_TASK_HANDLE_T * )m2mb_os_malloc( sizeof( EASY_AT_TASK_HANDLE_T ) );

  if( newHandler == NULL )
  {
    return M2MB_RESULT_FAIL;
  }

  osRes = m2mb_os_q_init( &( newHandler->easy_at_taskQueue ), &qAttrHandle );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_LOG_DEBUG( "ATPTASK : Queue init failed; error %d", osRes );
    m2mb_os_free( newHandler );
    return M2MB_RESULT_FAIL;
  }

  AZX_LOG_TRACE( "ATPTASK : Thread creation\r\n" );

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
  osRes = m2mb_os_taskCreate( &( newHandler->easy_at_taskHandle ), &taskAttr,
                              _easy_at_task_entry, newHandler );

  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_LOG_DEBUG( "ATPTASK : Thread creation failed; error %d\r\n", osRes );
    m2mb_os_free( newHandler );
    return M2MB_RESULT_FAIL;
  }

  *handle = (EASY_AT_TASK_HANDLE )newHandler;
  return M2MB_RESULT_SUCCESS;
}




EASY_AT_MODULE_T * easy_at_init( CHAR *module_name,  EASY_AT_ATCOMMAND_T *list,
                                        INT32 list_size )
{
  EASY_AT_MODULE_T *module = NULL;
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

  M2MB_ATP_HANDLE atpHandle;
  module = ( EASY_AT_MODULE_T * ) m2mb_os_malloc( sizeof( EASY_AT_MODULE_T ) );

  if( !module )
  {
    return NULL;
  }

  strncpy( module->module_name, module_name, sizeof( module->module_name ) );
  AZX_LOG_INFO( "Easy_at_init start\r\n" );
  retVal = m2mb_atp_init( &atpHandle, ( m2mb_atp_ind_callback )NULL, NULL );

  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2m_atp_init failed! Aborting.\r\n" );
    m2mb_os_free( module );
    return NULL;
  }
  else
  {
    if( NULL == atpHandle )
    {
      AZX_LOG_ERROR( "atpHandle is NULL! Aborting.\r\n" );
      m2mb_os_free( module );
      return NULL;
    }
    else
    {
      AZX_LOG_DEBUG( "m2mb_atp_init succeeded\r\n" );
    }
  }
  retVal = _easy_at_task_init( module, &appAtptaskHandle, 0, 0 );

  if( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_TRACE( "_easy_at_task_init succeeded\r\n" );
  }
  else
  {
    AZX_LOG_ERROR( "aptask_init failed! Aborting.\r\n" );
    m2mb_os_free( module );
    return NULL;
  }

  int i;
  int numberOfCommands = list_size;

  for( i = 0; i < numberOfCommands; i++ )
  {
    if( list[i].isHidden )
    {
    }
    else
    {
      AZX_LOG_INFO( "Register AT%s \r\n", list[i].cmd );
    }

    EASY_AT_TASK_HANDLE_T *tmp_easy_at_taskHandle = ( EASY_AT_TASK_HANDLE_T * )appAtptaskHandle;
    EASY_AT_TASK_USERDATA_T *easy_at_taskUserdata = ( EASY_AT_TASK_USERDATA_T * )m2mb_os_malloc( sizeof( EASY_AT_TASK_USERDATA_T ) );

    if( easy_at_taskUserdata == NULL )
    {
      return NULL;
    }

    easy_at_taskUserdata->easy_at_taskHandle =
    tmp_easy_at_taskHandle->easy_at_taskHandle;
    easy_at_taskUserdata->easy_at_taskQueue = tmp_easy_at_taskHandle->easy_at_taskQueue;
    easy_at_taskUserdata->taskCallback = list[i].Callback;
    easy_at_taskUserdata->taskDelegation = list[i].Delegation;
    easy_at_taskUserdata->module = module;

    retVal = m2mb_atp_register( atpHandle, ( CHAR * )list[i].cmd, list[i].atpFlags, _easy_at_task_group_callback,
                               ( void * )easy_at_taskUserdata );

    if(retVal == M2MB_RESULT_SUCCESS)
    {
      AZX_LOG_TRACE("Command Registered!\r\n");
    }
    else
    {
      AZX_LOG_ERROR("Cannot register! error: %d\r\n", retVal);
      m2mb_os_free( module );
      return NULL;
    }

   }

  module->last_AT_Instance = EASY_AT_INVALID_AT_INSTANCE;

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
CHAR *TrimAndRemoveQuotes( CHAR *pStr )
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
INT32 strToUL( CHAR *str, UINT32 *output )
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


INT32 my_cmds_at_init( void )
{
  EASY_AT_MODULE_T *module;
  EASY_AT_ATCOMMAND_T my_commands_List[] =
  {
    { "#M2MWRITE",   M2MB_ATP_NORML | M2MB_ATP_NOPIN | M2MB_ATP_NOSIM, M2MWRITE_AT_Callback, NULL, 0},
    { "#M2MADMIN",   M2MB_ATP_NORML | M2MB_ATP_NOPIN | M2MB_ATP_NOSIM, M2MADMIN_AT_Callback, NULL, 0},

  };
  module = easy_at_init( ( CHAR * ) CMD_MODULE_NAME, my_commands_List,
          sizeof(my_commands_List)/sizeof(my_commands_List[0]) );

  if( ! module )
  {
    return -1;
  }

  set_at_module( module );
  return 0;
}
