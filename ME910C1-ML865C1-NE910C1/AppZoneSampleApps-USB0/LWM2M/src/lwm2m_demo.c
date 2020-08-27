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
    21/05/2020
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

#include "m2mb_lwm2m.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

#include "lwm2m_demo.h"


/* Local defines ================================================================================*/
#define APN         "web.omnitel.it"
#define CTX_ID      1 /*PDP context ID*/

/*OBJECTs and RESOURCEs IDs*/
#define DEVICE_OBJ_ID 3
#define   DEVICE_BATTERY_RES_ID 9 /*integer, percentage*/


#define LOCATION_OBJ_ID 6
#define   LOCATION_LATITUDE_RES_ID 0 /*float, degrees*/
#define   LOCATION_LONGITUDE_RES_ID 1 /*float, degrees*/

#define CONN_STATS_OBJ_ID 7
#define   CONN_STATS_STOP_EXEC_RES_ID 7
#define   CONN_STATS_COLLECTION_PERIOD_RES_ID 8 /* integer, in seconds */

#define APN_CONN_PROFILE_OBJ_ID 11
#define   APN_PROFILE_APN_1_RES_ID 1 /*first CID apn string*/

#define M2MB_LWM2M_DEMO_OBJ_ID          35000
#define   DEMO_STRING_R_RES_ID          1        /* single string resource, Read only*/
#define   DEMO_INT_R_RES_ID             2        /* single integer resource, Read only*/
#define   DEMO_FLOAT_R_RES_ID           3        /* single floating point resource, Read only*/
#define   DEMO_BOOL_R_RES_ID            4        /* single boolean resource, Read only*/
#define   DEMO_OPAQUE_R_RES_ID          5        /* single opaque (raw data) resource, Read only*/
#define   DEMO_TIME_R_RES_ID            6        /* single time resource, Read only*/
#define   DEMO_OBJLINK_R_RES_ID         7        /* single object link resource, Read only*/

#define   DEMO_STRING_RW_RES_ID         11       /* single string resource, Read + Write*/
#define   DEMO_INT_RW_RES_ID            12       /* single integer resource, Read + Write*/
#define   DEMO_FLOAT_RW_RES_ID          13       /* single floating point resource, Read + Write*/
#define   DEMO_BOOL_RW_RES_ID           14       /* single boolean resource, Read + Write*/
#define   DEMO_OPAQUE_RW_RES_ID         15       /* single opaque (raw data) resource, Read + Write*/
#define   DEMO_TIME_RW_RES_ID           16       /* single time resource, Read + Write*/
#define   DEMO_OBJLINK_RW_RES_ID        17       /* single object link resource, Read + Write*/

#define   DEMO_MULTI_STRING_R_RES_ID    21       /* multiple string resource, Read only*/
#define   DEMO_MULTI_INT_R_RES_ID       22       /* multiple integer resource, Read only*/
#define   DEMO_MULTI_FLOAT_R_RES_ID     23       /* multiple floating point resource, Read only*/
#define   DEMO_MULTI_BOOL_R_RES_ID      24       /* multiple boolean resource, Read only*/
#define   DEMO_MULTI_OPAQUE_R_RES_ID    25       /* multiple opaque (raw data) resource, Read only*/
#define   DEMO_MULTI_TIME_R_RES_ID      26       /* multiple time resource, Read only*/
#define   DEMO_MULTI_OBJLINK_R_RES_ID   27       /* multiple object link resource, Read only*/

#define   DEMO_MULTI_STRING_RW_RES_ID   31       /* multiple string resource, Read + Write*/
#define   DEMO_MULTI_INT_RW_RES_ID      32       /* multiple integer resource, Read + Write*/
#define   DEMO_MULTI_FLOAT_RW_RES_ID    33       /* multiple floating point resource, Read + Write*/
#define   DEMO_MULTI_BOOL_RW_RES_ID     34       /* multiple boolean resource, Read + Write*/
#define   DEMO_MULTI_OPAQUE_RW_RES_ID   35       /* multiple opaque (raw data) resource, Read + Write*/
#define   DEMO_MULTI_TIME_RW_RES_ID     36       /* multiple time resource, Read + Write*/
#define   DEMO_MULTI_OBJLINK_RW_RES_ID  37       /* multiple object link resource, Read + Write*/

