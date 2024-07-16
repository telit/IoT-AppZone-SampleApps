/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    lwm2m_demo.c

  @brief
    The file contains the LWM2M utilities

  @details

  @version
    1.0.0
  @note


  @author


  @date
    21/07/2023
 */
/* Include files ================================================================================*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_rtc.h"

#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"

#include "m2mb_fs_stdio.h"
#include "m2mb_fs_posix.h"
#include "m2mb_fs_errno.h"

#include "m2mb_ati.h"
#include "m2mb_lwm2m.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"


#include "lwm2m_demo.h"
#include "lwm2m_utils.h"


/* Local defines ================================================================================*/
#define CTX_ID      1 /*PDP context ID used by LWM2M, 1 by default*/
#define USB0_AT_INSTANCE 1 /*  AT instance to be used for URC, by default it is the one linked to USB0*/

#define UPDATE_EV_BIT 0x00000001



/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

/*===== Utility strings to print statuses by numeric value =====*/
const char *IF_STATUS_STRING[] =
{
    "M2MB_LWM2M_IF_STATE_NONE",
    "M2MB_LWM2M_IF_STATE_DISABLED",
    "M2MB_LWM2M_IF_STATE_ACK_WAIT",
    "M2MB_LWM2M_IF_STATE_ACTIVE",
    "M2MB_LWM2M_IF_STATE_IDLE",
    "M2MB_LWM2M_IF_STATE_DEREG_WAIT",
    "M2MB_LWM2M_IF_STATE_DEREGISTERING"
};

const char *CL_STATUS_STRING[] =
{
    "M2MB_LWM2M_CL_STATE_DISABLED",
    "M2MB_LWM2M_CL_STATE_BOOTSTRAPPING",
    "M2MB_LWM2M_CL_STATE_BOOTSTRAPPED",
    "M2MB_LWM2M_CL_STATE_REGISTERING",
    "M2MB_LWM2M_CL_STATE_REGISTERED",
    "M2MB_LWM2M_CL_STATE_DEREGISTERING",
    "M2MB_LWM2M_CL_STATE_SUSPENDED"
};

const char *FOTA_INTERNAL_STATUS_STRING[] =
{
    "LWM2M_FOTA_INTERNAL_STATUS_IDLE",
    "LWM2M_FOTA_INTERNAL_STATUS_FOTA_REQ_INIT",
    "LWM2M_FOTA_INTERNAL_STATUS_WAIT_DWL_ACK",
    "LWM2M_FOTA_INTERNAL_STATUS_DOWNLOADING",
    "LWM2M_FOTA_INTERNAL_STATUS_DOWNLOADED",
    "LWM2M_FOTA_INTERNAL_STATUS_DELTA_CHECK",
    "LWM2M_FOTA_INTERNAL_STATUS_WAIT_UPD_ACK",
    "LWM2M_FOTA_INTERNAL_STATUS_UPDATING",
    "LWM2M_FOTA_INTERNAL_STATUS_REJECT",
    "LWM2M_FOTA_INTERNAL_STATUS_FAILED"
};

const char *FOTA_FW_OBJ_STATE_STRING[] =
{
    "LWM2M_FW_OBJ_STATE_IDLE",
    "LWM2M_FW_OBJ_STATE_DOWNLOADING",
    "LWM2M_FW_OBJ_STATE_DOWNLOADED",
    "LWM2M_FW_OBJ_STATE_UPDATING"
};

/*===== static variables for threads and events =====*/
static INT32 lwm2m_taskID;

static M2MB_OS_EV_HANDLE update_evHandle = NULL;



/*===== ONEEDGE related globals =====*/

/*Handles*/
static M2MB_LWM2M_HANDLE lwm2mHandle;


static M2MB_LWM2M_GET_STAT_RES_T lwm2mStat;

/*URI objects*/

static M2MB_LWM2M_OBJ_URI_T _obj_telit_service_uri          = { M2MB_LWM2M_URI_4_FIELDS,
    33211, 0, 0, 1};



/*===== AT instance related globals and defines =====*/

#define AT_BUF_SIZE 4096
static char g_at_rsp_buf[AT_BUF_SIZE];
static UINT16 buf_index = 0;


static M2MB_ATI_HANDLE ati_handles[2];

static M2MB_OS_SEM_HANDLE at_rsp_sem = NULL;
INT32 at_taskId;




/* Local function prototypes ====================================================================*/

/*!
 * @brief        AT instance callback function. It is called when data is received from the registered AT instance
 *
 * @param[in]    h           the AT handle associated to the instance where the data arrived
 * @param[in]    ati_event   uri structure carrying the object and resource ids
 * @param[in]    resp_size   variable holding the type size of data
 * @param[in]    resp_struct variable holding the size of incoming data,in bytes. depending resp_size, it can be a INT16 pointer or INT32 pointer
 *
 */
static void at_cmd_async_callback ( M2MB_ATI_HANDLE h, M2MB_ATI_EVENTS_E ati_event, UINT16 resp_size, void *resp_struct, void *userdata );


/*!
 * @brief        AT initialization utility. Configures the required AT instance to send/receive commands
 *
 * @param[in]    instance     The instance number to be initialized (0-2)
 * @retval       M2MB_RESULT_E value
 *
 */
static M2MB_RESULT_E at_cmd_async_init(INT16 instance);

/*!
 * @brief        AT deinitialization utility. Releases the required AT instance
 *
 * @param[in]    instance     The instance number to be released (0-2)
 * @retval       M2MB_RESULT_E value
 *
 */
M2MB_RESULT_E at_cmd_async_deinit(INT16 instance);


