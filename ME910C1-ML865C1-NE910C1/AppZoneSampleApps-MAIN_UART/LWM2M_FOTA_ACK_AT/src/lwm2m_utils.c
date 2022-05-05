/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    lwm2m_utils.c

  @brief
    The file contains the LWM2M resources utilities

  @details

  @version
    1.0.0
  @note


  @author


  @date
    23/11/2021
 */
/* Include files ================================================================================*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_rtc.h"

#include "m2mb_lwm2m.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "lwm2m_demo.h"
#include "lwm2m_utils.h"

static M2MB_OS_SEM_HANDLE lwm2mCSSemHandle = NULL;
static M2MB_OS_EV_HANDLE eventsHandleLwm2m = NULL;
static LWM2M_EVENT_RES_S event_res;

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
/*-----------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------*/
M2MB_OS_SEM_HANDLE get_lwm2mCSsem_handle(void)
{
  return lwm2mCSSemHandle;
}
/*-----------------------------------------------------------------------------------------------*/
M2MB_OS_EV_HANDLE get_lwm2mEvents_handle(void)
{
  return eventsHandleLwm2m;
}

LWM2M_EVENT_RES_S *get_event_res_p(void)
{
  return &event_res;
}

/*-----------------------------------------------------------------------------------------------*/
INT32 init_sync(void)
{
  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  INT32 result = 0;


  M2MB_OS_SEM_ATTR_HANDLE semAttrHandle;

  if (NULL == lwm2mCSSemHandle)
  {
    m2mb_os_sem_setAttrItem(&semAttrHandle,
        CMDS_ARGS(M2MB_OS_SEM_SEL_CMD_CREATE_ATTR, NULL,
            M2MB_OS_SEM_SEL_CMD_COUNT, 1 /*CS*/,
            M2MB_OS_SEM_SEL_CMD_TYPE, M2MB_OS_SEM_GEN));

    osRes = m2mb_os_sem_init( &lwm2mCSSemHandle, &semAttrHandle );
    if ( osRes != M2MB_OS_SUCCESS )
    {
      m2mb_os_sem_setAttrItem( &semAttrHandle, M2MB_OS_SEM_SEL_CMD_DEL_ATTR, NULL );
      AZX_LOG_CRITICAL("m2mb_os_sem_init failed!\r\n");
      lwm2mCSSemHandle = NULL;
      result = -1;
    }
    else
    {
      AZX_LOG_TRACE("m2mb_os_sem_init success\r\n");
      result = 0;
    }
  }

  if (NULL == eventsHandleLwm2m)
  {
    /* Init events handler */
    m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "lwm2m_ev"));
    osRes = m2mb_os_ev_init( &eventsHandleLwm2m, &evAttrHandle );
    if ( osRes != M2MB_OS_SUCCESS )
    {
      m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
      eventsHandleLwm2m = NULL;
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

/*-----------------------------------------------------------------------------------------------*/
/*releases the synchronization resources*/
void deinit_sync(void)
{
  if(eventsHandleLwm2m)
  {
    m2mb_os_ev_deinit( eventsHandleLwm2m );
    eventsHandleLwm2m = NULL;
  }
  if(lwm2mCSSemHandle)
  {
    m2mb_os_sem_deinit( lwm2mCSSemHandle );
    lwm2mCSSemHandle = NULL;
  }
}

/*-----------------------------------------------------------------------------------------------*/

INT32 set_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 inbuflen  )
{
  UINT32                  curEvBits;
  INT32 res = -1;
  M2MB_RESULT_E retVal;
  /*Get critical section*/
  m2mb_os_sem_get(lwm2mCSSemHandle, M2MB_OS_WAIT_FOREVER);

  m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_CLEAR);

  retVal = m2mb_lwm2m_set( h, uri, inbuf, inbuflen);

  //retVal is just the return value of the API, not the completed operation
  //the completed operation ends when an event is raised into the callback
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_set returned error %d\r\n",retVal );
  }
  else
  {
    if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
        eventsHandleLwm2m,
        EV_LWM2M_SET_RES_BIT | EV_LWM2M_FAIL_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_set timeout!\r\n");
      res = -2;
    }
    else
    {
      if ((curEvBits & EV_LWM2M_FAIL_RES_BIT) == EV_LWM2M_FAIL_RES_BIT)
      {
        AZX_LOG_ERROR("Failure event arrived!\r\n");
        res = -3;
      }
      else
      {
        res = 0;
      }
    }
  }

  /*release critical section*/
  m2mb_os_sem_put(lwm2mCSSemHandle);
  return res;
}