#define   DEMO_EXEC1_RES_ID             101      /* execute resource 1*/
#define   DEMO_EXEC2_RES_ID             102      /* execute resource 2*/


#define OBJECT_XML_NAME "object_35000.xml"




/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
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


M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

static M2MB_PDP_HANDLE pdpHandle = NULL;

static INT8 lwm2m_taskID;

/*===== ONEEDGE =====*/

/*Handles*/
static M2MB_LWM2M_HANDLE lwm2mHandle;
static M2MB_OS_EV_HANDLE eventsHandleLwm2m = NULL;

/*URI objects*/

static M2MB_LWM2M_OBJ_URI_T _obj_telit_service_uri          = { M2MB_LWM2M_URI_4_FIELDS,
    33211, 0,
    0,1};

/* Local function prototypes ====================================================================*/

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


/*!
 * @brief        Check existence of XML object description file in XML directory
 *
 * @param[in]    name    Name of the file to be checked
 * @retval               0 if OK, other values error
 *
 */
static int check_xml_file(const char* name);

static void convertUnixTimeToRTC_TIME(time_t t, M2MB_RTC_TIME_T *time, INT16 tz);

/* Static functions =============================================================================*/

static INT32 lwm2m_taskCB( INT32 event, INT32 i_uri, INT32 param2)
{
  (void) param2;
  MEM_W data_buffer[256] = {0};
  INT32 data_int = 0;
  UINT32 data_time = 0;
  double data_float = 0;

  M2MB_RESULT_E retVal;
  M2MB_LWM2M_OBJ_URI_T *pUri = (M2MB_LWM2M_OBJ_URI_T *) i_uri;
  M2MB_LWM2M_OBJ_URI_T uri = *pUri;


  M2MB_LWM2M_DATA_TYPE_E  data_type = M2MB_LWM2M_DATA_TYPE_INVALID;
  M2MB_OS_RESULT_E        osRes;

  UINT32                  curEvBits;

  m2mb_os_free(pUri);

  uri.uriLen = M2MB_LWM2M_URI_4_FIELDS;
  switch(event)
  {
  case EV_MON_URC_RECEIVED:

    AZX_LOG_TRACE("Asking a read operation for {%u/%u/%u/%u (%u)}\r\n",
        uri.obj, uri.objInst, uri.resource, uri.resourceInst, uri.uriLen );


    memset(data_buffer,0,sizeof(data_buffer));
    retVal = m2mb_lwm2m_read( lwm2mHandle, &(uri), data_buffer, sizeof(data_buffer));

    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "LWM2M read request succeeded\r\n" );
    }
    else
    {
      AZX_LOG_ERROR("Read request failed\r\n");
      return retVal;
    }

    /*wait for event*/
    AZX_LOG_TRACE("Waiting LWM2M read complete (10 seconds)...\r\n");
    osRes = m2mb_os_ev_get(eventsHandleLwm2m, EV_LWM2M_READ_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS(10000));
    if(osRes != M2MB_OS_SUCCESS)
    {
      AZX_LOG_ERROR("LWM2M read timeout!\r\n");
      return M2MB_RESULT_FAIL;
    }

    if (uri.obj == M2MB_LWM2M_DEMO_OBJ_ID)
    {
      switch(uri.resource)
      {

      /* ==================
       *
       * STRING TYPES
       *
       * ==================*/
      case DEMO_STRING_R_RES_ID:
      case DEMO_STRING_RW_RES_ID:
      case DEMO_MULTI_STRING_R_RES_ID:
      case DEMO_MULTI_STRING_RW_RES_ID:
        /*fallback*/

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_STRING;
        AZX_LOG_INFO("\r\nString data in {%u/%u/%u/%u} resource was updated to new content: <%s>\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            data_buffer );

        break;



        /* ==================
         *
         * INTEGER TYPES
         *
         * ==================*/
      case DEMO_INT_R_RES_ID:
      case DEMO_INT_RW_RES_ID:
      case DEMO_MULTI_INT_R_RES_ID:
      case DEMO_MULTI_INT_RW_RES_ID:
        /*fallback*/

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_INTEGER;

        /*Cast the data buffer as an integers array and take the first element*/
        data_int = (INT32) ( (INT32 *)data_buffer )[0];

        AZX_LOG_INFO("\r\nInteger data in {%u/%u/%u/%u} resource was updated to new value: %d\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            data_int);

        break;

        /* ==================
         *
         * FLOAT TYPES
         *
         * ==================*/
      case DEMO_FLOAT_R_RES_ID:
      case DEMO_FLOAT_RW_RES_ID:
      case DEMO_MULTI_FLOAT_R_RES_ID:
      case DEMO_MULTI_FLOAT_RW_RES_ID:
        /*fallback*/

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_FLOAT;

        /*Cast the data buffer as an integers array and take the first element*/
        data_float = (double) ( (double *)data_buffer )[0];

        AZX_LOG_INFO("\r\nFloat data in {%u/%u/%u/%u} resource was updated to new value: %f\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            data_float );

        break;


        /* ==================
         *
         * BOOLEAN TYPES
         *
         * ==================*/
      case DEMO_BOOL_R_RES_ID:
      case DEMO_BOOL_RW_RES_ID:
      case DEMO_MULTI_BOOL_R_RES_ID:
      case DEMO_MULTI_BOOL_RW_RES_ID:
        /*fallback*/

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_BOOLEAN;

        /*Cast the data buffer as an integers array and take the first element*/
        data_int = !!((INT32) ( (INT32 *)data_buffer )[0]);

        AZX_LOG_INFO("\r\nBoolean data in {%u/%u/%u/%u} resource was updated to new value: %s\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            (data_int>0)? "true":"false");

        break;

        /* ==================
         *
         * TIME TYPES
         *
         * ==================*/
      case DEMO_TIME_R_RES_ID:
      case DEMO_TIME_RW_RES_ID:
      case DEMO_MULTI_TIME_R_RES_ID:
      case DEMO_MULTI_TIME_RW_RES_ID:
      {
        /*fallback*/
        M2MB_RTC_TIME_T time_struct;
        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_TIME;

        /*Cast the data buffer as an integers array and take the first element*/
        data_time = (INT32) ( (INT32 *)data_buffer )[0];

        convertUnixTimeToRTC_TIME((time_t) (data_time), &time_struct, 0);

        AZX_LOG_INFO("\r\nTime data in {%u/%u/%u/%u} resource was updated to new value: %d"
            "(%04d/%02d/%02dT%02d:%02d:%02d+00:00)\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            data_time,
            time_struct.year, time_struct.mon, time_struct.day,
            time_struct.hour, time_struct.min, time_struct.sec);
      }
      break;


      /* ==================
       *
       * OPAQUE TYPES
       *
       * ==================*/
      case DEMO_OPAQUE_R_RES_ID:
      case DEMO_OPAQUE_RW_RES_ID:
      case DEMO_MULTI_OPAQUE_R_RES_ID:
      case DEMO_MULTI_OPAQUE_RW_RES_ID:
      {
        /*fallback*/
        int i;

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_OPAQUE;

        AZX_LOG_INFO("\r\nOpaque data in {%u/%u/%u/%u} resource was updated to new content:\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst);

        for (i=0; i<(int)sizeof(data_buffer);i++)
        {
          AZX_LOG_INFO("%02X ", data_buffer[i]);
        }
        AZX_LOG_INFO("\r\n");
      }
      break;

      default:
        AZX_LOG_WARN("\r\nUnexpected resource URI {%u/%u/%u/%u (%u)}!\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst, uri.uriLen );
        break;
      } /* switch(uri.resource) */

      AZX_LOG_TRACE("Data type: %d\r\n", data_type);
    }
    else
    {
      AZX_LOG_WARN("Unexpected object ID %u\r\n", uri.obj);
    }

    break;

  default:
    AZX_LOG_WARN("Unmanaged task event %d\r\n", event);
    break;
  } /* switch(event) */

  return 1;
}


static void lwm2mIndicationCB( M2MB_LWM2M_HANDLE h, M2MB_LWM2M_EVENT_E event, UINT16 resp_size, void *resp_struct,
    void *userdata )
{
  ( void ) h;
  ( void ) resp_size;
  ( void ) userdata;

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
      m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_ENABLE_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "Enable result %d\r\n", resp->result );
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
      m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_SET_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "set result %d\r\n", resp->result );
    }
    break;
  }

  /* event in response to m2mb_lwm2m_read() */
  case M2MB_LWM2M_READ_RES:
  {
    M2MB_LWM2M_READ_RES_T *resp = ( M2MB_LWM2M_READ_RES_T * )resp_struct;
    AZX_LOG_TRACE( "M2MB_LWM2M_READ_RES, result: %d\r\n", resp->result );

    AZX_LOG_DEBUG("Read type: %d; size: %u\r\n",
        resp->resType,
        resp->len
    );

    if( resp->result == M2MB_LWM2M_RES_SUCCESS )
    {
      m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_READ_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_ERROR("READ failed with error %d\r\n", resp->result );
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
      m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_WRITE_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "Enable write %d\r\n", resp->result );
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
      m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_SRV_REG_BIT, M2MB_OS_EV_SET);
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
      if (execUri.obj == M2MB_LWM2M_DEMO_OBJ_ID)
      {
        switch(execUri.resource)
        {
        case DEMO_EXEC1_RES_ID:
          AZX_LOG_INFO("Asked to execute resource %d\r\n", DEMO_EXEC1_RES_ID);
          break;

        case DEMO_EXEC2_RES_ID:
          AZX_LOG_INFO("Asked to execute resource %d\r\n", DEMO_EXEC2_RES_ID);
          break;
        default:
          AZX_LOG_WARN("\r\nUnexpected exec resource URI {%u/%u/%u/%u (%u)}!\r\n",
              execUri.obj, execUri.objInst, execUri.resource, execUri.resourceInst, execUri.uriLen );
          break;
        } /* switch(execUri.resource) */

      }
      else
      {
        AZX_LOG_WARN("Unexpected object ID %u\r\n", execUri.obj);
      }
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
      AZX_LOG_DEBUG("Monitoring enabled.\r\n\r\n");
      m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_MON_RES_BIT, M2MB_OS_EV_SET);
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
      AZX_LOG_INFO("GET STATUS.\r\nIF Status: %s\r\nClient Status: %s\r\n",
          IF_STATUS_STRING[resp->status], CL_STATUS_STRING[resp->clStatus]);
      m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_GET_STAT_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  default:
    AZX_LOG_DEBUG( "LWM2M EVENT %d\r\n", event );
    break;
  }
}


