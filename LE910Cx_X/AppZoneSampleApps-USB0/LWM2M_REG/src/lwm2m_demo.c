/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    lwm2m_demo.c

  @brief
    The file contains the LWM2M utilities

  @details

  @version
    1.0.5
  @note


  @author


  @date
    26/01/2023
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

#define OBJECT_XML_NAME "object_32010.xml"
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

/*===== ONEEDGE =====*/

/*Handles*/
static M2MB_LWM2M_HANDLE lwm2mHandle;

/*IDs*/
static UINT16 agentID = 0;     /* Telit agent ID */
static UINT16 shServerID = 0;  /* Short server ID */

/* Registration to a LwM2M Server variables */
static M2MB_LWM2M_REG_PARAM_T regReq;
static M2MB_LWM2M_REG_ELEMENT_RES_T *pRegList = NULL;
static UINT16 regListSize = 0;

/* Local function prototypes ====================================================================*/
/*-----------------------------------------------------------------------------
*!
* @brief
*   Callback function for Client generated LWM2M events
*
* @details
*   WARNING: do not place blocking action within callback, since it is issued by the agent's task!!
*
* @param
* @param[in]    h           LWM2M handle
* @param[in]    event       Event Identification
* @param[in]    resp_size   Size of the response buffer
* @param[in]    resp_struct Address of response buffer
* @param[in]    userdata    Address of user data
*
* @return
*   None
*
* @see
*
* @note
*
* @code
*
* @endcode
*
-----------------------------------------------------------------------------*/
static void lwm2mIndicationCB( M2MB_LWM2M_HANDLE h, M2MB_LWM2M_EVENT_E event,
    UINT16 resp_size, void *resp_struct, void *userdata );

/*-----------------------------------------------------------------------------
*!
* @brief
*   Execute LWM2M REG actions.
*
* @details
*   The actions available for this function are listed within the structure M2MB_LWM2M_REG_ACTION_E (defined inside m2mb_lwm2m.h).
*   What these action perform is described as follow:
*   M2MB_LWM2M_REG_ACTION_FORCE_DEREG  -> deregister from the given server request
*   M2MB_LWM2M_REG_ACTION_FORCE_REG    -> registration towards the given server request
*   M2MB_LWM2M_REG_ACTION_UPDATE_REG   -> update registration towards the given server request
*   M2MB_LWM2M_REG_ACTION_GET_SRV_INFO -> retrive the server registration status request
*
* @param
*   @param[in]    regAction     REG action requested
*
* @return
*   The function returns M2MB_RESULT_SUCCESS in case of success, otherwise returns M2MB_RESULT_FAIL if any error occurs
*
* @see
* "AT#LWM2MREG" command inside "LWM2M AT Commands Reference Guide"
*
* @note
*
* @code
*
* @endcode
*
-----------------------------------------------------------------------------*/
static M2MB_RESULT_E lwm2m_reg(M2MB_LWM2M_REG_ACTION_E regAction);
/*-----------------------------------------------------------------------------
*!
* @brief
*   Check existence of XML object description file in XML directory
*
* @details
*
* @param
* @param[in]    name    Name of the file to be checked
*
* @return
*   Returns 0 if OK, other values error
*
* @see
*
* @note
*
* @code
*
* @endcode
*
-----------------------------------------------------------------------------*/
static int check_xml_file(const char* name);