/*-----------------------------------------------------------------------------------------------*/

INT32 write_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 inbuflen)
{
  UINT32                  curEvBits;
  INT32 res = -1;
  M2MB_RESULT_E retVal;

  /*Get critical section*/
  m2mb_os_sem_get(lwm2mCSSemHandle, M2MB_OS_WAIT_FOREVER);

  m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_CLEAR);
  retVal = m2mb_lwm2m_write( h, uri, inbuf, inbuflen);

  //retVal is just the return value of the API, not the completed operation
  //the completed operation ends when an event is raised into the callback
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_write returned error %d\r\n",retVal );
  }
  else
  {
    if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
        eventsHandleLwm2m,
        EV_LWM2M_WRITE_RES_BIT | EV_LWM2M_FAIL_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_write timeout!\r\n");
      res = -2;
    }
    else
    {
      if ((curEvBits & EV_LWM2M_FAIL_RES_BIT) == EV_LWM2M_FAIL_RES_BIT)
      {
        AZX_LOG_ERROR("Failure event arrived!\r\n");
        res = -3;
      }
      else
      {
        res = 0;
      }
    }
  }

  /*release critical section*/
  m2mb_os_sem_put(lwm2mCSSemHandle);
  return res;
}

/*-----------------------------------------------------------------------------------------------*/

INT32 get_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 *inbuflen)
{
  UINT32                  curEvBits;
  INT32 res = -1;
  M2MB_RESULT_E retVal;

  /*Get critical section*/
  m2mb_os_sem_get(lwm2mCSSemHandle, M2MB_OS_WAIT_FOREVER);

  m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_CLEAR);

  retVal = m2mb_lwm2m_get( h, uri, inbuf, *inbuflen);

  //retVal is just the return value of the API, not the completed operation
  //the completed operation ends when an event is raised into the callback
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_get returned error %d\r\n",retVal );

  }
  else
  {
    if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
        eventsHandleLwm2m,
        EV_LWM2M_GET_RES_BIT | EV_LWM2M_FAIL_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_get timeout!\r\n");
      res = -2;
    }
    else
    {
      if ((curEvBits & EV_LWM2M_FAIL_RES_BIT) == EV_LWM2M_FAIL_RES_BIT)
      {
        AZX_LOG_ERROR("Failure event arrived!\r\n");
        res = -3;
      }
      else
      {

        res = 0;
      }
    }
  }
  /*release critical section*/
  m2mb_os_sem_put(lwm2mCSSemHandle);
  return res;
}

/*-----------------------------------------------------------------------------------------------*/