static int check_xml_file(const char* name)
{
  char path[64] = {0};
  struct M2MB_STAT info;
  sprintf(path, "/XML/%s", name);

  AZX_LOG_TRACE("Looking for <%s> file..\r\n", path);

  if(0 == m2mb_fs_stat(path, &info))
  {
    AZX_LOG_TRACE("File is present.\r\n");
    return 0;
  }
  else
  {
    INT32 last_errno = m2mb_fs_get_errno_value();

    if(last_errno == M2MB_FS_ENOENT)
    {
      AZX_LOG_WARN("File %s not found.\r\n", path);
      return -1;
    }
    else
    {
      AZX_LOG_ERROR("error with m2mb_fs_stat, errno is: %d\r\n", last_errno);
      return -2;
    }
  }
}

//strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now)
static void convertUnixTimeToRTC_TIME(time_t t, M2MB_RTC_TIME_T *p_rtc_time, INT16 tz)
{
  struct tm *_utctime = NULL;

  if(!p_rtc_time)
  {
    return;
  }

  memset(p_rtc_time,0,sizeof(M2MB_RTC_TIME_T));
  _utctime = gmtime((const time_t *)&t);

  p_rtc_time->year  = _utctime->tm_year + 1900;
  p_rtc_time->mon   = _utctime->tm_mon + 1;
  p_rtc_time->day   = _utctime->tm_mday;
  p_rtc_time->hour  = _utctime->tm_hour;
  p_rtc_time->min   = _utctime->tm_min;
  p_rtc_time->sec   = _utctime->tm_sec;
  p_rtc_time->tz    = tz;
}



