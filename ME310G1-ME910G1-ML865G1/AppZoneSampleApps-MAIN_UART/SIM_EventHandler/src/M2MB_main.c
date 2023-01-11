/*=========================================================================*/
/*   Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved.  */
/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sim Event Demo application. Debug prints on MAIN UART, <ins>using AZX log example functions</ins>
  @version
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author FabioPi

  @date
    2020-02-26
 */

/* Include files ================================================================================*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_fs_stdio.h"
#include "m2mb_ati.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"


/* Local defines ================================================================================*/
#define AT2_INSTANCE 1 /* AT instance number 2 to be used for URC, see 'AT#PORTCFG?' documentation to know which interface will be release */

#define AT_BUF_SIZE 4096
#define AT_RSP_TIMEOUT 120000

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
static unsigned char g_at_rsp_buf[AT_BUF_SIZE];
static UINT16 buf_index = 0;


static M2MB_ATI_HANDLE ati_handles[2];

static M2MB_OS_SEM_HANDLE at_rsp_sem = NULL;
INT32 at_taskId;

INT32 simev_taskId;

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
 * @brief       Sends a command to the required AT instance
 *
 * @param[in]    instance     The instance number to which the command will be sent
 * @param[in]    atCmd        The command to be sent
 * @param[out]   atRsp        The response buffer that will be filled with the command response
 * @param[in]    atRspMaxLen  The max size of the response buffer
 *
 * @retval       M2MB_RESULT_E value
 *
 */
static M2MB_RESULT_E send_async_at_command(INT16 instance, const CHAR *atCmd, CHAR *atRsp, UINT32 atRspMaxLen);

/*!
 * @brief        AT deinitialization utility. Releases the required AT instance
 *
 * @param[in]    instance     The instance number to be released (0-2)
 * @retval       M2MB_RESULT_E value
 *
 */
M2MB_RESULT_E at_cmd_async_deinit(INT16 instance);


/*!
 * @brief        AT URC callback, called with #SIMPR URCs arrive
 *
 * @param[in]    msg     The incoming urc message
 *
 */
static void SIMPR_urc_cb(const CHAR* msg);


/*!
 * @brief        User callback function that will be called if the SIM card is inserted
 *
 *
 */
static INT32 onSIMInserted(void);

/*!
 * @brief        User callback function that will be called if the SIM card gets removed
 *
 *
 */
static INT32 onSIMRemoved(void);

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

  AZX_LOG_TRACE("m2mb_ati_init() on instance %d\r\n", instance);
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


static M2MB_RESULT_E send_async_at_command(INT16 instance, const CHAR *atCmd, CHAR *atRsp, UINT32 atRspMaxLen)
{
  INT32 cmd_len = 0;
  M2MB_RESULT_E retVal;

  AZX_LOG_TRACE("Sending AT Command: %.*s\r\n",strlen(atCmd) -1, atCmd);

  m2mb_os_sem_get(at_rsp_sem, M2MB_OS_WAIT_FOREVER );  //get critical section

  cmd_len = strlen(atCmd);

  retVal = m2mb_ati_send_cmd(ati_handles[instance], (void*) atCmd, cmd_len);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("m2mb_ati_send_cmd() returned failure value\r\n");
    return retVal;
  }

  AZX_LOG_TRACE("Waiting command response...\r\n");
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
    memcpy(atRsp, g_at_rsp_buf, buf_index);
    m2mb_os_sem_put(at_rsp_sem);  /*Release CS*/
    return M2MB_RESULT_SUCCESS;
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

static INT32 onSIMInserted(void)
{
  //function to call when SIM is removed.
  AZX_LOG_INFO("\r\nSIM CARD HAS BEEN INSERTED!\r\n\r\n");

  return 0;
}

static INT32 onSIMRemoved(void)
{
  //function to call when SIM is removed.
  AZX_LOG_INFO("\r\nSIM CARD HAS BEEN REMOVED!\r\n\r\n");

  return 0;
}

/*-----------------------------------------------------------------------------------------------*/

static void SIMPR_urc_cb(const CHAR* msg)
{
  int sim, state;
  AZX_LOG_TRACE("SIMPR_urc_cb: <%s>\r\n", msg);
  if(2 != sscanf(msg,"#SIMPR: %d,%d", &sim, &state))
  {
    AZX_LOG_ERROR("cannot parse URC\r\n");
  }
  else
  {
    AZX_LOG_INFO("SIM %d state changed to %d!\r\n", sim, state);
    switch(state)
    {
    case 0:
      azx_tasks_sendMessageToTask(simev_taskId, 0, 0, 0);
      break;
    case 1:
      azx_tasks_sendMessageToTask(simev_taskId, 1, 0, 0);
      break;
    default:
      break;
    }
  }
}

/*-----------------------------------------------------------------------------------------------*/
static INT32 SIMEvTaskCB(INT32 ev, INT32 param1, INT32 param2)
{
  UNUSED_2(param1,param2);
  switch(ev)
  {
  case 0:
    onSIMRemoved();
    break;
  case 1:
    onSIMInserted();
    break;
  default:
    break;
  }

  return 0;
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
    char *p = strstr((const char*)g_at_rsp_buf,"#SIMPR: ");
    if(p)
    {
      char *e = strstr(p, "\r\n");
      if(e)
      {
        *e = '\0';
      }
      AZX_LOG_TRACE("SIMPR URC received.\r\n");
      SIMPR_urc_cb(p);
      memset(g_at_rsp_buf,0, AT_BUF_SIZE);
      buf_index = 0;
    }
    m2mb_os_sem_put(at_rsp_sem);  /*Release "IPC"*/
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------*/

/* Global functions =============================================================================*/
/*-----------------------------------------------------------------------------------------------*/


/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  ( void )argc;
  ( void )argv;

  CHAR rsp[100];
  M2MB_RESULT_E retVal;

  azx_tasks_init();

  azx_sleep_ms(3000);

  /* SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO( "Starting SIM Presence Demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__ );

  AZX_LOG_INFO("Please ensure SIM is not inserted before starting this sample application\r\n");

  at_cmd_async_init(AT2_INSTANCE);
  at_taskId = azx_tasks_createTask((CHAR*)"AT TASK", AZX_TASKS_STACK_XL, 10, AZX_TASKS_MBOX_M, atTaskCB);
  memset(g_at_rsp_buf,0, AT_BUF_SIZE);

  simev_taskId = azx_tasks_createTask((CHAR*)"SIM events", AZX_TASKS_STACK_XL, 11, AZX_TASKS_MBOX_M, SIMEvTaskCB);


  AZX_LOG_INFO("Sending command AT#SIMPR=1 to enable SIM presence URC messages...\r\n");
  retVal = send_async_at_command(AT2_INSTANCE, "AT#SIMPR=1\r", rsp, sizeof(rsp));
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Error sending command AT#SIMPR=1\r\n" );
  }
  else
  {
    if(strstr(rsp, "\r\nOK\r\n"))
    {
       AZX_LOG_INFO("Command set.\r\n");
    }
    else
    {
      AZX_LOG_WARN("Command response: <%s>\r\n\r\n", rsp);
    }
  }
}