/*!
 * @brief        AT URC callback, called with #OTAEV URCs arrive
 *
 * @param[in]    msg     The incoming urc message
 *
 */
static void otaev_urc_cb(const CHAR* msg);


/*!
 * @brief        fota ring indication manager, called when M2MB_LWM2M_FOTA_RING_IND arrive
 *
 *
 * @param[in]    action     The incoming fota ring action
 *
 */

static void fotaring_indication(M2MB_LWM2M_FOTA_RING_ACTION_E action);

/*!
 * @brief        Initialized the event handle for lwm2m events
 *
 * @retval       0 in case of success, > 0 in case of error
 *
 */
static INT32 init_ev_handle(void);

/*!
 * @brief        Performs a lwm2m fota ack when one of the events arrive
 *
 * @retval       0 in case of success, > 0 in case of error
 *
 */
static INT32 lwm2m_perform_ack(void);


/*!
 * @brief        Task callback function. It is called when a data event arrives in the LwM2M CB to read changed resources
 *
 * @param[in]    event    The incoming event. EV_MON_URC_RECEIVED is supported for now
 * @param[in]    i_uri    uri structure carrying the object and resource ids
 * @param[in]    param2   unused
 * @retval               0 if OK, other values error
 *
 */
static INT32 lwm2m_taskCB( INT32 event, INT32 info, INT32 param2);




/**
  @brief        Callback function for Client generated LWM2M events

  WARNING: do not place blocking action within callback, since it is issued by the agent's task!!

  @param[in]    h           LWM2M handle
  @param[in]    event       Event Identification
  @param[in]    resp_size   Size of the response buffer
  @param[in]    resp_struct Address of response buffer
  @param[in]    userdata    Address of user data
 */
static void lwm2mIndicationCB( M2MB_LWM2M_HANDLE h, M2MB_LWM2M_EVENT_E event,
    UINT16 resp_size, void *resp_struct, void *userdata );



/* Static functions =============================================================================*/

static void at_cmd_async_callback ( M2MB_ATI_HANDLE h, M2MB_ATI_EVENTS_E ati_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
  (void)userdata;

  INT32 resp_len;
  INT16 resp_len_short;
  AZX_LOG_TRACE("ati callback! Event: %d; resp_size: %u\r\n", ati_event, resp_size);
  if(ati_event == M2MB_RX_DATA_EVT )
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
    AZX_LOG_TRACE("Callback - available bytes: %d\r\n", resp_len);
    azx_tasks_sendMessageToTask(at_taskId, resp_len, (INT32)h, 0);
  }
}


/*-----------------------------------------------------------------------------------------------*/

static M2MB_RESULT_E at_cmd_async_init(INT16 instance)
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


/*-----------------------------------------------------------------------------------------------*/

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


/*-----------------------------------------------------------------------------------------------*/

static void otaev_urc_cb(const CHAR* msg)
{
  AZX_LOG_TRACE("otaev_urc_cb: %s\r\n", msg);
  if(strstr(msg, "FOTA REQUEST DROPPED"))
  {
    AZX_LOG_INFO("\r\n\r\nFOTA REQUEST DROPPED!\r\n");
  }
  else  if(strstr(msg, "DOWNLOAD FAILED"))
  {
    AZX_LOG_INFO("\r\n\r\nFOTA DOWNLOAD FAILED!\r\n");
  }
}


/*-----------------------------------------------------------------------------------------------*/


static void fotaring_indication(M2MB_LWM2M_FOTA_RING_ACTION_E action)
{

  AZX_LOG_TRACE("fotaring_indication: %d\r\n", action);

  if(action == M2MB_LWM2M_FW_OBJECT_ACTION_DOWNLOAD)
  {
    AZX_LOG_DEBUG("\r\n\r\nFOTA DOWNLOAD indication!\r\n");
    AZX_LOG_INFO("FOTA process is waiting for Download ACK, sending it!\r\n");
    if (0 != lwm2m_perform_ack())
    {
      return;
    }
  }
  else if(action == M2MB_LWM2M_FW_OBJECT_ACTION_UPDATE)
  {
    AZX_LOG_DEBUG("\r\n\r\nFOTA UPDATE indication!\r\n");
    M2MB_LWM2M_FOTA_STATE_T fotaStateParams;
    M2MB_RESULT_E retVal = m2mb_lwm2m_fota_status(lwm2mHandle, &fotaStateParams );
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      UINT32                  curEvBits;
      AZX_LOG_TRACE( "m2mb_lwm2m_fota_status request succeeded, waiting callback...\r\n" );

      if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
              get_lwm2mEvents_handle(),
              EV_LWM2M_FOTA_STATUS_RES_BIT,
              M2MB_OS_EV_GET_ANY_AND_CLEAR,
              &curEvBits,
              M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
      )
      )
      {
        AZX_LOG_ERROR("m2mb_lwm2m_fota_status timeout!\r\n");
        return;
      }
      else
      {
        AZX_LOG_TRACE("\r\n\r\nFOTA UPDATE OBJ STATUS: %s\r\nFOTA FW STATUS: %s\r\nremaining time: %llu\r\n\r\n",
                FOTA_FW_OBJ_STATE_STRING[fotaStateParams.fwUpdObjectStatus],
                FOTA_INTERNAL_STATUS_STRING[fotaStateParams.firmwareStatus],
                fotaStateParams.remainingTime);


        if((fotaStateParams.fwUpdObjectStatus == LWM2M_FW_OBJ_STATE_UPDATING) &&
                (fotaStateParams.firmwareStatus == LWM2M_FOTA_INTERNAL_STATUS_WAIT_UPD_ACK))
        {
          AZX_LOG_INFO("Waiting for Update ACK!\r\n");

          AZX_LOG_INFO("Before sending the ACK to apply FOTA, stop any ongoing operation (filesystem, sockets)\r\n");
          m2mb_os_ev_set(update_evHandle, UPDATE_EV_BIT, M2MB_OS_EV_SET);
        }
      }
    }
  }
}