/* Global functions =============================================================================*/
INT32 set_read_only_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, int value)
{
  UINT32                  curEvBits;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nSetting integer resource {%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nSetting integer resource {%u/%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }

  M2MB_RESULT_E retVal = m2mb_lwm2m_set( h, pUri, &value, sizeof(int));

  //retVal is just the return value of the API, not the completed operation
  //the completed operation ends when an event is raised into the callback
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_set returned error %d\r\n",retVal );
    return -1;
  }

  if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
      eventsHandleLwm2m,
      EV_LWM2M_SET_RES_BIT,
      M2MB_OS_EV_GET_ANY_AND_CLEAR,
      &curEvBits,
      M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
  )
  )
  {
    AZX_LOG_ERROR("LWM2M set timeout!\r\n");
    return -2;
  }
  else
  {
    return 0;
  }

}

INT32 write_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *data)
{
  UINT32                  curEvBits;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nWriting string resource {%u/%u/%u} value to %s on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        data);
  }
  else
  {
    AZX_LOG_INFO("\r\nWriting integer resource {%u/%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        data);
  }

  M2MB_RESULT_E retVal = m2mb_lwm2m_write( h, pUri, data, strlen(data));

  //retVal is just the return value of the API, not the completed operation
  //the completed operation ends when an event is raised into the callback
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_write returned error %d\r\n",retVal );
    return -1;
  }

  if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
      eventsHandleLwm2m,
      EV_LWM2M_WRITE_RES_BIT,
      M2MB_OS_EV_GET_ANY_AND_CLEAR,
      &curEvBits,
      M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
  )
  )
  {
    AZX_LOG_ERROR("LWM2M write timeout!\r\n");
    return -2;
  }
  else
  {
    return 0;
  }

}


