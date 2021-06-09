/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing how to perform an AT tunnel from Main UART to an AT instance. Debug prints on **USB1**.
  @version 
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    02/03/2019
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_uart.h"
#include "m2mb_ati.h"

#include "azx_tasks.h"
#include "azx_log.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
enum
{
  UART_TASK_READ_AND_SEND,
  UART_TASK_GET_AND_WRITE
};
/* Local statics ================================================================================*/

#define ATI_ID 0 /*AT0, bound to UART by default config*/



static M2MB_ATI_HANDLE ati_handles[2];

static INT32 main_uart_fd = -1;
static INT32 uart_task_id = -1;
static CHAR inBuf[1024];


/* Local function prototypes ====================================================================*/
static INT32 msgUARTTask(INT32 type, INT32 param1, INT32 param2);
static void UART_Cb( INT32 fd, M2MB_UART_IND_E uart_event, UINT16 resp_size, void *resp_struct, void *userdata );
static void at_cmd_async_callback ( M2MB_ATI_HANDLE h, M2MB_ATI_EVENTS_E ati_event, UINT16 resp_size,
                                    void *resp_struct, void *userdata );
static M2MB_RESULT_E at_cmd_async_init(INT16 instance);
static M2MB_RESULT_E at_cmd_async_deinit(INT16 instance);
static int init_uart(void);

/* Static functions =============================================================================*/


static INT32 msgUARTTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)param1;

  INT32 read, written;
  M2MB_RESULT_E retVal;
  SSIZE_T rsp_len;

  switch (type)
  {
  case UART_TASK_READ_AND_SEND:
    memset(inBuf, 0, sizeof(inBuf));
    AZX_LOG_DEBUG("Received data on uart, read it and send on ATI\r\n");
    read = m2mb_uart_read( main_uart_fd, inBuf, (SIZE_T)param2);
    if(read == -1)
    {
      AZX_LOG_ERROR("failed reading from uart!\r\n");
      return -1;
    }
    else
    {
      AZX_LOG_INFO("UART IN: <%.*s>. Sending to ATI...\r\n", read, inBuf);

      retVal = m2mb_ati_send_cmd(ati_handles[ATI_ID], (void*) inBuf, read);
      if ( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_ati_send_cmd() returned failure value\r\n");
        return retVal;
      }

    }
    break;

  case UART_TASK_GET_AND_WRITE:
    memset(inBuf, 0, sizeof(inBuf));
    AZX_LOG_DEBUG("Received data on ATI, read it and send on UART\r\n");

    rsp_len = m2mb_ati_rcv_resp(ati_handles[ATI_ID], inBuf, sizeof(inBuf));
    if(rsp_len == -1)
    {
      AZX_LOG_ERROR("Cannot read from ATI!\r\n");
      return -1;
    }

    AZX_LOG_DEBUG("Received: <%.*s>\r\n", rsp_len, inBuf);
    if(rsp_len > 0)
    {
      written = m2mb_uart_write( main_uart_fd, inBuf, (SIZE_T)rsp_len);
      if(written == -1)
      {
        AZX_LOG_ERROR("failed writing in uart!\r\n");
        return -1;
      }
    }
    break;

  default:
    AZX_LOG_WARN("Unexpected: %d..\r\n", type);
    break;
  }
  return 0;
}


static void UART_Cb( INT32 fd, M2MB_UART_IND_E uart_event, UINT16 resp_size, void *resp_struct, void *userdata )
{
  (void)fd;
  (void)resp_size;
  (void)userdata;

  UINT32 myUartRxLen;

  if( M2MB_UART_RX_EV == uart_event )
  {
    myUartRxLen = *((UINT32 *)resp_struct);
    AZX_LOG_DEBUG("Received %u bytes\r\n", myUartRxLen);
    azx_tasks_sendMessageToTask( uart_task_id, UART_TASK_READ_AND_SEND, 0, (INT32)myUartRxLen );
  }
}


static void at_cmd_async_callback ( M2MB_ATI_HANDLE h, M2MB_ATI_EVENTS_E ati_event, UINT16 resp_size,
                                     void *resp_struct, void *userdata )
{
  (void)h;
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
    AZX_LOG_DEBUG("Callback - available bytes: %d\r\n", resp_len);
    azx_tasks_sendMessageToTask( uart_task_id, UART_TASK_GET_AND_WRITE, 0, (INT32)resp_len );

  }

}



static M2MB_RESULT_E at_cmd_async_init(INT16 instance)
{

  AZX_LOG_DEBUG("m2mb_ati_init() on instance %d\r\n", instance);
  if ( m2mb_ati_init(&ati_handles[instance], instance, at_cmd_async_callback, NULL) == M2MB_RESULT_SUCCESS )
  {
    return M2MB_RESULT_SUCCESS;
  }
  else
  {
    AZX_LOG_ERROR("m2mb_ati_init() returned failure value\r\n" );
    return M2MB_RESULT_FAIL;
  }
}

static M2MB_RESULT_E at_cmd_async_deinit(INT16 instance)
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



static int init_uart(void)
{
  M2MB_RESULT_E retVal;
  M2MB_UART_CFG_T cfg;
  /* Get a UART handle first */
  if(main_uart_fd == -1)
  {
    main_uart_fd = m2mb_uart_open( "/dev/tty0", 0 );
  }


  if(main_uart_fd != -1)
  {

    AZX_LOG_INFO("Uart opened, setting callback for data..\r\n");
    m2mb_uart_ioctl(main_uart_fd, M2MB_UART_IOCTL_GET_CFG, &cfg);
    cfg.cb_fn = UART_Cb;
    m2mb_uart_ioctl(main_uart_fd, M2MB_UART_IOCTL_SET_CFG, &cfg);

    uart_task_id = azx_tasks_createTask((char*) "uart_task", AZX_TASKS_STACK_XL, 5, AZX_TASKS_MBOX_S, msgUARTTask);

    AZX_LOG_TRACE("Task ID: %d.\r\n", uart_task_id);

    if (uart_task_id <= 0)
    {
      AZX_LOG_ERROR("cannot create uart task!\r\n");
      return -1;
    }

    retVal = at_cmd_async_init(ATI_ID);
     if ( retVal == M2MB_RESULT_SUCCESS )
     {
       AZX_LOG_TRACE( "at_cmd_async_init() returned success value\r\n" );
     }
     else
     {
       AZX_LOG_ERROR( "at_cmd_async_init() returned failure value\r\n" );
       m2mb_uart_close(main_uart_fd);
       return -1;
     }
    m2mb_uart_write(main_uart_fd, "Starting AT tunnel demo app. Waiting for AT commands...\r\n", strlen("Starting AT tunnel demo app. Waiting for AT commands...\r\n"));
    return 0;
  }
  else
  {
    return -1;
  }
}

/* Global functions =============================================================================*/
/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{

  (void)argc;
  (void)argv;
  AZX_LOG_CFG_T cfg =
  {
    /*.log_level*/   AZX_LOG_LEVEL_DEBUG,
    /*.log_channel*/ AZX_LOG_TO_USB1,
    /*.log_colours*/ 0
  };
  
  
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(2000) );

  azx_tasks_init();


  azx_log_init(&cfg);\
  AZX_LOG_INFO("Starting AT tunnel demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  init_uart();
  
  
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(300000) ); /*wait 5 minutes*/
  
  at_cmd_async_deinit(ATI_ID);
}