/*-----------------------------------------------------------------------------------------------*/

static INT32 atTaskCB(INT32 len, INT32 h, INT32 param2)
{
  UNUSED_2(len,param2);
  SSIZE_T rsp_len;
  M2MB_ATI_HANDLE handle = (M2MB_ATI_HANDLE) h;


  rsp_len = m2mb_ati_rcv_resp(handle, g_at_rsp_buf + buf_index, AT_BUF_SIZE - buf_index);
  if(rsp_len != -1)
  {
    buf_index += rsp_len;

    AZX_LOG_TRACE("Buffer content (%d): <%s>\r\n", buf_index,  g_at_rsp_buf);
    char *p = strstr(g_at_rsp_buf,"#OTAEV");
    if(p)
    {
      char *e = strstr(p, "\r\n");
      if(e)
      {
        *e = '\0';
      }
      AZX_LOG_TRACE("OTAEV URC received.\r\n");
      otaev_urc_cb(p);
      memset(g_at_rsp_buf,0, AT_BUF_SIZE);
      buf_index = 0;
    }
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------*/

static INT32 init_ev_handle(void)
{
  {
    M2MB_OS_RESULT_E        osRes;
    M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
    INT32 result = 0;

    if (NULL == update_evHandle)
    {
      /* Init events handler */
      m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "lwm2m_ev"));
      osRes = m2mb_os_ev_init( &update_evHandle, &evAttrHandle );
      if ( osRes != M2MB_OS_SUCCESS )
      {
        m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
        update_evHandle = NULL;
        AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
        result += -2;
      }
      else
      {
        AZX_LOG_TRACE("m2mb_os_ev_init success\r\n");
        result = 0;
      }
    }

    return result;
  }
}


/*-----------------------------------------------------------------------------------------------*/

static INT32 lwm2m_perform_ack(void)
{
  INT32   retVal;
  UINT32  curEvBits;
  M2MB_LWM2M_FOTA_ACK_REQ_T fotaAckParams;
  fotaAckParams.action     = M2MB_LWM2M_FOTA_ACK_ALL;
  retVal = m2mb_lwm2m_fota_ack(lwm2mHandle, &fotaAckParams);
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
        get_lwm2mEvents_handle(),
        EV_LWM2M_FOTA_ACK_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_fota_ack timeout!\r\n");
      return 1;
    }
    AZX_LOG_TRACE( "m2mb_lwm2m_fota_ack request succeeded\r\n" );
    return 0;
  }
  else
  {
    AZX_LOG_ERROR("m2mb_lwm2m_fota_ack failed\r\n");
    return 2;
  }

}


/*-----------------------------------------------------------------------------------------------*/

static INT32 lwm2m_taskCB( INT32 event, INT32 i_uri, INT32 param2)
{
  UNUSED_2(i_uri, param2);

  switch(event)
  {
    case EV_MON_URC_RECEIVED:
    {
      M2MB_LWM2M_OBJ_URI_T *pUri = (M2MB_LWM2M_OBJ_URI_T *) i_uri;
      M2MB_LWM2M_OBJ_URI_T uri = *pUri;

      m2mb_os_free(pUri);

      uri.uriLen = M2MB_LWM2M_URI_4_FIELDS;
      AZX_LOG_TRACE("Asking a read operation for {%u/%u/%u/%u (%u)}\r\n",
              uri.obj, uri.objInst, uri.resource, uri.resourceInst, uri.uriLen );

      if (uri.obj == FIRMWARE_UPDATE_OBJ_ID)
      {
        INT32 retVal;
        M2MB_LWM2M_FOTA_STATE_T fotaStateParams;
        AZX_LOG_DEBUG("Firmware update event!\r\n");

        retVal = m2mb_lwm2m_fota_status(lwm2mHandle, &fotaStateParams );
        if ( retVal == M2MB_RESULT_SUCCESS )
        {
          UINT32                  curEvBits;
          AZX_LOG_TRACE( "m2mb_lwm2m_fota_status request succeeded, waiting callback...\r\n" );

          if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
                  get_lwm2mEvents_handle(),
                  EV_LWM2M_FOTA_STATUS_RES_BIT,
                  M2MB_OS_EV_GET_ANY_AND_CLEAR,
                  &curEvBits,
                  M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
          )
          )
          {
            AZX_LOG_ERROR("m2mb_lwm2m_fota_status timeout!\r\n");
            return 1;
          }
          else
          {
            AZX_LOG_TRACE("\r\n\r\nFOTA UPDATE OBJ STATUS: %s\r\nFOTA FW STATUS: %s\r\nremaining time: %llu\r\n\r\n",
                    FOTA_FW_OBJ_STATE_STRING[fotaStateParams.fwUpdObjectStatus],
                    FOTA_INTERNAL_STATUS_STRING[fotaStateParams.firmwareStatus],
                    fotaStateParams.remainingTime);

            /*Cycle until the INIT status is complete*/
            while (fotaStateParams.firmwareStatus == LWM2M_FOTA_INTERNAL_STATUS_FOTA_REQ_INIT)
            {
              retVal = m2mb_lwm2m_fota_status(lwm2mHandle, &fotaStateParams );
              if ( retVal == M2MB_RESULT_SUCCESS )
              {
                UINT32                  curEvBits;
                AZX_LOG_TRACE( "m2mb_lwm2m_fota_status request succeeded, waiting callback...\r\n" );

                if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
                        get_lwm2mEvents_handle(),
                        EV_LWM2M_FOTA_STATUS_RES_BIT,
                        M2MB_OS_EV_GET_ANY_AND_CLEAR,
                        &curEvBits,
                        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
                )
                )
                {
                  AZX_LOG_ERROR("m2mb_lwm2m_fota_status timeout!\r\n");
                  return 1;
                }
                else
                {
                  AZX_LOG_TRACE("\r\n\r\nFOTA UPDATE OBJ STATUS: %s\r\nFOTA FW STATUS: %s\r\nremaining time: %llu\r\n\r\n",
                          FOTA_FW_OBJ_STATE_STRING[fotaStateParams.fwUpdObjectStatus],
                          FOTA_INTERNAL_STATUS_STRING[fotaStateParams.firmwareStatus],
                          fotaStateParams.remainingTime);
                }
              }
              azx_sleep_ms(1000);
            }
          }
        }
        else
        {
          AZX_LOG_ERROR( "m2mb_lwm2m_fota_status failed!\r\n");
        }
      }
      else
      {
        AZX_LOG_WARN("Unexpected object ID %u\r\n", uri.obj);
      }
    }
    break;

    case EV_FOTA_RING_IND:
      AZX_LOG_DEBUG("FOTA RING indication with event %d\r\n", param2);
      fotaring_indication((M2MB_LWM2M_FOTA_RING_ACTION_E)param2);
      break;
    default:
      AZX_LOG_WARN("Unmanaged task event %d\r\n", event);
      break;
  } /* switch(event) */

  return 1;
}