M2MB_RESULT_E read_integer_from_uri(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 *result)
{
  int data[2];
  M2MB_RESULT_E retVal;

  M2MB_OS_RESULT_E osRes;
  UINT32 curEvBits;


  AZX_LOG_TRACE("m2mb_lwm2m_read for integer from {%d/%d/%d}\r\n",
      pUri->obj, pUri->objInst, pUri->resource );

  retVal = m2mb_lwm2m_read( h, pUri, data, sizeof(data));
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_TRACE( "m2mb_lwm2m_read request succeeded\r\n" );
  }

  /*wait for event*/
  AZX_LOG_TRACE("Waiting LWM2M read complete (10 seconds)...\r\n");
  osRes = m2mb_os_ev_get(eventsHandleLwm2m, EV_LWM2M_READ_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS(10000));
  if(osRes != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("LWM2M read timeout!\r\n");
    return M2MB_RESULT_FAIL;
  }

  *result = data[0];
  AZX_LOG_TRACE("Received data <%d> from portal\r\n", *result);
  return M2MB_RESULT_SUCCESS;
}

M2MB_RESULT_E read_double_from_uri(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double *result)
{
  double data[2];
  M2MB_RESULT_E retVal;

  M2MB_OS_RESULT_E osRes;
  UINT32 curEvBits;


  AZX_LOG_TRACE("m2mb_lwm2m_read for double from {%d/%d/%d}\r\n",
      pUri->obj, pUri->objInst, pUri->resource );

  retVal = m2mb_lwm2m_read( h, pUri, data, sizeof(data));
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_TRACE( "m2mb_lwm2m_read request succeeded\r\n" );
  }

  /*wait for event*/
  AZX_LOG_TRACE("Waiting LWM2M read complete (10 seconds)...\r\n");
  osRes = m2mb_os_ev_get(eventsHandleLwm2m, EV_LWM2M_READ_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS(10000));
  if(osRes != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("LWM2M read timeout!\r\n");
    return M2MB_RESULT_FAIL;
  }

  *result = data[0];
  AZX_LOG_TRACE("Received data <%f> from portal\r\n", *result);
  return M2MB_RESULT_SUCCESS;
}



