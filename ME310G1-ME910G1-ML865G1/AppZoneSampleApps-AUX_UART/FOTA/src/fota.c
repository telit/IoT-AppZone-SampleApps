/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    fota.c

  @brief
    The file contains the FOTA implementation

  @details
  
  @version 
    1.0.3
  @note
  

  @author


  @date
    25/05/2018
*/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "m2mb_types.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_fs_posix.h"
#include "m2mb_fs_errno.h"

#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"
#include "m2mb_fota.h"
#include "m2mb_power.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "fota.h"

#include "app_cfg.h" /*FOR LOCALPATH define*/

/* Macro =============================================================================*/
#define APN  "web.omnitel.it"
#define PDP_CTX (UINT8)3

#define FTP_ADDR "x.x.x.x"
#define FTP_PORT AZX_FTP_DEFAULT_PORTNUM

#define FTP_USER "user"
#define FTP_PASS "password"

#define FILE_URI "file/remote/path/delta.bin"
#define SESSION_FILE  LOCALPATH "/ota_done"

/* Global variables =============================================================================*/
M2MB_FOTA_HANDLE fota_handle;

M2MB_PDP_HANDLE pdpHandle = NULL;
M2MB_FOTA_FTP_CFG_T fotaDwFtp;

extern INT32 fota_task_id;
extern INT32 main_task_id;

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;


/* Global functions =============================================================================*/
static void FOTADownloadCallBack( M2MB_FOTA_HANDLE h, M2MB_FOTA_IND_E fota_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
  (void) resp_size;
  (void) userdata;
  
  if( ( fota_event == M2MB_FOTA_EVENT_HTTP_DELTA_TRANSFER_END ) || ( fota_event == M2MB_FOTA_EVENT_FTP_DELTA_TRANSFER_END ) )
  {
    M2MB_FOTA_CB_RESP_DELTA_TRANSFER_END_T *resp = ( M2MB_FOTA_CB_RESP_DELTA_TRANSFER_END_T *)resp_struct;

    if(resp->result == M2MB_FOTA_DOWNLOAD_SUCCESS)
    {
      if( fota_handle != h ) // For debug
      {
        AZX_LOG_ERROR("FAIL: FOTA download FAIL - handles don't match\r\n");
        azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
      }

      AZX_LOG_DEBUG("FOTA download Success - performing packet validation...\r\n");
      azx_tasks_sendMessageToTask(fota_task_id, CHECKDELTA,0,0);
    }
    else
    {
      AZX_LOG_ERROR("FOTA download FAIL - giving up\r\n");
      AZX_LOG_DEBUG("resp->result: %d\r\n", resp->result);
      azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
    }
  }
}


void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
  (void) h;
  (void) resp_size;
  (void) myUserdata;
  
  M2MB_NET_REG_STATUS_T *stat_info;

  switch (net_event)
  {
#if 0
    case M2MB_NET_REG_STATUS_IND:
      AZX_LOG_DEBUG("Network change event!\r\n");
      m2mb_net_get_reg_status_info(h);
      break;
#endif

    case M2MB_NET_GET_REG_STATUS_INFO_RESP:
      stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
      //AZX_LOG_DEBUG("GET NET status resp is STAT: %d, RAT: %d, SRVDOMAIN: %d, AREA CODE: 0x%X, CELL ID: 0x%X\r\n", stat_info->stat, stat_info->rat, stat_info->srvDomain, stat_info->areaCode, stat_info->cellID);
      if  (stat_info->stat == 1 || stat_info->stat == 5)
      {
        AZX_LOG_DEBUG("Module is registered to network\r\n");
        m2mb_os_ev_set(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
      }
      break;

    default:
      AZX_LOG_DEBUG("unexpected net_event: %d\r\n", net_event);
      break;
  }
}

void PdpCallback(M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata)
{
  (void) userdata;
  
  struct M2MB_SOCKET_BSD_SOCKADDR_IN CBtmpAddress;
  CHAR CBtmpIPaddr[32];

  switch (pdp_event)
  {
    case M2MB_PDP_UP:
      AZX_LOG_DEBUG ("Context activated!\r\n");
      m2mb_pdp_get_my_ip(h, cid, M2MB_PDP_IPV4, &CBtmpAddress.sin_addr.s_addr);
      m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET, &CBtmpAddress.sin_addr.s_addr, ( CHAR * )&( CBtmpIPaddr ), sizeof( CBtmpIPaddr ) );
      AZX_LOG_DEBUG( "IP address: %s\r\n\r\n", CBtmpIPaddr);
      //azx_sleep_ms( 1000 );
      azx_tasks_sendMessageToTask(fota_task_id, DOWNLOAD,0,0);
      break;

    case M2MB_PDP_DOWN:
      AZX_LOG_TRACE ("Context deactivate\r\n");
      break;
    default:
      AZX_LOG_DEBUG("unexpected pdp_event: %d\r\n", pdp_event);
      break;
  }
}