/* Static functions =============================================================================*/
static void lwm2mIndicationCB( M2MB_LWM2M_HANDLE h, M2MB_LWM2M_EVENT_E event, UINT16 resp_size, void *resp_struct,
    void *userdata )
{
  UNUSED_3(h, resp_size, userdata);

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
      AZX_LOG_DEBUG( "LWM2M disable result OK\r\n");
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_DISABLE_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_WARN( "Disable result %d\r\n", resp->result );
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_FAIL_RES_BIT, M2MB_OS_EV_SET);
    }
    break;
  }

  /* event in response to any change in the client registration state */
  case  M2MB_LWM2M_SRV_INFO_IND:
  {
    M2MB_LWM2M_SRV_INFO_IND_T *resp = ( M2MB_LWM2M_SRV_INFO_IND_T * )resp_struct;

    AZX_LOG_TRACE("M2MB_LWM2M_SRV_INFO_IND, resp->info = %d\r\n", resp->info);

    switch(resp->info)
    {
    case M2MB_LWM2M_CL_STATE_DISABLED:
      AZX_LOG_DEBUG("M2MB_LWM2M_CL_STATE_DISABLED\r\n");
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_SRV_DISABLED_BIT, M2MB_OS_EV_SET);
      break;
    case M2MB_LWM2M_CL_STATE_BOOTSTRAPPING:
      AZX_LOG_DEBUG( "resp->info == M2MB_LWM2M_CL_STATE_BOOTSTRAPPING\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_BOOTSTRAPPED:
      AZX_LOG_DEBUG( "resp->info == M2MB_LWM2M_CL_STATE_BOOTSTRAPPED\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_REGISTERING:
      AZX_LOG_DEBUG( "resp->info == M2MB_LWM2M_CL_STATE_REGISTERING\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_REGISTERED:
      AZX_LOG_DEBUG( "resp->info == M2MB_LWM2M_CL_STATE_REGISTERED\tresp->shServerId == %d\r\n", resp->shServerId );
      shServerID = resp->shServerId;
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_SRV_REG_BIT, M2MB_OS_EV_SET);
      break;
    case M2MB_LWM2M_CL_STATE_DEREGISTERING:
      AZX_LOG_DEBUG( "resp->info == M2MB_LWM2M_CL_STATE_DEREGISTERING\r\n" );
      break;
    case M2MB_LWM2M_CL_STATE_SUSPENDED:
      AZX_LOG_DEBUG( "resp->info == M2MB_LWM2M_CL_STATE_SUSPENDED\r\n" );
      break;
    default:
      AZX_LOG_WARN( "resp->info: unexpected value!! %d\r\n", resp->info);
      break;
    }
    break;
  }

  /* event in response to m2mb_lwm2m_get_reg_entries() */
  case M2MB_LWM2M_GET_REG_NUM_RES:
  {
    M2MB_LWM2M_GET_REG_NUM_RES_T *resp = ( M2MB_LWM2M_GET_REG_NUM_RES_T * )resp_struct;

    if( resp != NULL )
    {
      if( resp->result == M2MB_LWM2M_RES_SUCCESS )
      {
        AZX_LOG_DEBUG( "Overall items number is %d\r\n", resp->regEntries);
        regListSize = resp->regEntries;
        m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_GET_REG_ENTRIES_RES_BIT, M2MB_OS_EV_SET);
      }
      else
      {
        AZX_LOG_ERROR( "Getting M2MB_LWM2M_GET_REG_NUM_RES result %d\r\n", resp->result );
      }
    }
    else
    {
      AZX_LOG_ERROR( "invalid pointer received\r\n" );
    }
    break;
  }

  /* event in response to m2mb_lwm2m_reg() */
  case M2MB_LWM2M_REG_RES:
  {
    M2MB_LWM2M_REG_RES_T *resp = ( M2MB_LWM2M_REG_RES_T * )resp_struct;
    if( resp != NULL )
    {
      if( resp->result == M2MB_LWM2M_RES_SUCCESS )
      {
        if(pRegList != NULL)
        {
          memcpy(pRegList, resp->list, (resp->listSize * sizeof(M2MB_LWM2M_REG_ELEMENT_RES_T)));
        }
      }
      else
      {
        AZX_LOG_ERROR( "M2MB_LWM2M_REG_RES FAIL. error code: %d\r\n", resp->error);
      }
      m2mb_os_ev_set(get_lwm2mEvents_handle(), EV_LWM2M_REG_RES_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      AZX_LOG_ERROR( "invalid pointer received\r\n" );
    }
    break;
  }

  default:
    AZX_LOG_DEBUG( "LWM2M EVENT %d\r\n", event );
    break;
  }
}
/*-----------------------------------------------------------------------------------------------*/
static M2MB_RESULT_E lwm2m_reg(M2MB_LWM2M_REG_ACTION_E regAction)
{
  UINT32 curEvBits;
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  switch(regAction)
  {
    case M2MB_LWM2M_REG_ACTION_FORCE_DEREG:
    {
      /*
       * Start deregistration action for the required server (by using shServerID) for the current LWM2M agent (defined by agentID)
       * Because there is no data retrieved by this action, m2mb_lwm2m_reg api takes listPointer as NULL
       * This section acts as: AT#LWM2MREG=<agentID>,0,<shServerID>
       */
      AZX_LOG_INFO("Deregistration action using LWM2M REG api!\r\n");
      memset( &regReq, 0, sizeof( M2MB_LWM2M_REG_PARAM_T ) );
      regReq.agentId = agentID;
      regReq.serverId = &shServerID;
      regReq.actionId = M2MB_LWM2M_REG_ACTION_FORCE_DEREG;
      if( m2mb_lwm2m_reg( lwm2mHandle, &regReq, NULL, regListSize ) == M2MB_RESULT_SUCCESS )
      {
        /*
         * Wait for m2mb_lwm2m_reg event and then wait for the deregistration event
         */
        m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_REG_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
        AZX_LOG_INFO("Waiting deregistration event!\r\n");
        m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_SRV_DISABLED_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
        AZX_LOG_INFO("Deregistration event received!\r\n");
      }
      else
      {
        AZX_LOG_ERROR( "m2mb_lwm2m_reg request failed\r\n" );
        retVal = M2MB_RESULT_FAIL;
      }
      break;
    }
    case M2MB_LWM2M_REG_ACTION_FORCE_REG:
    {
      /*
       * Start registration action for the required server (by using shServerID) for the current LWM2M agent (defined by agentID)
       * Because there is no data retrieved by this action, m2mb_lwm2m_reg api takes listPointer as NULL
       * This section acts as: AT#LWM2MREG=<agentID>,1,<shServerID>
       */
      AZX_LOG_INFO("Registration action using LWM2M REG api!\r\n");
      memset( &regReq, 0, sizeof( M2MB_LWM2M_REG_PARAM_T ) );
      regReq.agentId = agentID;
      regReq.serverId = &shServerID; // requires the entire list of servers data to be returned
      regReq.actionId = M2MB_LWM2M_REG_ACTION_FORCE_REG;
      if( m2mb_lwm2m_reg( lwm2mHandle, &regReq, NULL, regListSize ) == M2MB_RESULT_SUCCESS )
      {
        /*
         * Wait for m2mb_lwm2m_reg event and then wait for the registration event
         */
        m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_REG_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
        AZX_LOG_INFO("Waiting registration event!\r\n");
        m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_SRV_REG_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
        AZX_LOG_INFO("Registration event received!\r\n");
      }
      else
      {
        AZX_LOG_ERROR( "m2mb_lwm2m_reg request failed\r\n" );
        retVal = M2MB_RESULT_FAIL;
      }
      /*******************************************************************/
      break;
    }
    case M2MB_LWM2M_REG_ACTION_UPDATE_REG:
    {
      break;
    }
    case M2MB_LWM2M_REG_ACTION_GET_SRV_INFO:
    {
      /*
       * Get all the entries for server information.
       * The number of entries is returned by the event M2MB_LWM2M_GET_REG_NUM_RES
       */
      if( m2mb_lwm2m_get_reg_entries( lwm2mHandle ) != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR( "m2mb_lwm2m_get_reg_entries request failed\r\n" );
        retVal = M2MB_RESULT_FAIL;
      }
      else
      {
        m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_GET_REG_ENTRIES_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

        if(regListSize)
        {
          /*
           * Get information action for the required server (by using shServerID) for the current LWM2M agent (defined by agentID)
           * If all information about all server are required, regReq.serverId has to be set as NULL
           * This section acts as: AT#LWM2MREG=<agentID>,3,<shServerID>
           */
          memset( &regReq, 0, sizeof( M2MB_LWM2M_REG_PARAM_T ) );
          regReq.agentId = agentID;
          regReq.actionId = M2MB_LWM2M_REG_ACTION_GET_SRV_INFO;
          regReq.serverId = &shServerID;
          pRegList = ( M2MB_LWM2M_REG_ELEMENT_RES_T * )m2mb_os_calloc( regListSize * sizeof( M2MB_LWM2M_REG_ELEMENT_RES_T ) );
          if( pRegList != NULL )
          {
            if( m2mb_lwm2m_reg( lwm2mHandle, &regReq, pRegList, regListSize ) == M2MB_RESULT_SUCCESS )
            {
              m2mb_os_ev_get(get_lwm2mEvents_handle(), EV_LWM2M_REG_RES_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
              for(UINT16 regInfo = 0; regInfo < regListSize; regInfo++)
              {
                AZX_LOG_INFO("sshid[%d] status[%d] lastReg[%lld] nextReg[%lld] lastActivity[%lld]\r\n",
                        pRegList->sshid,
                        pRegList->status,
                        pRegList->lastReg,
                        pRegList->nextReg,
                        pRegList->lastActivity);
                pRegList++;
              }
            }
            else
            {
              AZX_LOG_ERROR( "m2mb_lwm2m_reg request failed\r\n" );
              retVal = M2MB_RESULT_FAIL;
            }
            m2mb_os_free(pRegList);
          }
          else
          {
            retVal = M2MB_RESULT_FAIL;
          }
        }
        else
        {
          AZX_LOG_WARN( "No entries for m2mb_lwm2m_reg\r\n" );
          retVal = M2MB_RESULT_FAIL;
        }
      }
      break;
    }
    case M2MB_LWM2M_REG_ACTION_MAX_NUM:
    default:
      break;
  }

  return retVal;
}
/*-----------------------------------------------------------------------------------------------*/
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

/* Global functions =============================================================================*/

UINT8 oneedge_init( void)
{
  M2MB_RESULT_E retVal;
  M2MB_LWM2M_ENABLE_REQ_T pars;

  M2MB_OS_RESULT_E        osRes;
  UINT32                  curEvBits;

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

    m2mb_lwm2m_deinit(lwm2mHandle);
    lwm2mHandle = NULL;
    return 1;
  }

  azx_sleep_ms(1000);

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
  UNUSED_3(type, param1, param2);

  INT32  task_status = type;
  INT32  ret         = 0;
  UINT32 curEvBits;

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

    /*
     * Get current LWM2M server information
     */
    if(lwm2m_reg(M2MB_LWM2M_REG_ACTION_GET_SRV_INFO) != M2MB_RESULT_SUCCESS)
    {
      task_status = APPLICATION_EXIT;
      break;
    }
    azx_sleep_ms(2000);

    /*
     * Act client deregistration
     */
    if(lwm2m_reg(M2MB_LWM2M_REG_ACTION_FORCE_DEREG) != M2MB_RESULT_SUCCESS)
    {
      task_status = APPLICATION_EXIT;
      break;
    }
    azx_sleep_ms(10000);

    /*
     * Act client registration
     */
    if(lwm2m_reg(M2MB_LWM2M_REG_ACTION_FORCE_REG) != M2MB_RESULT_SUCCESS)
    {
      task_status = APPLICATION_EXIT;
      break;
    }
    azx_sleep_ms(2000);

    task_status = APPLICATION_EXIT;
  } while(0);

  if (task_status == APPLICATION_EXIT)
  {
    if(lwm2mHandle)
    {
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
    }

    deinit_sync();

    AZX_LOG_INFO("Application complete.\r\n");
  }

  return 0;
}