void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
  (void)resp_size;
  (void)myUserdata;

  M2MB_NET_REG_STATUS_T *stat_info;

  switch (net_event)
  {



  case M2MB_NET_GET_REG_STATUS_INFO_RESP:
    stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
    if  (stat_info->stat == 1 || stat_info->stat == 5)
    {
      AZX_LOG_TRACE("Module is registered to cell 0x%X!\r\n", stat_info->cellID);
      m2mb_os_ev_set(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      m2mb_net_get_reg_status_info(h); //try again
    }
    break;


  default:
    AZX_LOG_DEBUG("unexpected net_event: %d\r\n", net_event);
    break;

  }
}

void PdpCallback(M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata)
{
  (void)userdata;
  struct M2MB_SOCKET_BSD_SOCKADDR_IN CBtmpAddress;
  CHAR CBtmpIPaddr[32];

  switch (pdp_event)
  {
  case M2MB_PDP_UP:
    AZX_LOG_DEBUG ("Context activated!\r\n");
    m2mb_pdp_get_my_ip(h, cid, M2MB_PDP_IPV4, &CBtmpAddress.sin_addr.s_addr);
    m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET, &CBtmpAddress.sin_addr.s_addr, ( CHAR * )&( CBtmpIPaddr ), sizeof( CBtmpIPaddr ) );
    AZX_LOG_TRACE( "IP address: %s\r\n", CBtmpIPaddr);
    m2mb_os_ev_set(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_SET);
    break;

  case M2MB_PDP_DOWN:
    AZX_LOG_TRACE ("Context deactivated!\r\n");
    m2mb_os_ev_set(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_CLEAR);
    break;
  default:
    AZX_LOG_DEBUG("unexpected pdp_event: %d\r\n", pdp_event);
    break;

  }
}

