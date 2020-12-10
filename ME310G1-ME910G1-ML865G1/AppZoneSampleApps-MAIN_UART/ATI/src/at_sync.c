/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    m2mb_async.c

  @brief
    The file contains the syncronous implementation of AT example

  @details

  @description
    Synchronous implementation of AT instance communication (using a polling
    check)
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
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

static void * mydata;

static M2MB_ATI_HANDLE ati_handles[2];


/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
M2MB_RESULT_E at_cmd_sync_init(INT16 instance)
{
  M2MB_RESULT_E res;
  AZX_LOG_DEBUG("m2mb_ati_init() on instance %d\r\n", instance);
  res = m2mb_ati_init(&ati_handles[instance], instance, NULL, mydata);
  return res;
}

M2MB_RESULT_E at_cmd_sync_deinit(INT16 instance)
{

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


M2MB_RESULT_E send_sync_at_command(INT16 instance, const CHAR *atCmd, CHAR *atRsp, UINT32 atRspMaxLen)
{
  INT32 cmd_len = 0;
  SSIZE_T rsp_len;
  M2MB_RESULT_E retVal;

  AZX_LOG_DEBUG("Sending AT Command: %.*s\r\n",strlen(atCmd) -1, atCmd);

  cmd_len = strlen(atCmd);

  retVal = m2mb_ati_send_cmd(ati_handles[instance], (void*) atCmd, cmd_len);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("m2mb_ati_send_cmd() returned failure value\r\n");
    return retVal;
  }

  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(2000) );


  memset(atRsp,0x00,atRspMaxLen);
  rsp_len = m2mb_ati_rcv_resp(ati_handles[instance], atRsp, atRspMaxLen);
  if ( rsp_len <= 0 )
  {
    AZX_LOG_ERROR( "m2mb_ati_rcv_resp() returned failure value\r\n");
    return M2MB_RESULT_FAIL;
  }
  return M2MB_RESULT_SUCCESS;

}