INT32 read_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 *inbuflen)
{
  UINT32  curEvBits;
  INT32   res = -1;

  /*Get critical section*/
  m2mb_os_sem_get(lwm2mCSSemHandle, M2MB_OS_WAIT_FOREVER);

  m2mb_os_ev_set(eventsHandleLwm2m, EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_CLEAR);
  M2MB_RESULT_E retVal = m2mb_lwm2m_read( h, uri, inbuf, *inbuflen);

  //retVal is just the return value of the API, not the completed operation
  //the completed operation ends when an event is raised into the callback
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_lwm2m_read returned error %d\r\n",retVal );
  }
  else
  {
    if( M2MB_OS_SUCCESS != m2mb_os_ev_get(
        eventsHandleLwm2m,
        EV_LWM2M_READ_RES_BIT | EV_LWM2M_FAIL_RES_BIT,
        M2MB_OS_EV_GET_ANY_AND_CLEAR,
        &curEvBits,
        M2MB_OS_MS2TICKS(10000) /*wait 10 seconds for the event to occur*/
    )
    )
    {
      AZX_LOG_ERROR("m2mb_lwm2m_read timeout!\r\n");
      res = -2;
    }
    else
    {
      if ((curEvBits & EV_LWM2M_FAIL_RES_BIT) == EV_LWM2M_FAIL_RES_BIT)
      {
        AZX_LOG_ERROR("Failure event arrived!\r\n");
        res = -3;
      }
      else
      {
        res = 0;
      }
    }
  }
  /*release critical section*/
  m2mb_os_sem_put(lwm2mCSSemHandle);
  return res;
}

/*-----------------------------------------------------------------------------------------------*/

/*=====================================
 * SET READ ONLY RESOURCES UTILITIES
 * ===================================*/


INT32 set_read_only_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 value)
{
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

  return set_resource( h, pUri, &value, (UINT16)sizeof(INT32));
}

/*-----------------------------------------------------------------------------------------------*/

INT32 set_read_only_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 value)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nSetting timestamp resource {%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nSetting timestamp resource {%u/%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }

  return set_resource( h, pUri, &value, (UINT16)sizeof(UINT64));
}

/*-----------------------------------------------------------------------------------------------*/


INT32 set_read_only_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double value)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nSetting double resource {%u/%u/%u} value to %f on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nSetting double resource {%u/%u/%u/%u} value to %f on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }
  return set_resource(h, pUri, &value, (UINT16)sizeof(double));
}

/*-----------------------------------------------------------------------------------------------*/

INT32 set_read_only_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN value)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nSetting boolean resource {%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nSetting boolean resource {%u/%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }

  return set_resource(h, pUri, &value, (UINT16)sizeof(INT32));
}

/*-----------------------------------------------------------------------------------------------*/

INT32 set_read_only_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 datalen)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nSetting opaque resource {%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nSetting opaque resource {%u/%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  return set_resource(h, pUri, data, datalen);
}

/*-----------------------------------------------------------------------------------------------*/

INT32 set_read_only_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *data)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nSetting string resource {%u/%u/%u} value to <%s> on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        data);
  }
  else
  {
    AZX_LOG_INFO("\r\nSetting string resource {%u/%u/%u/%u} value to <%s> on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        data);
  }

  return set_resource(h, pUri, data, (UINT16)strlen(data));
}

/*-----------------------------------------------------------------------------------------------*/

/*=====================================
 * WRITE RW (or WRITE-ONLY) RESOURCES UTILITIES
 * ===================================*/


INT32 write_rw_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 value)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nWriting integer resource {%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nWriting integer resource {%u/%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }

  return write_resource( h, pUri, &value, (UINT16)sizeof(INT32));
}

/*-----------------------------------------------------------------------------------------------*/

INT32 write_rw_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 value)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nWriting timestamp resource {%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nWriting timestamp resource {%u/%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }

  return write_resource( h, pUri, &value, (UINT16)sizeof(UINT64));
}

/*-----------------------------------------------------------------------------------------------*/

INT32 write_rw_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double value)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nWriting double resource {%u/%u/%u} value to %f on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nWriting double resource {%u/%u/%u/%u} value to %f on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }
  return write_resource(h, pUri, &value, (UINT16)sizeof(double));
}

/*-----------------------------------------------------------------------------------------------*/

INT32 write_rw_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN value)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nWriting boolean resource {%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        value);
  }
  else
  {
    AZX_LOG_INFO("\r\nWriting boolean resource {%u/%u/%u/%u} value to %d on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        value);
  }
  return write_resource(h, pUri, &value, (UINT16)sizeof(int));
}