UINT8 oneedge_init( void)
{
  M2MB_RESULT_E retVal;
  M2MB_LWM2M_ENABLE_REQ_T pars;

  INT32 service_enable = 1;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;


  if(0 != check_xml_file(OBJECT_XML_NAME))
  {
    AZX_LOG_CRITICAL("%s file is not present in XML folder!\r\n", OBJECT_XML_NAME);
    return -1;
  }


  /* Init events handler */
  osRes = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "lwm2m_ev"));
  osRes = m2mb_os_ev_init( &eventsHandleLwm2m, &evAttrHandle );

  if ( osRes != M2MB_OS_SUCCESS )
  {
    m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");

  }
  else
  {
    AZX_LOG_TRACE("m2mb_os_ev_init success\r\n");
  }


  //get the handle of the lwm2m client on _h
  retVal = m2mb_lwm2m_init( &lwm2mHandle, lwm2mIndicationCB, ( void * )NULL );

  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_init returned error %d\r\n", retVal );
    m2mb_os_ev_deinit( eventsHandleLwm2m );
    return -1;
  }

  retVal = m2mb_lwm2m_write( lwm2mHandle, &_obj_telit_service_uri, &service_enable, sizeof( INT32 ) );
  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_write returned error %d\r\n", retVal );

    m2mb_os_ev_deinit( eventsHandleLwm2m );

    m2mb_lwm2m_deinit( lwm2mHandle );
    return -1;
  }

  lwm2m_taskID = azx_tasks_createTask((char*) "LWM2M_TASK", AZX_TASKS_STACK_M, 4, AZX_TASKS_MBOX_S, lwm2m_taskCB);

  AZX_LOG_TRACE("Task ID: %d.\r\n", lwm2m_taskID);
  if(lwm2m_taskID <= 0)
  {
    AZX_LOG_ERROR("Cannot create lwm2m managing task!\r\n");
    m2mb_os_ev_deinit( eventsHandleLwm2m );

    m2mb_lwm2m_deinit( lwm2mHandle );
    return -1;
  }

  pars.apnclass = CTX_ID; /*CID*/
  pars.guardRequestEventSecs = 5;
  pars.guardReleaseEventSecs = 5;
  pars.commandType = M2MB_LWM2MENA_CMD_TYPE_SET;
  pars.mode = M2MB_LWM2M_MODE_NO_ACK;

  retVal = m2mb_lwm2m_enable( lwm2mHandle, &pars );
  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_enable returned error %d\r\n", retVal );
    m2mb_os_ev_deinit( eventsHandleLwm2m );
    m2mb_lwm2m_deinit( lwm2mHandle );
    return -1;
  }
  if(M2MB_OS_SUCCESS != m2mb_os_ev_get(
      eventsHandleLwm2m,
      EV_LWM2M_ENABLE_RES_BIT,
      M2MB_OS_EV_GET_ANY_AND_CLEAR,
      &curEvBits,
      M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
  )
  )
  {
    AZX_LOG_ERROR("m2mb_lwm2m_enable timeout!\r\n");
    m2mb_os_ev_deinit( eventsHandleLwm2m );

    azx_sleep_ms(2000);
    m2mb_lwm2m_deinit(lwm2mHandle);
    return 1;
  }



  azx_sleep_ms(1000);




  /*Create an instance of the demo's custom object*/
  {
    M2MB_LWM2M_NEW_INST_REQ_T new_inst_params;
    M2MB_LWM2M_OBJ_URI_T uri_new_inst = { M2MB_LWM2M_URI_3_FIELDS, M2MB_LWM2M_DEMO_OBJ_ID, 0, 0, 0};

    new_inst_params.agent = 0; /*Telit Agent*/

    /*If OK the instance was not present, and so it was created. If an error
       is received in the callback, it is likely because the instance already exists. */
    retVal = m2mb_lwm2m_newinst( lwm2mHandle, &uri_new_inst, &new_inst_params );
    if( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_lwm2m_newinst returned error %d\r\n", retVal );
      m2mb_os_ev_deinit( eventsHandleLwm2m );
      m2mb_lwm2m_deinit( lwm2mHandle );
      return -1;
    }
  }


  /*Register a monitor on a resource by creating a URI object and passing it to m2mb_lwm2m_mon*/
  {
    M2MB_LWM2M_MON_REQ_T mon;

    M2MB_LWM2M_OBJ_URI_T uri_mon = {
        M2MB_LWM2M_URI_4_FIELDS,
        M2MB_LWM2M_DEMO_OBJ_ID, 0,
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
        eventsHandleLwm2m,
        EV_LWM2M_MON_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_mon timeout!\r\n");
      m2mb_os_ev_deinit( eventsHandleLwm2m );

      m2mb_lwm2m_disable(lwm2mHandle);
      azx_sleep_ms(2000);
      m2mb_lwm2m_deinit(lwm2mHandle);
      return 1;
    }
  }

  AZX_LOG_INFO("Waiting LWM2M Registering (120 seconds timeout)...\r\n");
  osRes = m2mb_os_ev_get(eventsHandleLwm2m, EV_LWM2M_SRV_REG_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS(120000));
  if(osRes != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("LWM2M Register timeout!\r\n");
    m2mb_os_ev_deinit( eventsHandleLwm2m );

    m2mb_lwm2m_disable(lwm2mHandle);
    azx_sleep_ms(2000);
    m2mb_lwm2m_deinit(lwm2mHandle);
    return 1;
  }

  return 0;
}