INT32 fotaTask(INT32 type, INT32 param1, INT32 param2)
{
  (void) param1;
  (void) param2;
  
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  void *userdata = NULL;
  M2MB_FOTA_STATE_E FotaState;
  M2MB_FOTA_RESULT_CODE_E res_code;
  INT32 fd = -1;

  AZX_LOG_TRACE("Fota_Task Started\r\n");

  switch (type)
  {
    case INITFOTA:

      AZX_LOG_DEBUG("Init FOTA...\r\n");
      if(-1 == (fd = m2mb_fs_open(SESSION_FILE, M2MB_O_RDONLY)))
      {
        INT32 last_errno = m2mb_fs_get_errno_value();
        if(M2MB_FS_ENOENT == last_errno)
        {
          AZX_LOG_INFO("\r\nSession file not present, procede with FOTA...\r\n");
        }
        else
        {
          AZX_LOG_ERROR("Unexpected filesystem error when checking for session file: %d\r\n", last_errno);
          return -1;
        }
      }
      else
      {
        AZX_LOG_INFO("\r\nSession file is already present, stop.\r\n");
        m2mb_fs_close(fd);
        m2mb_fs_unlink(SESSION_FILE);
        return 0;
      }


      retVal = m2mb_fota_init( &fota_handle, FOTADownloadCallBack, userdata );
      if (retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_fota_init FAIL\r\n");
        azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
      }
      else
      {
        AZX_LOG_TRACE("m2mb_fota_init PASS\r\n");
#ifndef LE910CXL
        azx_tasks_sendMessageToTask(fota_task_id,RESET,0,0);
#else
        azx_tasks_sendMessageToTask(main_task_id, INIT,0,0);
#endif
      }
      break;

    case RESET:
      retVal = m2mb_fota_reset( fota_handle );
      if( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_fota_reset FAIL \r\n");
        azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
      }
      else
      {
        AZX_LOG_DEBUG("m2mb_fota_reset PASS \r\n");
        azx_tasks_sendMessageToTask( fota_task_id, GETSTATE, 0, 0);
      }
      break;


    case GETSTATE:
      retVal = m2mb_fota_state_get( fota_handle, &FotaState );
      if( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_fota_state_get FAIL \r\n");
        azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
      }
      else
      {
        AZX_LOG_TRACE("Fota state: 0x%04X.\r\n", FotaState);

        if( FotaState != M2MB_FOTA_STATE_IDLE )
        {
          AZX_LOG_DEBUG("m2mb_fota_state_get  !M2MB_FOTA_STATE_IDLE\r\n");
          azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
        }
        else
        {
          AZX_LOG_DEBUG("m2mb_fota_state_get  M2MB_FOTA_STATE_IDLE\r\n");
          azx_tasks_sendMessageToTask(main_task_id, INIT,0,0);
        }
      }
      break;

    case DOWNLOAD:
      //FTP
      fotaDwFtp.socketCfg.cid = PDP_CTX;
      fotaDwFtp.socketCfg.cid_active = false;
      sprintf( fotaDwFtp.sessionCfg.URI, "%s/%s", FTP_ADDR, FILE_URI );
      sprintf( fotaDwFtp.sessionCfg.user, "%s", FTP_USER );
      sprintf( fotaDwFtp.sessionCfg.psw, "%s", FTP_PASS );
      fotaDwFtp.sessionCfg.passiveMode = true;
      fotaDwFtp.sessionCfg.timeout = 5000;  //hundreds of ms [100-5000]
      fotaDwFtp.sessionCfg.IPPignoring = 0; //0 ip ignoring not enabled, 1 otherwise
      fotaDwFtp.sessionCfg.FTPSEn = 0;      //0 disable ftps security
      fotaDwFtp.sessionCfg.protocol = M2MB_FOTA_BASIC_FTP_PROTOCOL;
      fotaDwFtp.socketCfg.connID = 1;       //from 1 to 6
      fotaDwFtp.socketCfg.pktSize = 300; //bytes [1-1500], 0 authomatically chosen by device
      fotaDwFtp.socketCfg.inactivityTimeout = 90; //seconds [1-65535], 0 -> no timetout
      fotaDwFtp.socketCfg.connectionTimeout = 600; //hundreds of ms [10-1200], 0 -> notimeout
      fotaDwFtp.socketCfg.dataSendingTimeout = 50; //hundreds of ms [1-255], 0-> no timeout

      //Get Delta FOTA
      AZX_LOG_DEBUG("\r\nTrying to download \"%s\" delta file...\r\n", FILE_URI);
      retVal = m2mb_fota_get_delta( fota_handle,M2MB_FOTA_DOWNLOAD_TYPE_FTP,(void*)(&fotaDwFtp) );

      if (retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_fota_get_delta FAIL\r\n");
        azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
      }
      else
      {
        AZX_LOG_DEBUG("m2mb_fota_get_delta OK - Waiting for the completion callback\r\n");
      }
      break;

    case CHECKDELTA:

      AZX_LOG_DEBUG("Validating delta file...\r\n");
      retVal = m2mb_fota_update_package_check_setup( fota_handle, M2MB_FOTA_CHECK_SETUP_SOURCE );
      if (retVal != M2MB_RESULT_SUCCESS)
      {
        m2mb_fota_result_code_get(fota_handle, &res_code);
        AZX_LOG_DEBUG("Packet validation failed with code %d\r\n", res_code);
        azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
      }
      else
      {
        AZX_LOG_TRACE("Packet validation PASS!\r\n");
        azx_tasks_sendMessageToTask(fota_task_id,UPDATE,0,0);
      }
      break;

    case UPDATE:
      AZX_LOG_DEBUG("Packet is valid, start update...\r\n");
      retVal = m2mb_fota_start(fota_handle);
      if (retVal != M2MB_RESULT_SUCCESS)
      {
        AZX_LOG_DEBUG("m2mb_fota_start FAIL\r\n");
        azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
      }
      else
      {
        M2MB_RESULT_E retVal;
        M2MB_POWER_HANDLE h = NULL;

        AZX_LOG_DEBUG("m2mb_fota_start PASS\r\n");
        azx_sleep_ms(2000);
     
        retVal = m2mb_power_init(&h, NULL, NULL);
        if ( retVal == M2MB_RESULT_SUCCESS )
        {
          /*Create a session file so at next boot the app does not restart the FOTA*/
          if (-1 == m2mb_fs_open(SESSION_FILE, M2MB_O_RDONLY | M2MB_O_CREAT, M2MB_ALLPERMS))
          {
            AZX_LOG_ERROR("Cannot create session file!\r\n");
            break;
          }

          AZX_LOG_DEBUG("\r\nRebooting...After reboot there will be the new FW running on module!\r\n");
          m2mb_power_reboot( h );
        }
        else
        {
          AZX_LOG_DEBUG("m2mb_power_init FAIL\r\n");
          azx_tasks_sendMessageToTask(main_task_id,APPLICATION_EXIT,0,0);
        }
      }
      break;

    default:
      break;
    }

  return 0;
}