/*-----------------------------------------------------------------------------------------------*/


INT32 write_rw_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 datalen)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nWriting opaque resource {%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nWriting opaque resource {%u/%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }
  return write_resource(h, pUri, data, datalen);
}

/*-----------------------------------------------------------------------------------------------*/

INT32 write_rw_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *data)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nWriting string resource {%u/%u/%u} value to <%s> on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource,
        data);
  }
  else
  {
    AZX_LOG_INFO("\r\nWriting string resource {%u/%u/%u/%u} value to <%s> on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst,
        data);
  }
  return write_resource(h, pUri, data, strlen(data));
}


/*=====================================
 * GET WRITE ONLY RESOURCES UTILITIES
 * ===================================*/
/*-----------------------------------------------------------------------------------------------*/


INT32 get_write_only_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 *value)
{
  INT32 data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nGetting integer resource {%u/%u/%u} valueon LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nGetting integer resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  retVal = get_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    *value = data[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 get_write_only_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 *value)
{
  INT32 data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nGetting timestamp resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nGetting timestamp resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }
  retVal = get_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    UINT64 *pData = (UINT64 *)data;
    *value = pData[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 get_write_only_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double *value)
{
  double data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nGetting double resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nGetting double resource {%u/%u/%u/%u} value  on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }
  retVal = get_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    *value = data[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 get_write_only_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN *value)
{
  INT32 data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nGetting boolean resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nGetting boolean resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  retVal = get_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    *value = (BOOLEAN)data[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 get_write_only_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 *datalen)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nGetting opaque resource {%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nGetting opaque resource {%u/%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  return get_resource(h, pUri, data, datalen);
}

/*-----------------------------------------------------------------------------------------------*/


INT32 get_write_only_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *string, UINT16 string_max_size)
{
  UINT16 dataLen = string_max_size;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nGetting string resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nGetting string resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  return get_resource(h, pUri, string, &dataLen );
}




/*=====================================
 * READ RW (OR READ-ONLY) RESOURCES UTILITIES
 * ===================================*/

/*-----------------------------------------------------------------------------------------------*/


INT32 read_rw_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 *value)
{
  INT32 data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nReading integer resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nReading integer resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  retVal = read_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    *value = data[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 read_rw_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 *value)
{
  INT32 data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nReading timestamp resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nReading timestamp resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }
  retVal = read_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    UINT64 *pData = (UINT64 *)data;
    *value = pData[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 read_rw_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double *value)
{
  double data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nReading double resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nReading double resource {%u/%u/%u/%u} value  on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }
  retVal = read_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    *value = data[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 read_rw_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN *value)
{
  INT32 data[2];
  UINT16 datasize = sizeof(data);
  INT32 retVal;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nReading boolean resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nReading boolean resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  retVal = read_resource( h, pUri,  data, &datasize);
  if(retVal == 0)
  {
    *value = (BOOLEAN)data[0];
  }
  return retVal;
}

/*-----------------------------------------------------------------------------------------------*/


INT32 read_rw_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 *datalen)
{
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nReading opaque resource {%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nReading opaque resource {%u/%u/%u/%u} on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  return read_resource(h, pUri, data, datalen);
}

/*-----------------------------------------------------------------------------------------------*/


INT32 read_rw_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *string, UINT16 string_max_size)
{
  UINT16 dataLen = string_max_size;
  if(pUri->uriLen == M2MB_LWM2M_URI_3_FIELDS)
  {
    AZX_LOG_INFO("\r\nReading string resource {%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource);
  }
  else
  {
    AZX_LOG_INFO("\r\nReading string resource {%u/%u/%u/%u} value on LWM2M client.\r\n",
        pUri->obj, pUri->objInst, pUri->resource, pUri->resourceInst);
  }

  return read_resource(h, pUri, string, &dataLen );
}

/**/