/*-----------------------------------------------------------------------------------------------*/
/*!
 * @brief        LwM2M indication callback function. It is called when any event happens LwM2M wise
 *
 * @param[in]    h            The LwM2M client handle
 * @param[in]    event        the specific event
 * @param[in]    resp_size    unused here
 * @param[in]    resp_struct  Depending on the event, the structure carrying the data
 * @retval       none
 *
 */
static void lwm2mIndicationCB( M2MB_LWM2M_HANDLE h, M2MB_LWM2M_EVENT_E event, UINT16 resp_size, void *resp_struct,
    void *userdata )
{
  ( void ) h;
  ( void ) resp_size;
  LWM2M_EVENT_RES_S *pEvRes = (LWM2M_EVENT_RES_S *)userdata;

  /* Client generated events */
  switch( event )
  {

  /* event in response to m2mb_lwm2m_enable() */
  case M2MB_LWM2M_ENABLE_RES:
  {
    M2MB_LWM2M_ENABLE_RES_T *resp = ( M2MB_LWM2M_ENABLE_RES_T * ) resp_struct;
    if(resp->result == M2MB_LWM2M_RES_SUCCESS)
    {
      AZX_LOG_INFO( "LWM2M enable result OK\r\n");
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_ENABLE_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "Enable result %d\r\n", resp->result );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  /* event in response to m2mb_lwm2m_disable() */
  case M2MB_LWM2M_DISABLE_RES:
  {
    M2MB_LWM2M_DISABLE_RES_T *resp = ( M2MB_LWM2M_DISABLE_RES_T * ) resp_struct;
    if(resp->result == M2MB_LWM2M_RES_SUCCESS)
    {
      AZX_LOG_INFO( "LWM2M disable result OK\r\n");
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_DISABLE_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "Disable result %d\r\n", resp->result );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  /* event in response to m2mb_lwm2m_get() */
  case M2MB_LWM2M_GET_RES:
  {
    M2MB_LWM2M_GET_RES_T *resp = ( M2MB_LWM2M_GET_RES_T * )resp_struct;

    /*copy in the event result structure (provided during m2mb_lwm2m_init) the relevant info*/
    pEvRes->resp_len = resp->len;
    pEvRes->uri = resp->reqURI;
    pEvRes->result = resp->result;

    if(resp->result == M2MB_LWM2M_RES_SUCCESS)
    {
      AZX_LOG_TRACE( "M2MB_LWM2M_GET_RES, result OK\r\n" );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_GET_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "get result %d\r\n", resp->result );
      m2mb_os_ev_set(get_lwm2mEvents_handle(),  EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  /* event in response to m2mb_lwm2m_set() */
  case M2MB_LWM2M_SET_RES:
  {
    M2MB_LWM2M_SET_RES_T *resp = ( M2MB_LWM2M_SET_RES_T * )resp_struct;
    if(resp->result == M2MB_LWM2M_RES_SUCCESS)
    {
      AZX_LOG_TRACE( "M2MB_LWM2M_SET_RES, result OK\r\n" );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_SET_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "set result %d\r\n", resp->result );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  /* event in response to m2mb_lwm2m_read() */
  case M2MB_LWM2M_READ_RES:
  {
    M2MB_LWM2M_READ_RES_T *resp = ( M2MB_LWM2M_READ_RES_T * )resp_struct;

    /*copy in the event result structure (provided during m2mb_lwm2m_init) the relevant info*/
    pEvRes->resp_len = resp->len;
    pEvRes->uri = resp->reqURI;
    pEvRes->result = resp->result;

    AZX_LOG_TRACE( "M2MB_LWM2M_READ_RES, result: %d\r\n", resp->result );

    if( resp->result == M2MB_LWM2M_RES_SUCCESS )
    {
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_READ_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_ERROR("READ failed with error %d\r\n", resp->result );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  /* event in response to m2mb_lwm2m_write() */
  case M2MB_LWM2M_WRITE_RES:
  {
    M2MB_LWM2M_WRITE_RES_T *resp = ( M2MB_LWM2M_WRITE_RES_T * )resp_struct;
    if(resp->result == M2MB_LWM2M_RES_SUCCESS)
    {
      AZX_LOG_TRACE( "M2MB_LWM2M_WRITE_RES, result OK\r\n");
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_WRITE_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "write res %d\r\n", resp->result );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }
  case M2MB_LWM2M_EXIST_RES:
  {
    M2MB_LWM2M_EXIST_RES_T *resp = ( M2MB_LWM2M_EXIST_RES_T * )resp_struct;

    if( resp->result == M2MB_LWM2M_RES_SUCCESS )
    {
      /*copy in the event result structure (provided during m2mb_lwm2m_init) the relevant info*/
      pEvRes->resp_len = resp->isExistent;
      pEvRes->uri = resp->reqURI;
      pEvRes->result = resp->result;

      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_EXIST_RES_BIT, M2MB_OS_EV_SET);

    }
    else
    {
      /*if the result is not SUCCESS, but it is M2MB_LWM2M_RES_FAIL_BAD_URI, the URI is not present.*/
      if(resp->result == M2MB_LWM2M_RES_FAIL_BAD_URI )
      {
        /*copy in the event result structure (provided during m2mb_lwm2m_init) the relevant info*/
        pEvRes->resp_len = 0;
        pEvRes->uri = resp->reqURI;
        pEvRes->result = resp->result;
        m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_EXIST_RES_BIT, M2MB_OS_EV_SET);
      }
      else
      {
        /*Generic failure*/
        AZX_LOG_WARN( "exist res %d\r\n", resp->result);
        m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
      }
    }
    break;
  }

  /* event in response to m2mb_lwm2m_new_inst() */
  case M2MB_LWM2M_NEW_INST_RES:
  {
    // event in response to the m2mb_lwm2m_newinst
    M2MB_LWM2M_NEW_INST_RES_T *resp = ( M2MB_LWM2M_NEW_INST_RES_T * )resp_struct;

    switch( resp->result )
    {
    case M2MB_LWM2M_RES_SUCCESS:
      AZX_LOG_TRACE( "New Instance created successfully\r\n" );
      break;

    case M2MB_LWM2M_RES_FAIL_NOT_ALLOWED:
      AZX_LOG_DEBUG( "New instance creation not allowed (already present?)\r\n" );
      break;

    default:
      AZX_LOG_DEBUG( "Creating object instance failed. Result: %d\r\n", resp->result );
      break;
    }

    break;
  }


  case  M2MB_LWM2M_SRV_INFO_IND:
  {

    M2MB_LWM2M_SRV_INFO_IND_T *resp = ( M2MB_LWM2M_SRV_INFO_IND_T * )resp_struct;

    AZX_LOG_TRACE("M2MB_LWM2M_SRV_INFO_IND, resp->info = %d\r\n", resp->info);

    switch(resp->info)
    {
    case M2MB_LWM2M_CL_STATE_DISABLED:
      break;
    case M2MB_LWM2M_CL_STATE_BOOTSTRAPPING:
      AZX_LOG_INFO( "resp->info == M2MB_LWM2M_CL_STATE_BOOTSTRAPPING\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_BOOTSTRAPPED:
      AZX_LOG_INFO( "resp->info == M2MB_LWM2M_CL_STATE_BOOTSTRAPPED\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_REGISTERING:
      AZX_LOG_INFO( "resp->info == M2MB_LWM2M_CL_STATE_REGISTERING\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_REGISTERED:
      AZX_LOG_INFO( "resp->info == M2MB_LWM2M_CL_STATE_REGISTERED\r\n" );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_SRV_REG_BIT, M2MB_OS_EV_SET);
      break;
    case M2MB_LWM2M_CL_STATE_DEREGISTERING:
      AZX_LOG_INFO( "resp->info == M2MB_LWM2M_CL_STATE_DEREGISTERING\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_SUSPENDED:
      AZX_LOG_INFO( "resp->info == M2MB_LWM2M_CL_STATE_SUSPENDED\r\n" );
      break;
    default:
      AZX_LOG_WARN( "resp->info: unexpected value!! %d\r\n", resp->info);
      break;
    }
    break;
  }

  /* event in case a resource changed (if monitoring is enabled) */
  case M2MB_LWM2M_MON_INFO_IND:
  {
    /*
      An URC was received, like
      #LWM2MMON: UPD,"/3311/x/5850/0
     */
    /* event that brings a resource change information */
    M2MB_LWM2M_OBJ_URI_T * p_uri = (M2MB_LWM2M_OBJ_URI_T *)m2mb_os_malloc(sizeof(M2MB_LWM2M_OBJ_URI_T));

    M2MB_LWM2M_MON_INFO_IND_T *pInfo = ( M2MB_LWM2M_MON_INFO_IND_T * )resp_struct;

    AZX_LOG_TRACE("\r\nM2MB_LWM2M_MON_INFO_IND\r\n");

    if(p_uri)
    {
      memcpy(p_uri, &(pInfo->uri), sizeof(M2MB_LWM2M_OBJ_URI_T));
      AZX_LOG_INFO( "\r\nResource /%u/%u/%u/%u changed!\r\n",
          p_uri->obj, p_uri->objInst,
          p_uri->resource, p_uri->resourceInst
      );

      azx_tasks_sendMessageToTask( lwm2m_taskID, EV_MON_URC_RECEIVED, (INT32)p_uri, 0);
    }
    else
    {
      AZX_LOG_ERROR("Cannot allocate uri struct\r\n");
    }
    break;
  }

  case M2MB_LWM2M_NFYACK_URI_RES:
  {
    /* event in response to the m2mb_lwm2m_nfy_ack_uri */
    M2MB_LWM2M_NFYACK_URI_RES_T *resp = ( M2MB_LWM2M_NFYACK_URI_RES_T * )resp_struct;
    if( ( resp ) && ( resp->result == M2MB_LWM2M_RES_SUCCESS ) )
    {
      // code
      AZX_LOG_TRACE( "m2mb_lwm2m_nfy_ack_uri response OK\r\n" );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_NFYADD_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  case M2MB_LWM2M_NFYACK_STATUS_RES:
  {
    /* event in response to the m2mb_lwm2m_nfy_ack_status */
    M2MB_LWM2M_NFYACK_STATUS_RES_T *resp = ( M2MB_LWM2M_NFYACK_STATUS_RES_T * )resp_struct;

    if( ( resp ) && ( resp->result == M2MB_LWM2M_RES_SUCCESS ) )
    {
      AZX_LOG_TRACE( "Current status for Notify Ack reporting is %s\r\n\r\n", ( ( resp->enabled == TRUE ) ? ( "ENABLED" ) : ( "DISABLED" ) ) );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_NFYSTAT_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  case M2MB_LWM2M_NFYACK_LIST_RES:
  {
    /* event in response to the m2mb_lwm2m_nfy_ack_list */
    M2MB_LWM2M_NFYACK_LIST_RES_T *resp = ( M2MB_LWM2M_NFYACK_LIST_RES_T * )resp_struct;

    if( ( resp ) && ( resp->result == M2MB_LWM2M_RES_SUCCESS ) )
    {
      AZX_LOG_TRACE( "m2mb_lwm2m_nfy_ack_list %hu elements read\r\n", resp->listElementsNumber );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_NFYACK_LIST_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  case M2MB_LWM2M_NFYACK_INFO_IND:
  {
    /* event that brings a Notify Ack information */
    M2MB_LWM2M_NFYACK_INFO_IND_T *pInfo = ( M2MB_LWM2M_NFYACK_INFO_IND_T * )resp_struct;

    if( pInfo )
    {
      AZX_LOG_TRACE( "#LWM2MNFYACK: agentId %hu, SSID %hu, uri \"/%hu/%hu/%hu\", %s\r\n",
          pInfo->agent,
          pInfo->shServerId,
          pInfo->uri.obj,
          pInfo->uri.objInst,
          pInfo->uri.resource,
          ( pInfo->nfyState == M2MB_LWM2M_NFY_STATE_ACK_RECEIVED ) ? ( "ACK RECEIVED" ) : ( "ACK NOT RECEIVED" ) );

      if(pInfo->nfyState == M2MB_LWM2M_NFY_STATE_ACK_RECEIVED )
      {
        m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_NFYACK_BIT, M2MB_OS_EV_SET);
      }
    }
    break;
  }

  case  M2MB_LWM2M_SESSION_INFO_IND:
  {
    AZX_LOG_TRACE( "\r\nM2MB_LWM2M_SESSION_INFO_IND\r\n" );
    M2MB_LWM2M_INFO_IND_T *resp = ( M2MB_LWM2M_INFO_IND_T * )resp_struct;

    if( resp->info == M2MB_LWM2M_INFO_RESOURCE_EXECUTE )
    {
      M2MB_LWM2M_OBJ_URI_T *pUri = ( M2MB_LWM2M_OBJ_URI_T * )resp->data;
      M2MB_LWM2M_OBJ_URI_T execUri = *pUri;
      AZX_LOG_INFO( "\r\nInfo Exec Ind: %u/%u/%u/%u\r\n", execUri.obj, execUri.objInst,
          execUri.resource, execUri.resourceInst );
    }
    if( resp->info == M2MB_LWM2M_INFO_FOTA_REBOOT )
    {
      AZX_LOG_INFO( "\r\n------- Info, DEVICE REBOOT FOR FOTA ------\r\n" );
    }
    break;
  }

  /* event in response to m2mb_lwm2m_mon() */
  case M2MB_LWM2M_MON_RES:
  {
    M2MB_LWM2M_MON_RES_T *resp = ( M2MB_LWM2M_MON_RES_T * )resp_struct;
    AZX_LOG_TRACE( "Resource change monitor setting result: %d\r\n", resp->result );

    if( resp->result == M2MB_LWM2M_RES_SUCCESS )
    {
      AZX_LOG_INFO("Monitoring enabled.\r\n\r\n");
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_MON_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  /* event in response to m2mb_lwm2m_get_stat() */
  case M2MB_LWM2M_GET_STAT_RES:
  {
    M2MB_LWM2M_GET_STAT_RES_T *resp = ( M2MB_LWM2M_GET_STAT_RES_T * )resp_struct;

    AZX_LOG_TRACE( "Get Stat result: %d\r\n", resp->result );

    if( resp->result == M2MB_LWM2M_RES_SUCCESS )
    {
      AZX_LOG_INFO("\r\nIF Status: %s\r\nClient Status: %s\r\n",
          IF_STATUS_STRING[resp->status], CL_STATUS_STRING[resp->clStatus]);
      memcpy(&lwm2mStat, resp, sizeof(M2MB_LWM2M_GET_STAT_RES_T));
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_GET_STAT_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  case M2MB_LWM2M_FOTA_CFG_RES:
  { // event in response to the m2mb_lwm2m_fota_cfg
    M2MB_LWM2M_FOTA_CFG_RES_T *resp = ( M2MB_LWM2M_FOTA_CFG_RES_T * )resp_struct;

    if( resp != NULL )
    {
      AZX_LOG_TRACE( "Returning resource type: result %d\r\n", resp->result );

      if( resp->result == M2MB_LWM2M_RES_SUCCESS )
      {
        AZX_LOG_TRACE( "Fota config succeeded\r\n" );
        m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FOTA_CFG_RES_BIT, M2MB_OS_EV_SET);
      }
    }
    break;
  }
  case M2MB_LWM2M_FOTA_STATE_RES:
  {
    // event in response to the m2mb_lwm2m_fota_status
    M2MB_LWM2M_FOTA_STATE_RES_T *resp = ( M2MB_LWM2M_FOTA_STATE_RES_T * )resp_struct;

    if( resp != NULL )
    {
      AZX_LOG_TRACE( "Returning resource type: result %d\r\n", resp->result );

      if( resp->result == M2MB_LWM2M_RES_SUCCESS )
      {
        AZX_LOG_TRACE( "Fota status read succeeded\r\n" );
        m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FOTA_STATUS_RES_BIT, M2MB_OS_EV_SET);
      }
    }
    break;
  }
  case M2MB_LWM2M_FOTA_ACK_RES:
  { // event in response to the m2mb_lwm2m_fota_ack
    M2MB_LWM2M_FOTA_ACK_RES_T *resp = ( M2MB_LWM2M_FOTA_ACK_RES_T * )resp_struct;

    if( resp != NULL )
    {
      AZX_LOG_TRACE( "Returning resource type: result %d", resp->result );

      if( resp->result == M2MB_LWM2M_RES_SUCCESS )
      {
        AZX_LOG_TRACE( "Fota ack succeeded\r\n" );
        m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FOTA_ACK_RES_BIT, M2MB_OS_EV_SET);
      }
    }
    break;
  }
  case M2MB_LWM2M_FOTA_RING_IND:
  { // event that matches FOTA RING URCs
    M2MB_LWM2M_FOTA_RING_INFO_IND_T *resp = ( M2MB_LWM2M_FOTA_RING_INFO_IND_T * )resp_struct;
    if( resp != NULL )
    {
      AZX_LOG_DEBUG( "FOTA RING EVENT %d\r\n", resp->fotaEvent );
      azx_tasks_sendMessageToTask( lwm2m_taskID, EV_FOTA_RING_IND, 0, (INT32)resp->fotaEvent);
    }
    break;
  }

  default:
    AZX_LOG_DEBUG( "LWM2M EVENT %d\r\n", event );
    break;
  }
}


/*-----------------------------------------------------------------------------------------------*/



/* Global functions =============================================================================*/

UINT8 oneedge_init( void)
{
  M2MB_RESULT_E retVal;
  M2MB_LWM2M_ENABLE_REQ_T pars;

  INT32 service_enable = 1;
  M2MB_LWM2M_FOTA_CFG_REQ_T cfg_params;

  M2MB_OS_RESULT_E        osRes;
  UINT32                  curEvBits;
  UINT16 agentID = 0;     /* Telit agent ID */

  memset(get_event_res_p(),0, sizeof(LWM2M_EVENT_RES_S));


  //get the handle of the lwm2m client on _h
  retVal = m2mb_lwm2m_init( &lwm2mHandle, lwm2mIndicationCB, ( void * )get_event_res_p() );

  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_init returned error %d\r\n", retVal );

    return 1;
  }

  m2mb_lwm2m_agent_config( lwm2mHandle, agentID );

  cfg_params.mode = M2MB_LWM2M_FOTA_MODE_ACK_ALL;
  cfg_params.toutAct = M2MB_LWM2M_FOTA_TOUT_ACTION_RESET_SM;

  retVal = m2mb_lwm2m_fota_cfg( lwm2mHandle, &cfg_params );
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_fota_cfg request failed\r\n" );
    m2mb_lwm2m_deinit( lwm2mHandle );
    lwm2mHandle = NULL;
    return 1;
  }

  if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
      get_lwm2mEvents_handle(),
      EV_LWM2M_FOTA_CFG_RES_BIT,
      M2MB_OS_EV_GET_ANY_AND_CLEAR,
      &curEvBits,
      M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
  )
  )
  {
    AZX_LOG_ERROR("m2mb_lwm2m_fota_cfg timeout!\r\n");

    azx_sleep_ms(2000);
    m2mb_lwm2m_deinit(lwm2mHandle);
    lwm2mHandle = NULL;
    return 1;
  }

  retVal = m2mb_lwm2m_write( lwm2mHandle, &_obj_telit_service_uri, &service_enable, sizeof( INT32 ) );
  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_write returned error %d\r\n", retVal );

    m2mb_lwm2m_deinit( lwm2mHandle );
    lwm2mHandle = NULL;
    return 1;
  }

  lwm2m_taskID = azx_tasks_createTask((char*) "LWM2M_TASK", AZX_TASKS_STACK_M, 4, AZX_TASKS_MBOX_S, lwm2m_taskCB);

  AZX_LOG_TRACE("Task ID: %d.\r\n", lwm2m_taskID);
  if(lwm2m_taskID <= 0)
  {
    AZX_LOG_ERROR("Cannot create lwm2m managing task!\r\n");

    m2mb_lwm2m_deinit( lwm2mHandle );
    lwm2mHandle = NULL;
    return 1;
  }

  /*Checking client status*/
  m2mb_lwm2m_get_stat(lwm2mHandle);

  if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
      get_lwm2mEvents_handle(),
      EV_LWM2M_GET_STAT_RES_BIT,
      M2MB_OS_EV_GET_ANY_AND_CLEAR,
      &curEvBits,
      M2MB_OS_MS2TICKS(2000) /*wait 2 seconds for the event to occur*/
  ))
  {
    AZX_LOG_ERROR("LWM2M get status timeout!\r\n");
    m2mb_lwm2m_deinit( lwm2mHandle );
    lwm2mHandle = NULL;
    return 2;
  }

  if(lwm2mStat.status == M2MB_LWM2M_IF_STATE_NONE || lwm2mStat.status == M2MB_LWM2M_IF_STATE_DISABLED)
  {
    AZX_LOG_INFO("Enable lwm2m client\r\n");
    pars.apnclass = CTX_ID; /*CID*/
    pars.guardRequestEventSecs = 5;
    pars.guardReleaseEventSecs = 5;
    pars.commandType = M2MB_LWM2MENA_CMD_TYPE_SET;
    pars.mode = M2MB_LWM2M_MODE_NO_ACK;

    retVal = m2mb_lwm2m_enable( lwm2mHandle, &pars );
    if( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_lwm2m_enable returned error %d\r\n", retVal );
      m2mb_lwm2m_deinit( lwm2mHandle );
      lwm2mHandle = NULL;
      return 1;
    }
    if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
        get_lwm2mEvents_handle(),
        EV_LWM2M_ENABLE_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_enable timeout!\r\n");

      azx_sleep_ms(2000);
      m2mb_lwm2m_deinit(lwm2mHandle);
      lwm2mHandle = NULL;
      return 1;
    }
  }
  else
  {
    AZX_LOG_INFO("Enable already done, skip. State: %s\r\n", IF_STATUS_STRING[lwm2mStat.status]);
  }
  azx_sleep_ms(1000);

  m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_CLEAR);


  /*Register a monitor on a resource by creating a URI object and passing it to m2mb_lwm2m_mon*/
  {
    M2MB_LWM2M_MON_REQ_T mon;

    M2MB_LWM2M_OBJ_URI_T uri_mon = {
        M2MB_LWM2M_URI_4_FIELDS,
        FIRMWARE_UPDATE_OBJ_ID, 0,
        0, 0
    };

    mon.mode   = M2MB_LWM2M_MON_MODE_SET_CMD;
    mon.action = M2MB_LWM2M_MON_ENABLE;

    retVal = m2mb_lwm2m_mon(lwm2mHandle, &uri_mon, &mon );
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "m2mb_lwm2m_mon request succeeded\r\n" );
    }

    if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
        get_lwm2mEvents_handle(),
        EV_LWM2M_MON_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_mon timeout!\r\n");


      m2mb_lwm2m_disable(lwm2mHandle);
      if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
          get_lwm2mEvents_handle(),
          EV_LWM2M_DISABLE_RES_BIT,
          M2MB_OS_EV_GET_ANY_AND_CLEAR,
          &curEvBits,
          M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
          )
      )
      {
        AZX_LOG_ERROR("m2mb_lwm2m_disable timeout!\r\n");
      }
      m2mb_lwm2m_deinit(lwm2mHandle);
      lwm2mHandle = NULL;
      return 1;
    }
  }

  AZX_LOG_INFO("Waiting LWM2M Registering (120 seconds timeout)...\r\n");
  osRes = m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_SRV_REG_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS(120000));
  if(osRes != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("LWM2M Register timeout!\r\n");


    m2mb_lwm2m_disable(lwm2mHandle);
    if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
        get_lwm2mEvents_handle(),
        EV_LWM2M_DISABLE_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
        )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_disable timeout!\r\n");
    }
    m2mb_lwm2m_deinit(lwm2mHandle);
    lwm2mHandle = NULL;
    return 1;
  }

  return 0;
}

