/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    m2mb_async.c

  @brief
    The file contains the asyncronous implementation of AT example

  @details

  @description
    Asynchronous implementation of AT instance communication (using a 
    callback function)
  @version 
    1.0.0
  @note
    

  @author
	Fabio Pintus
  @date
    18/07/2019
*/
/* Include files ================================================================================*/

#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_ati.h"

#include "azx_log.h"


/* Local defines ================================================================================*/
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define AT_RSP_TIMEOUT 120000

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

static unsigned char g_at_rsp_buf[4096];

static M2MB_ATI_HANDLE ati_handles[2];

static M2MB_OS_SEM_HANDLE at_rsp_sem = NULL;

static int ati_state = M2MB_STATE_IDLE_EVT;

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
static void at_cmd_async_callback ( M2MB_ATI_HANDLE h, M2MB_ATI_EVENTS_E ati_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
  (void)h;
  (void)userdata;
  
  INT32 resp_len;
  INT16 resp_len_short;
  AZX_LOG_TRACE("ati callback! Event: %d; resp_size: %u\r\n", ati_event, resp_size);

  if(ati_event == M2MB_RX_DATA_EVT )
  {
    if(ati_state == M2MB_STATE_IDLE_EVT)
    {
      
      AZX_LOG_TRACE("This is an UNSOLICITED\r\n");
      //read and discard data
      //m2mb_ati_rcv_resp(h, tmp_buf, 100);
      //AZX_LOG_TRACE("UNSOLICITED: %s\r\n", tmp_buf);
    }
    else /*Normal data reception, read and append into global buffer*/
    {
      
      if (resp_size == 2)
      {
        resp_len_short = *(INT16*)resp_struct;
        resp_len = resp_len_short;
      }
      else
      {
        resp_len = *(INT32*)resp_struct;
      }
      AZX_LOG_DEBUG("Callback - available bytes: %d\r\n", resp_len);
    }
  }
  else
  {
    ati_state = ati_event;
  }

  if(ati_event == M2MB_STATE_IDLE_EVT) /*AT parser changed to IDLE, meaning the command execution completed.*/
  {
    AZX_LOG_TRACE("UNLOCKING AT semaphore\r\n");
    m2mb_os_sem_put(at_rsp_sem);
  }
}


/* Global functions =============================================================================*/

M2MB_RESULT_E at_cmd_async_init(INT16 instance)
{
  M2MB_OS_SEM_ATTR_HANDLE semAttrHandle;

  if (NULL == at_rsp_sem)
  {
    m2mb_os_sem_setAttrItem( &semAttrHandle, CMDS_ARGS( M2MB_OS_SEM_SEL_CMD_CREATE_ATTR,  NULL,M2MB_OS_SEM_SEL_CMD_COUNT, 1 /*CS*/, M2MB_OS_SEM_SEL_CMD_TYPE, M2MB_OS_SEM_BINARY,M2MB_OS_SEM_SEL_CMD_NAME, "ATRSPSem"));
    m2mb_os_sem_init( &at_rsp_sem, &semAttrHandle );
  }

  AZX_LOG_DEBUG("m2mb_ati_init() on instance %d\r\n", instance);
  if ( m2mb_ati_init(&ati_handles[instance], instance, at_cmd_async_callback, at_rsp_sem) == M2MB_RESULT_SUCCESS )
  {
    return M2MB_RESULT_SUCCESS;
  }
  else
  {
    AZX_LOG_ERROR("m2mb_ati_init() returned failure value\r\n" );
    return M2MB_RESULT_FAIL;
  }
}

M2MB_RESULT_E at_cmd_async_deinit(INT16 instance)
{
  if (NULL != at_rsp_sem)
  {
    m2mb_os_sem_deinit( at_rsp_sem);
    at_rsp_sem=NULL;
  }

  AZX_LOG_DEBUG("m2mb_ati_deinit() on instance %d\r\n", instance);
  if ( m2mb_ati_deinit(ati_handles[instance]) == M2MB_RESULT_SUCCESS )
  {
    return M2MB_RESULT_SUCCESS;
  }
  else
  {
    return M2MB_RESULT_FAIL;
  }
}


M2MB_RESULT_E send_async_at_command(INT16 instance, const CHAR *atCmd, CHAR *atRsp, UINT32 atRspMaxLen)
{
  INT32 cmd_len = 0;
  SSIZE_T rsp_len;
  M2MB_RESULT_E retVal;
  AZX_LOG_DEBUG("Sending AT Command: %.*s\r\n",strlen(atCmd) -1, atCmd);

  m2mb_os_sem_get(at_rsp_sem, M2MB_OS_WAIT_FOREVER );  //get critical section

  memset(g_at_rsp_buf,0, sizeof(g_at_rsp_buf));


  cmd_len = strlen(atCmd);

  retVal = m2mb_ati_send_cmd(ati_handles[instance], (void*) atCmd, cmd_len);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("m2mb_ati_send_cmd() returned failure value\r\n");
    return retVal;
  }

  AZX_LOG_DEBUG("waiting command response...\r\n");
  //Wait for AT command response...
  if( M2MB_OS_SUCCESS != m2mb_os_sem_get(at_rsp_sem, M2MB_OS_MS2TICKS(AT_RSP_TIMEOUT) ) )/* waiting for "IPC" semaphore */
  {
    //failure,
    AZX_LOG_ERROR("semaphore timeout!\r\n");
    return M2MB_RESULT_FAIL;
  }
  else
  {
    memset(atRsp,0x00,atRspMaxLen);

    AZX_LOG_DEBUG("Receive response...\r\n");
    rsp_len = m2mb_ati_rcv_resp(ati_handles[instance], atRsp, atRspMaxLen);
    if(rsp_len == -1)
    {
      m2mb_os_sem_put(at_rsp_sem);  /*Release CS*/
      return M2MB_RESULT_FAIL;
    }

    m2mb_os_sem_put(at_rsp_sem);  /*Release CS*/
    return M2MB_RESULT_SUCCESS;
  }
}

