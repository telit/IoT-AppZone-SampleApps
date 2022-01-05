/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    lwm2m_demo.c

  @brief
    The file contains the LWM2M utilities

  @details

  @version
    1.0.4
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


#include "lwm2m_demo.h"
#include "lwm2m_utils.h"


/* Local defines ================================================================================*/
#define CTX_ID      1 /*PDP context ID*/

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

static INT8 lwm2m_taskID;

/*===== ONEEDGE =====*/

/*Handles*/
static M2MB_LWM2M_HANDLE lwm2mHandle;

/*URI objects*/

static M2MB_LWM2M_OBJ_URI_T _obj_telit_service_uri          = { M2MB_LWM2M_URI_4_FIELDS,
    33211, 0, 0, 1};

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

/*-----------------------------------------------------------------------------------------------*/
/*!
 * @brief        Task callback function. It is called when a data event arrives in the LwM2M CB
 *
 * @param[in]    event    The incoming event. EV_MON_URC_RECEIVED is supported for now
 * @param[in]    i_uri    uri structure carrying the object and resource ids
 * @param[in]    param2   unused
 * @retval               0 if OK, other values error
 *
 */
static INT32 lwm2m_taskCB( INT32 event, INT32 i_uri, INT32 param2)
{
  (void) param2;
  UINT8 opaque_buffer[1024] = {0};
  CHAR string_buffer[256] = {0};

  INT32 data_int = 0;
  BOOLEAN data_bool = FALSE;
  UINT64 data_time = 0;
  double data_float = 0;


  INT32 retVal;
  M2MB_LWM2M_OBJ_URI_T *pUri = (M2MB_LWM2M_OBJ_URI_T *) i_uri;
  M2MB_LWM2M_OBJ_URI_T uri = *pUri;



  M2MB_LWM2M_DATA_TYPE_E  data_type = M2MB_LWM2M_DATA_TYPE_INVALID;

  m2mb_os_free(pUri);

  uri.uriLen = M2MB_LWM2M_URI_4_FIELDS;
  switch(event)
  {
    case EV_MON_URC_RECEIVED:

    AZX_LOG_TRACE("Asking a read operation for {%u/%u/%u/%u (%u)}\r\n",
        uri.obj, uri.objInst, uri.resource, uri.resourceInst, uri.uriLen );

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

        memset(string_buffer,0,sizeof(string_buffer));
        retVal = read_rw_string_resource(lwm2mHandle, &(uri),
		         string_buffer, (UINT16)sizeof(string_buffer));
        if(retVal != 0)
        {
          return retVal;
        }

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_STRING;
        AZX_LOG_INFO("\r\nString data in {%u/%u/%u/%u} resource was updated to new content: <%s>\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            string_buffer );
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

        retVal = read_rw_integer_resource(lwm2mHandle, &(uri), &data_int);
        if(retVal != 0)
        {
          return retVal;
        }
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

        retVal = read_rw_double_resource(lwm2mHandle, &(uri), &data_float);
        if(retVal != 0)
        {
          return retVal;
        }


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

        retVal = read_rw_boolean_resource(lwm2mHandle, &(uri), &data_bool);
        if(retVal != 0)
        {
          return retVal;
        }

        AZX_LOG_INFO("\r\nBoolean data in {%u/%u/%u/%u} resource was updated to new value: %s\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            (data_bool)? "true":"false");

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


        retVal = read_rw_timestamp_resource(lwm2mHandle, &(uri), &data_time);
        if(retVal != 0)
        {
          return retVal;
        }

        convertUnixTimeToRTC_TIME((time_t)(data_time), &time_struct, 0);

        AZX_LOG_INFO("\r\nTime data in {%u/%u/%u/%u} resource was updated to new value: %llu"
            " (%04d/%02d/%02dT%02d:%02d:%02d+00:00)\r\n",
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
        UINT16 data_size = (UINT16)sizeof(opaque_buffer);
        memset(opaque_buffer,0,sizeof(opaque_buffer));
        retVal = read_rw_opaque_resource(lwm2mHandle, &(uri), opaque_buffer, &data_size);
        if(retVal != 0)
        {
          return retVal;
        }
        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_OPAQUE;

        AZX_LOG_INFO("\r\nOpaque data in {%u/%u/%u/%u} resource was updated to new content (%u bytes):\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst, data_size);

        for (i=0; i < data_size; i++)
        {
          AZX_LOG_INFO("%02X ", opaque_buffer[i]);
        }
        AZX_LOG_INFO("\r\n");
      }
      break;



      /* *************************
       *
       * WRITE ONLY TYPES
       *
       * *************************/

      case DEMO_STRING_W_RES_ID:
      case DEMO_MULTI_STRING_W_RES_ID:
        /*fallback*/

        data_type = M2MB_LWM2M_DATA_TYPE_STRING;

        memset(string_buffer,0,sizeof(string_buffer));
        retVal = get_write_only_string_resource(lwm2mHandle, &(uri), string_buffer, (UINT16)sizeof(string_buffer));
        if(retVal != 0)
        {
          return retVal;
        }

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_STRING;
        AZX_LOG_INFO("\r\nString data in {%u/%u/%u/%u} resource was updated to new content: <%s>\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            string_buffer );
        break;

      case DEMO_INT_W_RES_ID:
      case DEMO_MULTI_INT_W_RES_ID:
        /*fallback*/

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_INTEGER;

        retVal = get_write_only_integer_resource(lwm2mHandle, &(uri), &data_int);
        if(retVal != 0)
        {
          return retVal;
        }
        AZX_LOG_INFO("\r\nInteger data in {%u/%u/%u/%u} resource was updated to new value: %d\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            data_int);
        break;

      case DEMO_FLOAT_W_RES_ID:
      case DEMO_MULTI_FLOAT_W_RES_ID:
        /*fallback*/

        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_FLOAT;

        retVal = get_write_only_double_resource(lwm2mHandle, &(uri), &data_float);
        if(retVal != 0)
        {
          return retVal;
        }


        AZX_LOG_INFO("\r\nFloat data in {%u/%u/%u/%u} resource was updated to new value: %f\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            data_float );
        break;

      case DEMO_BOOL_W_RES_ID:
      case DEMO_MULTI_BOOL_W_RES_ID:
        /*fallback*/

        retVal = get_write_only_boolean_resource(lwm2mHandle, &(uri), &data_bool);
        if(retVal != 0)
        {
          return retVal;
        }

        AZX_LOG_INFO("\r\nBoolean data in {%u/%u/%u/%u} resource was updated to new value: %s\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            (data_bool)? "true":"false");
        break;

      case DEMO_TIME_W_RES_ID:
      case DEMO_MULTI_TIME_W_RES_ID:
        /*fallback*/
        M2MB_RTC_TIME_T time_struct;
        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_TIME;


        retVal = get_write_only_timestamp_resource(lwm2mHandle, &(uri), &data_time);
        if(retVal != 0)
        {
          return retVal;
        }

        convertUnixTimeToRTC_TIME((time_t)(data_time), &time_struct, 0);

        AZX_LOG_INFO("\r\nTime data in {%u/%u/%u/%u} resource was updated to new value: %llu"
            " (%04d/%02d/%02dT%02d:%02d:%02d+00:00)\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst,
            data_time,
            time_struct.year, time_struct.mon, time_struct.day,
            time_struct.hour, time_struct.min, time_struct.sec);
        break;

      case DEMO_OPAQUE_W_RES_ID:
      case DEMO_MULTI_OPAQUE_W_RES_ID:
      {
        /*fallback*/
        int i;
        UINT16 data_size = (UINT16)sizeof(opaque_buffer);
        memset(opaque_buffer,0,data_size);
        retVal = get_write_only_opaque_resource(lwm2mHandle, &(uri), opaque_buffer, &data_size);
        if(retVal != 0)
        {
          return retVal;
        }
        /*Store data_type for further management*/
        data_type = M2MB_LWM2M_DATA_TYPE_OPAQUE;

        AZX_LOG_INFO("\r\nOpaque data in {%u/%u/%u/%u} resource was updated to new content (%u bytes):\r\n",
            uri.obj, uri.objInst, uri.resource, uri.resourceInst, data_size);

        for (i=0; i < data_size; i++)
        {
          AZX_LOG_INFO("%02X ", opaque_buffer[i]);
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
      if (execUri.obj == M2MB_LWM2M_DEMO_OBJ_ID)
      {
        switch(execUri.resource)
        {
        case DEMO_EXEC1_RES_ID:
          AZX_LOG_INFO("Asked to execute resource %d\r\n", DEMO_EXEC1_RES_ID);
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
      AZX_LOG_INFO("GET STATUS.\r\nIF Status: %s\r\nClient Status: %s\r\n",
          IF_STATUS_STRING[resp->status], CL_STATUS_STRING[resp->clStatus]);
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_GET_STAT_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }
  default:
    AZX_LOG_DEBUG( "LWM2M EVENT %d\r\n", event );
    break;
  }
}

/*-----------------------------------------------------------------------------------------------*/
/*!
 * @brief        Checks if the input  XML file is present in the device filesystem
 *
 * @param[in]    name         The file name (without path)
 * @retval       0 in case of success, negative value otherwise
 *
 */
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

/*-----------------------------------------------------------------------------------------------*/
/*!
 * @brief        used to convert a unix timestamp into a RTC structure
 *
 * @param[in]    t            The input timestamp
 * @param[in]    p_rtc_time   pointer to the structure to be filled
 * @param[in]    tz           timezone to be used
 * @retval                    none
 *
 */
static void convertUnixTimeToRTC_TIME(time_t t, M2MB_RTC_TIME_T *p_rtc_time, INT16 tz)
{
  struct tm *_utctime = NULL;

  if(!p_rtc_time)
  {
    return;
  }

  memset(p_rtc_time,0,sizeof(M2MB_RTC_TIME_T));
  _utctime = gmtime((const time_t *)&t);
  if(_utctime)
  {
    p_rtc_time->year  = _utctime->tm_year + 1900;
    p_rtc_time->mon   = _utctime->tm_mon + 1;
    p_rtc_time->day   = _utctime->tm_mday;
    p_rtc_time->hour  = _utctime->tm_hour;
    p_rtc_time->min   = _utctime->tm_min;
    p_rtc_time->sec   = _utctime->tm_sec;
    p_rtc_time->tz    = tz;
  }
}


/* Global functions =============================================================================*/

UINT8 oneedge_init( void)
{
  M2MB_RESULT_E retVal;
  M2MB_LWM2M_ENABLE_REQ_T pars;

  INT32 service_enable = 1;
  M2MB_LWM2M_OBJ_URI_T exist_uri;

  M2MB_OS_RESULT_E        osRes;
  UINT32                  curEvBits;
  UINT16 agentID = 0;     /* Telit agent ID */

  memset(get_event_res_p(),0, sizeof(LWM2M_EVENT_RES_S));

  if(0 != check_xml_file(OBJECT_XML_NAME))
  {
    AZX_LOG_CRITICAL("%s file is not present in XML folder!\r\n", OBJECT_XML_NAME);
    return 1;
  }

  //get the handle of the lwm2m client on _h
  retVal = m2mb_lwm2m_init( &lwm2mHandle, lwm2mIndicationCB, ( void * )get_event_res_p() );

  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_init returned error %d\r\n", retVal );

    return 1;
  }

  m2mb_lwm2m_agent_config( lwm2mHandle, agentID );

  retVal = m2mb_lwm2m_write( lwm2mHandle, &_obj_telit_service_uri, &service_enable, sizeof( INT32 ) );
  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_write returned error %d\r\n", retVal );

    m2mb_lwm2m_deinit( lwm2mHandle );
    return 1;
  }

  lwm2m_taskID = azx_tasks_createTask((char*) "LWM2M_TASK", AZX_TASKS_STACK_M, 4, AZX_TASKS_MBOX_S, lwm2m_taskCB);

  AZX_LOG_TRACE("Task ID: %d.\r\n", lwm2m_taskID);
  if(lwm2m_taskID <= 0)
  {
    AZX_LOG_ERROR("Cannot create lwm2m managing task!\r\n");

    m2mb_lwm2m_deinit( lwm2mHandle );
    return 1;
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
    m2mb_lwm2m_deinit( lwm2mHandle );
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
    return 1;
  }

  azx_sleep_ms(1000);

  m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_CLEAR);

  /* query of the custom object URI for agent 0 (Telit)*/

  exist_uri.uriLen       = M2MB_LWM2M_URI_3_FIELDS;
  exist_uri.obj          = M2MB_LWM2M_DEMO_OBJ_ID;
  exist_uri.objInst      = 0;
  exist_uri.resource     = DEMO_STRING_R_RES_ID;
  exist_uri.resourceInst = 0;

  retVal = m2mb_lwm2m_exist( lwm2mHandle, &exist_uri );
  if( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_exist returned error %d\r\n", retVal );
    m2mb_lwm2m_deinit( lwm2mHandle );
    return 1;
  }

  osRes = m2mb_os_ev_get( get_lwm2mEvents_handle(),
                          EV_LWM2M_EXIST_RES_BIT | EV_LWM2M_FAIL_RES_BIT,
                          M2MB_OS_EV_GET_ANY_AND_CLEAR,
                          &curEvBits,
                          M2MB_OS_MS2TICKS(5000));
  if(osRes != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("LWM2M exists timeout!\r\n");
    m2mb_lwm2m_disable(lwm2mHandle);
    azx_sleep_ms(2000);
    m2mb_lwm2m_deinit(lwm2mHandle);
    return 1;
  }

  if ((curEvBits & EV_LWM2M_FAIL_RES_BIT) == EV_LWM2M_FAIL_RES_BIT)
  {
    AZX_LOG_ERROR("Failure event arrived!\r\n");
    m2mb_lwm2m_disable(lwm2mHandle);
    azx_sleep_ms(2000);
    m2mb_lwm2m_deinit(lwm2mHandle);
    return 1;
  }
  else
  {
    /*len is 0 if the object does not exist, 1 otherwise*/
    if(get_event_res_p()->resp_len == 0)
    {
      /*Create an instance of the demo's custom object*/
      M2MB_LWM2M_NEW_INST_REQ_T new_inst_params;
      M2MB_LWM2M_OBJ_URI_T uri_new_inst = { M2MB_LWM2M_URI_3_FIELDS, M2MB_LWM2M_DEMO_OBJ_ID, 0, 0, 0};

      new_inst_params.agent = agentID; /*Telit Agent*/

      /*If OK the instance was not present, and so it was created. If an error
         is received in the callback, it is likely because the instance already exists. */
      retVal = m2mb_lwm2m_newinst( lwm2mHandle, &uri_new_inst, &new_inst_params );
      if( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR( "m2mb_lwm2m_newinst returned error %d\r\n", retVal );

        m2mb_lwm2m_deinit( lwm2mHandle );
        return 1;
      }
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
      azx_sleep_ms(2000);
      m2mb_lwm2m_deinit(lwm2mHandle);
      return 1;
    }
  }

  AZX_LOG_INFO("Waiting LWM2M Registering (120 seconds timeout)...\r\n");
  osRes = m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_SRV_REG_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS(120000));
  if(osRes != M2MB_OS_SUCCESS)
  {
    AZX_LOG_ERROR("LWM2M Register timeout!\r\n");


    m2mb_lwm2m_disable(lwm2mHandle);
    azx_sleep_ms(2000);
    m2mb_lwm2m_deinit(lwm2mHandle);
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
  M2MB_RESULT_E retVal;
  M2MB_LWM2M_OBJ_URI_T resource_uri = {0};

  INT32 ret = 0;
  M2MB_RTC_TIMEVAL_T currTime = {1234567890, 0}; /*dummy value as fallback*/
  INT32 rtcfd = -1;

  int task_status = type;


  do
  {
    AZX_LOG_INFO("Initializing resources...\r\n");

    init_sync();

    azx_sleep_ms(8000);

    /*Initialize LWM2M*/
    ret = oneedge_init();
    if (ret != 0)
    {
      task_status = APPLICATION_EXIT;
      break;
    }

    rtcfd = m2mb_rtc_open("/dev/rtc0",0);
    if (rtcfd != -1){
      AZX_LOG_TRACE( "RTC opened\r\n");
    } else {

      AZX_LOG_ERROR("Cannot open RTC!");
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

    AZX_LOG_INFO("\r\n================================\r\n"
        "READ-ONLY RESOURCES\r\n"
        "================================\r\n");


    /* ==================================================
     * Sending a read only integer resource with a SET operation
     * Filling resource URI with required parameters (single instance read only integer, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_INT_R_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    set_read_only_integer_resource(lwm2mHandle, &resource_uri, 50);
    {
      /*Try and read again the set value*/
      INT32 value = 0;
      if(read_rw_integer_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Integer value is now %d\r\n", value);
      }

    }

    azx_sleep_ms(5000);

    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");
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

    azx_sleep_ms(5000);

    /* ==================================================
     * Sending a read only multi-instance  resource with a SET operation
     * Use the same multi instance read only integer, but set instance 1 this time
     * ==================================================*/
    resource_uri.resourceInst = 1;

    set_read_only_integer_resource(lwm2mHandle, &resource_uri, 11);


    azx_sleep_ms(5000);

    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");
    /* ==================================================
     * Sending a read only float resource with a SET operation
     * Filling resource URI with required parameters (single instance read only double, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_FLOAT_R_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    set_read_only_double_resource(lwm2mHandle, &resource_uri, 20.5);

    {
      /*Try and read again the set value*/
      double value = 0;
      if(read_rw_double_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Double value is now %f\r\n", value);
      }

    }

    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a read only boolean resource with a SET operation
     * Filling resource URI with required parameters (single instance read only boolean, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_BOOL_R_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    set_read_only_boolean_resource(lwm2mHandle, &resource_uri, TRUE);

    {
      /*Try and read again the set value*/
      BOOLEAN value = 0;
      if(read_rw_boolean_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Boolean value is now %s\r\n", value? "true": "false");
      }

    }

    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");



    /* ==================================================
     * Sending a read only timestamp resource with a SET operation
     * Filling resource URI with required parameters (single instance read only timestamp, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_TIME_R_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    /*
     * Get time and date in timestamp and date/time format and print them
     */
    if(m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_TIMEVAL, &currTime) == 0)
    {
      AZX_LOG_INFO("\r\nCurrent time in seconds from the epoch: %u\r\n", currTime.sec);
    }
    else
    {
      AZX_LOG_ERROR("Cannot get current time from module RTC!");
      currTime.sec = 1583881200; /*dummy date*/
    }

    {
      UINT64 value =  (UINT64)currTime.sec;
      //m2mb_lwm2m_set(lwm2mHandle, &resource_uri, &value, sizeof(value));
      set_read_only_timestamp_resource(lwm2mHandle, &resource_uri, value);

      /*Try and read again the set value*/
      value = 0;
      if(read_rw_timestamp_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Timestamp value is now %llu\r\n", value);
      }
    }

    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");


    /* ==================================================
     * Sending a read only opaque resource with a SET operation
     * Filling resource URI with required parameters (single instance read only opaque, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_OPAQUE_R_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    {
      UINT8 buf[20] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
      UINT16 recvDatalen = sizeof(buf);
      set_read_only_opaque_resource(lwm2mHandle, &resource_uri, buf, 10);

      azx_sleep_ms(5000);
      /*Try and read again the set value*/
      memset(buf, 0, sizeof(buf));
      if(read_rw_opaque_resource(lwm2mHandle, &resource_uri, buf, &recvDatalen))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        int _index;
        AZX_LOG_INFO("\r\n---Opaque content is now (%u bytes): ", recvDatalen);
        for (_index = 0; _index < (int)recvDatalen; _index++)
        {
          AZX_LOG_INFO("0x%02X ", buf[_index]);
        }

        AZX_LOG_INFO("\r\n");
      }
    }

    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a read only string resource with a SET operation
     * Filling resource URI with required parameters (single instance read only string, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_STRING_R_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    {
      CHAR string[20] = {0};
      sprintf(string, "Hello World!");
      set_read_only_string_resource(lwm2mHandle, &resource_uri, string);

      /*Try and read again the set value*/
      memset(string, 0, sizeof(string));
      if(read_rw_string_resource(lwm2mHandle, &resource_uri, string, sizeof(string)))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---String content is now: <%s>\r\n", string);
      }
    }

    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /*------------------------ RW ------------------------------------*/


    AZX_LOG_INFO("\r\n================================\r\n"
        "READ-WRITE RESOURCES\r\n"
        "================================\r\n");

    /* ==================================================
     * Sending a r/w integer resource with a WRITE operation
     * Filling resource URI with required parameters (single instance rw integer, in this case)
     * ==================================================*/

    /* Fill resource URI with required parameters (single instance read only integer, in this case)*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_INT_RW_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    write_rw_integer_resource(lwm2mHandle, &resource_uri, 50);


    {
      /*Try and read again the set value*/
      INT32 value = 0;
      if(read_rw_integer_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Integer value is now %d\r\n", value);
      }

    }

    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a r/w float resource with a WRITE operation
     * Filling resource URI with required parameters (single instance rw float, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_FLOAT_RW_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    write_rw_double_resource(lwm2mHandle, &resource_uri, 20.5);

    {
      /*Try and read again the set value*/
      double value = 0;
      if(read_rw_double_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Double value is now %f\r\n", value);
      }

    }


    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");


    /* ==================================================
     * Sending a r/w boolean resource with a WRITE operation
     * Filling resource URI with required parameters (single instance rw boolean, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_BOOL_RW_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    write_rw_boolean_resource(lwm2mHandle, &resource_uri, TRUE);

    {
      /*Try and read again the set value*/
      BOOLEAN value = 0;
      if(read_rw_boolean_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Boolean value is now %s\r\n", value? "true": "false");
      }

    }



    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a r/w time resource with a WRITE operation
     * Filling resource URI with required parameters (single instance rw time, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_TIME_RW_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;


    /*
     * Get time and date in timestamp and date/time format and print them
     */
    if(m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_TIMEVAL, &currTime) == 0)
    {
      AZX_LOG_INFO("\r\nCurrent time in seconds from the epoch: %u\r\n", currTime.sec);
    }
    else
    {
      AZX_LOG_ERROR("Cannot get current time from module RTC!");
      currTime.sec = 1583881200; /*dummy date*/
    }

    {
      UINT64 value = currTime.sec;
      write_rw_timestamp_resource(lwm2mHandle, &resource_uri, value);


      /*Try and read again the set value*/
      if(read_rw_timestamp_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Timestamp value is now %llu\r\n", value);
      }
    }


    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a r/w opaque resource with a WRITE operation
     * Filling resource URI with required parameters (single instance rw opaque, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_OPAQUE_RW_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    {
      UINT8 buf[20] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
      UINT16 recvDatalen = sizeof(buf);
      write_rw_opaque_resource(lwm2mHandle, &resource_uri, buf, 10);
      azx_sleep_ms(5000);
      /*Try and read again the set value*/
      memset(buf, 0, sizeof(buf));
      if(read_rw_opaque_resource(lwm2mHandle, &resource_uri, buf, &recvDatalen))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        int _index;
        AZX_LOG_INFO("\r\n---Opaque content is now (%u bytes): ", recvDatalen);
        for (_index = 0; _index < (int)recvDatalen; _index++)
        {
          AZX_LOG_INFO("0x%02X ", buf[_index]);
        }

        AZX_LOG_INFO("\r\n");
      }
    }


    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a r/w string resource with a WRITE operation
     * Filling resource URI with required parameters (single instance rw string, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_STRING_RW_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;
    {
      CHAR string[20] = {0};
      sprintf(string, "Hello World!");
      write_rw_string_resource(lwm2mHandle, &resource_uri, string);

      /*Try and read again the set value*/
      memset(string, 0, sizeof(string));
      if(read_rw_string_resource(lwm2mHandle, &resource_uri, string, sizeof(string)))
      {
        AZX_LOG_ERROR("Failed reading resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---String content is now: <%s>\r\n", string);
      }
    }


    /*------------------------ Write-only ------------------------------------*/


    AZX_LOG_INFO("\r\n================================\r\n"
        "WRITE-ONLY RESOURCES\r\n"
        "================================\r\n");

    /* ==================================================
     * Sending a write-only integer resource with a WRITE operation
     * Filling resource URI with required parameters (single instance w integer, in this case)
     * ==================================================*/

    /* Fill resource URI with required parameters (single instance read only integer, in this case)*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_INT_W_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    write_rw_integer_resource(lwm2mHandle, &resource_uri, 50);


    {
      /*Try and read again the set value*/
      INT32 value = 0;
      if(get_write_only_integer_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed getting resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Integer value is now %d\r\n", value);
      }

    }

    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a write-only float resource with a WRITE operation
     * Filling resource URI with required parameters (single instance w float, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_FLOAT_W_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    write_rw_double_resource(lwm2mHandle, &resource_uri, 20.5);

    {
      /*Try and read again the set value*/
      double value = 0;
      if(get_write_only_double_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed getting resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Double value is now %f\r\n", value);
      }

    }


    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");


    /* ==================================================
     * Sending a write-only boolean resource with a WRITE operation
     * Filling resource URI with required parameters (single instance w boolean, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_BOOL_W_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    write_rw_boolean_resource(lwm2mHandle, &resource_uri, TRUE);

    {
      /*Try and read again the set value*/
      BOOLEAN value = 0;
      if(get_write_only_boolean_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed getting resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Boolean value is now %s\r\n", value? "true": "false");
      }

    }



    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a write-only time resource with a WRITE operation
     * Filling resource URI with required parameters (single instance w time, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_TIME_W_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;


    /*
     * Get time and date in timestamp and date/time format and print them
     */
    if(m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_TIMEVAL, &currTime) == 0)
    {
      AZX_LOG_INFO("\r\nCurrent time in seconds from the epoch: %u\r\n", currTime.sec);
    }
    else
    {
      AZX_LOG_ERROR("Cannot get current time from module RTC!");
      currTime.sec = 1583881200; /*dummy date*/
    }

    {
      UINT64 value = currTime.sec;
      write_rw_timestamp_resource(lwm2mHandle, &resource_uri, value);


      /*Try and read again the set value*/
      if(get_write_only_timestamp_resource(lwm2mHandle, &resource_uri, &value))
      {
        AZX_LOG_ERROR("Failed getting resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---Timestamp value is now %llu\r\n", value);
      }
    }


    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a write-only opaque resource with a WRITE operation
     * Filling resource URI with required parameters (single instance w opaque, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_OPAQUE_W_RES_ID;
    resource_uri.resourceInst = 0;                      /*set instance 0*/
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    {
      UINT8 buf[20] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
      UINT16 recvDatalen = sizeof(buf);
      write_rw_opaque_resource(lwm2mHandle, &resource_uri, buf, 10);
      azx_sleep_ms(5000);
      /*Try and read again the set value*/
      memset(buf, 0, sizeof(buf));
      if(get_write_only_opaque_resource(lwm2mHandle, &resource_uri, buf, &recvDatalen))
      {
        AZX_LOG_ERROR("Failed getting resource!\r\n");
      }
      else
      {
        int _index;
        AZX_LOG_INFO("\r\n---Opaque content is now (%u bytes): ", recvDatalen);
        for (_index = 0; _index < (int)recvDatalen; _index++)
        {
          AZX_LOG_INFO("0x%02X ", buf[_index]);
        }

        AZX_LOG_INFO("\r\n");
      }
    }


    azx_sleep_ms(5000);
    AZX_LOG_INFO("\r\n---------------------------------------------------\r\n");

    /* ==================================================
     * Sending a write-only string resource with a WRITE operation
     * Filling resource URI with required parameters (single instance w string, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_STRING_W_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;
    {
      CHAR string[20] = {0};
      sprintf(string, "Hello World!");
      write_rw_string_resource(lwm2mHandle, &resource_uri, string);

      /*Try and read again the set value*/
      memset(string, 0, sizeof(string));
      if(get_write_only_string_resource(lwm2mHandle, &resource_uri, string, sizeof(string)))
      {
        AZX_LOG_ERROR("Failed getting resource!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\n---String content is now: <%s>\r\n", string);
      }
    }

    azx_sleep_ms(5000);

    AZX_LOG_INFO("\r\n-----------------------------------"
        "\r\nResources operations examples done.\r\n"
        "-----------------------------------\r\n");


    AZX_LOG_INFO("\r\n-----------------------------------"
        "\r\nWill perform a SET with notify ACK enabled.\r\n"
        "-----------------------------------\r\n");

    /* The resource URI must match all observations set in the device profile for this device */
    resource_uri.uriLen       = M2MB_LWM2M_URI_3_FIELDS;
    resource_uri.obj          = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst      = 0;
    resource_uri.resource     = 0;
    resource_uri.resourceInst = 0;  // better set it to 0, even if M2MB_LWM2M_URI_3_FIELDS

    AZX_LOG_INFO("Enable notify ack\r\n");
    retVal = m2mb_lwm2m_nfy_ack_uri( lwm2mHandle, TRUE, &resource_uri );
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_lwm2m_nfy_ack_uri request failed!" );
      task_status = APPLICATION_EXIT;
      break;
    }
    if (M2MB_OS_SUCCESS
        != m2mb_os_ev_get(get_lwm2mEvents_handle(),
            EV_LWM2M_NFYADD_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
            M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
        ))
    {
      AZX_LOG_ERROR("m2mb_lwm2m_nfy_ack_uri timeout!\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }

    /* The resource URI must match all observations set in the device profile for this device */
    resource_uri.uriLen       = M2MB_LWM2M_URI_3_FIELDS;
    resource_uri.obj          = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst      = 0;
    resource_uri.resource     = DEMO_INT_R_RES_ID;
    resource_uri.resourceInst = 0;

    retVal = m2mb_lwm2m_nfy_ack_uri( lwm2mHandle, TRUE, &resource_uri );
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_lwm2m_nfy_ack_uri request failed!" );
      task_status = APPLICATION_EXIT;
      break;
    }
    if (M2MB_OS_SUCCESS
        != m2mb_os_ev_get(get_lwm2mEvents_handle(),
            EV_LWM2M_NFYADD_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
            M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
        ))
    {
      AZX_LOG_ERROR("m2mb_lwm2m_nfy_ack_uri timeout!\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }

    /* setting the status to ENABLED */
    retVal = m2mb_lwm2m_nfy_ack_status( lwm2mHandle, TRUE, TRUE );
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_lwm2m_nfy_ack_status request failed" );
      task_status = APPLICATION_EXIT;
      break;
    }
    if (M2MB_OS_SUCCESS
        != m2mb_os_ev_get(get_lwm2mEvents_handle(),
            EV_LWM2M_NFYSTAT_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
            M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
        ))
    {
      AZX_LOG_ERROR("m2mb_lwm2m_nfy_ack_status timeout!\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }

    azx_sleep_ms(2000);

    /* ==================================================
     * Sending a read only integer resource with a SET operation
     * Filling resource URI with required parameters (single instance read only integer, in this case)
     * ==================================================*/
    resource_uri.obj = M2MB_LWM2M_DEMO_OBJ_ID;
    resource_uri.objInst = 0;
    resource_uri.resource = DEMO_INT_R_RES_ID;
    resource_uri.resourceInst = 0;
    resource_uri.uriLen = M2MB_LWM2M_URI_3_FIELDS;

    /* Setting a different value on this resource so the notification will be sent to the server */
    set_read_only_integer_resource(lwm2mHandle, &resource_uri, 60);
    {

      /* wait notify*/
      if (M2MB_OS_SUCCESS != m2mb_os_ev_get( get_lwm2mEvents_handle(),
          EV_LWM2M_NFYACK_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
          M2MB_OS_MS2TICKS(30000) /*wait 30 seconds for the event to occur*/
      ))
      {
        AZX_LOG_ERROR("lwm2m notify ack timeout!\r\n");
      }
      else
      {
        AZX_LOG_INFO("\r\nACK received from server!\r\n");
          /*Try and read again the set value*/
        INT32 value = 0;
        if(read_rw_integer_resource(lwm2mHandle, &resource_uri, &value))
        {
          AZX_LOG_ERROR("Failed reading resource!\r\n");
        }
        else
        {
          AZX_LOG_INFO("\r\n---Integer value is now %d\r\n", value);
        }
      }
    }

    AZX_LOG_INFO("\r\nDisable notify ack\r\n");
    /* setting the status to DISABLED */
    retVal = m2mb_lwm2m_nfy_ack_status( lwm2mHandle, TRUE, FALSE );
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_lwm2m_nfy_ack_status request failed" );
      task_status = APPLICATION_EXIT;
      break;
    }
    if (M2MB_OS_SUCCESS
        != m2mb_os_ev_get(get_lwm2mEvents_handle(),
            EV_LWM2M_NFYSTAT_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
            M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
        ))
    {
      AZX_LOG_ERROR("m2mb_lwm2m_nfy_ack_status timeout!\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }

    AZX_LOG_INFO("Done.\r\n");


    AZX_LOG_INFO("\r\nWaiting for events from the OneEdge portal. Please write on monitored resources or call an 'exec' one.\r\n\r\n");

    while(1)
    {
      /*Wait*/
      azx_sleep_ms(10000);
    }
    task_status = APPLICATION_EXIT;
  } while(0);

  if (task_status == APPLICATION_EXIT)
  {
    if(lwm2mHandle)
    {
      m2mb_lwm2m_disable(lwm2mHandle);
      azx_sleep_ms(2000);
      m2mb_lwm2m_deinit(lwm2mHandle);
    }

    deinit_sync();
    if (rtcfd != -1)
    {
      m2mb_rtc_close(rtcfd);
    }
    AZX_LOG_DEBUG("Application complete.\r\n");
  }

  return 0;
}