INT32 msgLWM2MTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;

  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle = NULL;
  UINT32                  curEvBits;

  M2MB_LWM2M_OBJ_URI_T resource_uri;

  M2MB_NET_HANDLE netHandle;
  INT32 ret;

  int task_status = type;

  void *myUserdata = NULL;

  do
  {
    AZX_LOG_INFO("Initializing resources...\r\n");

    /* Init events handler */
    osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "net_pdp_ev"));
    osRes = m2mb_os_ev_init( &net_pdp_evHandle, &evAttrHandle );

    if ( osRes != M2MB_OS_SUCCESS )
    {
      m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
      AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }
    else
    {
      AZX_LOG_TRACE("m2mb_os_ev_init success\r\n");
    }

    /* check network registration and configure PDP context */
    retVal = m2mb_net_init(&netHandle, NetCallback, myUserdata);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "m2mb_net_init returned M2MB_RESULT_SUCCESS\r\n");
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_net_init did not return M2MB_RESULT_SUCCESS\r\n" );
      task_status = APPLICATION_EXIT;
      break;
    }


    AZX_LOG_TRACE("Waiting for registration...\r\n");

    retVal = m2mb_net_get_reg_status_info(netHandle);
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
      task_status = APPLICATION_EXIT;
      break;
    }

    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);



    AZX_LOG_TRACE("PDP context initialization\r\n");
    retVal = m2mb_pdp_init(&pdpHandle, PdpCallback, myUserdata);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "m2mb_pdp_init returned M2MB_RESULT_SUCCESS\r\n");
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
      task_status = APPLICATION_EXIT;
      break;
    }


    /*
     * Just set the APN, as LwM2M agent will automatically manage the connection.
     * !! This could require a reboot if LTE networks are in use !!
     *
     * */
    retVal = m2mb_pdp_APN_set( pdpHandle, 1 /*LWM2M uses CID 1 by default*/, (char*)APN );
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "m2mb_pdp_APN_set returned M2MB_RESULT_SUCCESS\r\n");
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_pdp_APN_set did not return M2MB_RESULT_SUCCESS\r\n" );
      task_status = APPLICATION_EXIT;
      break;
    }

    m2mb_pdp_deinit(pdpHandle);



    azx_sleep_ms(8000);

    /*Initialize LWM2M*/
    ret = oneedge_init();
    if (ret != 0)
    {
      task_status = APPLICATION_EXIT;
      break;
    }


    AZX_LOG_INFO("\r\nWaiting for events from portal. Write on monitored resource or call an exec\r\n\r\n");


    /*Checking client status*/
    m2mb_lwm2m_get_stat(lwm2mHandle);

    if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
        eventsHandleLwm2m,
        EV_LWM2M_GET_STAT_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(2000) /*wait 2 seconds for the event to occur*/
    ))
    {
      AZX_LOG_ERROR("LWM2M get status timeout!\r\n");
      return -2;
    }


    /* ==================================================
     * Sending a read only resource with a SET operation
     * ==================================================*/

    /* Fill resource URI with required parameters (single instance read only integer, in this case)*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_INT_R_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    set_read_only_integer_resource(lwm2mHandle, &resource_uri, 50);


    /* ==================================================
     * Sending a read only multi-instance resource with a SET operation
     * Filling resource URI with required parameters (multi instance read only integer, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_MULTI_INT_R_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_4_FIELDS;

    set_read_only_integer_resource(lwm2mHandle, &resource_uri, 10);


    /* ==================================================
     * Sending a read only multi-instance  resource with a SET operation
     * Use the same multi instance read only integer, but set instance 1 this time
     * ==================================================*/
    resource_uri.resourceInst = 1;

    set_read_only_integer_resource(lwm2mHandle, &resource_uri, 11);




    /* ==================================================
     * Sending a string resource with a WRITE operation
     * ==================================================*/

    /* Fill resource URI with required parameters (single instance read only integer, in this case)*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_STRING_RW_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;
    write_string_resource(lwm2mHandle, &resource_uri, (CHAR *)"demo_string");


    while(1)
    {
      /*Wait*/
      azx_sleep_ms(1000);

    }
    task_status = APPLICATION_EXIT;
  } while(0);

  if (task_status == APPLICATION_EXIT)
  {
    if(pdpHandle)
    {
      m2mb_pdp_deinit(pdpHandle);
    }

    if(netHandle)
    {
      m2mb_net_deinit(netHandle);
    }

    if(net_pdp_evHandle)
    {
      m2mb_os_ev_deinit(net_pdp_evHandle);
    }

    AZX_LOG_DEBUG("Application complete.\r\n");
  }

  return 0;
}