/*-----------------------------------------------------------------------------------------------*/



INT32 msgLWM2MTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;

  UINT32                  curEvBits;
  INT32 ret = 0;

  int task_status = type;
  do
  {
    AZX_LOG_INFO("Initializing resources...\r\n");


    at_cmd_async_init(USB0_AT_INSTANCE);
    at_taskId = azx_tasks_createTask((CHAR*)"AT TASK", AZX_TASKS_STACK_XL, 10, AZX_TASKS_MBOX_M, atTaskCB);
    memset(g_at_rsp_buf,0, AT_BUF_SIZE);


    init_sync();
    init_ev_handle();

    azx_sleep_ms(8000);

    /*Initialize LWM2M*/
    ret = oneedge_init();
    if (ret != 0)
    {
      task_status = APPLICATION_EXIT;
      break;
    }



    /*Checking client status*/
    m2mb_lwm2m_get_stat(lwm2mHandle);

    if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
        get_lwm2mEvents_handle(),
        EV_LWM2M_GET_STAT_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(2000) /*wait 2 seconds for the event to occur*/
    ))
    {
      AZX_LOG_ERROR("LWM2M get status timeout!\r\n");
      return -2;
    }

    azx_sleep_ms(5000);


    AZX_LOG_INFO("\r\nWaiting for events from the OneEdge portal. Please start a FOTA process.\r\n\r\n");

    /*Wait for UPDATE event*/
    m2mb_os_ev_get(update_evHandle,
        UPDATE_EV_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_WAIT_FOREVER
    );

    AZX_LOG_INFO("\r\nAsked to perform UPDATE step, stopping all operations now.\r\n");

    azx_sleep_ms(2000);

    if (0 != lwm2m_perform_ack())
    {
      return 1;
    }

    task_status = APPLICATION_EXIT;
  } while(0);

  if (task_status == APPLICATION_EXIT)
  {
    deinit_sync();

    AZX_LOG_INFO("Application complete.\r\n");
  }

  return 0;
}