INT32 mainTask(INT32 type, INT32 param1, INT32 param2)
{
  (void) param1;
  (void) param2;

  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  M2MB_NET_HANDLE h;
  
  int ret;

  void *myUserdata = NULL;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;

  switch (type)
  {
    case INIT:
      AZX_LOG_DEBUG("INIT\r\n");
      /* Init events handler */
      osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "net_pdp_ev"));
      osRes = m2mb_os_ev_init( &net_pdp_evHandle, &evAttrHandle );

      if ( osRes != M2MB_OS_SUCCESS )
      {
        m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
        AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
        return -1;
      }
      else
      {
        AZX_LOG_DEBUG("m2mb_os_ev_init success\r\n");
      }

      retVal = m2mb_net_init(&h, NetCallback, myUserdata);
      if ( retVal == M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_DEBUG( "m2mb_net_init returned M2MB_RESULT_SUCCESS\r\n");
      }
      else
      {
        AZX_LOG_ERROR( "m2mb_net_init did not return M2MB_RESULT_SUCCESS\r\n" );
      }

      AZX_LOG_DEBUG("Waiting for registration...\r\n");

      retVal = m2mb_net_get_reg_status_info(h);
      if ( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
      }

      /*Wait for network registration event to occur (released in NetCallback function) */
      m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);



      AZX_LOG_DEBUG("Pdp context initialization\r\n");
      retVal = m2mb_pdp_init(&pdpHandle, PdpCallback, myUserdata);
      if ( retVal == M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_TRACE( "m2mb_pdp_init returned M2MB_RESULT_SUCCESS\r\n");
      }
      else
      {
        AZX_LOG_ERROR( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
      }

      azx_sleep_ms(2000);

      AZX_LOG_DEBUG("Activate PDP with APN %s on cid %d....\r\n", APN, PDP_CTX);
      retVal = m2mb_pdp_activate(pdpHandle, PDP_CTX, (CHAR *) APN, (CHAR *) NULL, (CHAR *) NULL, M2MB_PDP_IPV4);
      if ( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("Cannot activate pdp context.\r\n");
      }
      break;

    case APPLICATION_EXIT:
      if(pdpHandle)
      {
        ret = m2mb_pdp_deactivate(pdpHandle, PDP_CTX);
        if(ret != M2MB_RESULT_SUCCESS)
        {
          AZX_LOG_ERROR("CANNOT DEACTIVATE PDP\r\n");
          return -1;
        }
        else
        {
          AZX_LOG_DEBUG("m2mb_pdp_deactivate returned success \r\n");
        }
      }

      AZX_LOG_DEBUG("Application complete.\r\n");
      break;

    default:
      AZX_LOG_DEBUG( "Type not expected in main_task\r\n" );
      break;
  }

  return 0;
}

