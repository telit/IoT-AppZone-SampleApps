/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing how to send data from main UART to a connected TCP server. Debug messages are printed on AUX UART port.

  @version 
    1.0.1
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
#include <stdarg.h>

#include "m2mb_types.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"

#include "m2mb_uart.h"

#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"


/* Local defines ================================================================================*/

#define APN          "internetm2m.air.com"
#define SERVER       "modules.telit.com"
#define SERVER_PORT  10510

#define PDP_CTX   (UINT8)3

#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define EV_PDP_BIT         (UINT32)0x2    /*0x0000000000000010*/

#define APPLICATION_EXIT -1
/* Global variables =============================================================================*/

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

M2MB_PDP_HANDLE pdpHandle;
M2MB_SOCKET_BSD_SOCKET sock_client = M2MB_SOCKET_BSD_INVALID_SOCKET;

INT32 main_uart_fd = -1;

INT32 uart_task_id = -1;
INT32 socket_recv_id = -1;


CHAR inBuf[1024];



/* Global functions =============================================================================*/

int get_host_ip_by_name(const CHAR* host)
{
  UINT32 r_addr = 0;
  struct M2MB_SOCKET_BSD_HOSTENT *phe;
  struct M2MB_SOCKET_BSD_HOSTENT he;
  char tmpbuf[1024];
  int herr;
  if ( ( ( m2mb_socket_bsd_get_host_by_name_2_r_cid( host, M2MB_SOCKET_BSD_AF_INET,  &he, tmpbuf, (SIZE_T) 1024, &phe, (INT32*) &herr, PDP_CTX) ) != 0 ) ||
      ( phe == NULL ) )
  {
    return 0;
  }
  else
  {
    memcpy((char *)&r_addr, phe->h_addr_list[0], phe->h_length);
    return r_addr;
  }
}

INT32 get_pending_bytes(M2MB_SOCKET_BSD_SOCKET s, INT32 *pending)
{
  INT32 optlen=sizeof(int);
  int ret;
  ret = m2mb_socket_bsd_get_sock_opt( s, M2MB_SOCKET_BSD_SOL_SOCKET, M2MB_SOCKET_BSD_SO_RXDATA, (void*) pending, &optlen );
  if( 0 != ret)
  {
    return -1;
  }
  else
  {
    return 0;
  }

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
    azx_tasks_sendMessageToTask( uart_task_id, 1, 0, (INT32)myUartRxLen );
  }
}



INT32 M2M_msgUARTTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)param1;
  
  INT32 read, sent;
  switch (type)
  {
  case 1:
    memset(inBuf, 0, sizeof(inBuf));
    AZX_LOG_DEBUG("Received data on uart, read it and send to socket!\r\n");
    read = m2mb_uart_read( main_uart_fd, inBuf, (SIZE_T)param2);
    if(read == -1)
    {
      AZX_LOG_ERROR("failed reading from uart!\r\n");
      return -1;
    }
    else
    {
      AZX_LOG_INFO("UART IN: <%.*s>. Sending to socket...\r\n", read, inBuf);
      sent = m2mb_socket_bsd_send(sock_client, inBuf, read, 0);
      if (sent == -1)
      {
        AZX_LOG_ERROR("failed sending to socket!\r\n");
        return -1;
      }
      else
      {
        AZX_LOG_INFO("Data sent to socket!\r\n");

        //test: try to read echo from server....
        azx_tasks_sendMessageToTask( socket_recv_id, 0, sent, 0 );
      }
    }
    break;


  default:
      AZX_LOG_WARN("Unexpected: %d..\r\n", type);
    break;
  }
  return 0;
}


#define MIN(a,b) (((a)<(b))?(a):(b))
INT32 socket_msgRecvTask(INT32 type, INT32 sent, INT32 param2)
{
  (void)type;
  (void)param2;
  
  INT32 read;
  INT32 pending = 0;
  INT32 res;
  INT32 timeout = 10000; //10 seconds
  char myBuf[512];

  AZX_LOG_DEBUG("Test, read echo from server! Last time %d bytes were sent\r\n", sent);

  res = 0;
  pending = 0;
  while(res == 0 && pending == 0 && timeout > 0)
  {
    res = get_pending_bytes(sock_client, &pending);
    AZX_LOG_DEBUG("Pending: %d\r\n", pending);

    azx_sleep_ms(500); //wait for a while;
    timeout -= 500;
  }
  if (res != 0)
  {
    AZX_LOG_ERROR("failed getting pending bytes!\r\n");
    return -1;
  }

  if (timeout == 0)
  {
    AZX_LOG_ERROR("data timeout!\r\n");
    return -1;
  }

  read = m2mb_socket_bsd_recv(sock_client, myBuf, MIN(sent, (INT32)sizeof(myBuf)), 0);
  if (read == -1)
  {
    AZX_LOG_ERROR("failed reading from socket!\r\n");
    return -1;
  }
  else
  {
    AZX_LOG_INFO("Response from server (%d bytes): <%.*s>\r\n", read, read, myBuf);

    m2mb_uart_write( main_uart_fd, "<<<", 3);
    m2mb_uart_write( main_uart_fd, (const void *) myBuf, read);
    m2mb_uart_write( main_uart_fd, "\r\n", 2);
  }
  return 0;
}


void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
  (void)resp_size;
  (void)myUserdata;

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
    if  (stat_info->stat == 1 || stat_info->stat == 5)
    {
      AZX_LOG_DEBUG("Module is registered to cell 0x%X!\r\n", stat_info->cellID);
      m2mb_os_ev_set(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      m2mb_net_get_reg_status_info(h); //call it again
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
    AZX_LOG_DEBUG( "IP address: %s\r\n", CBtmpIPaddr);

    m2mb_os_ev_set(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_SET);
    break;

  case M2MB_PDP_DOWN:
    AZX_LOG_DEBUG ("Context successfully deactivated!\r\n");
    break;
  default:
    AZX_LOG_DEBUG("unexpected pdp_event: %d\r\n", pdp_event);
    break;

  }
}

INT32 socket_init(void)
{
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

  M2MB_NET_HANDLE h;
  
  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;

  struct M2MB_SOCKET_BSD_SOCKADDR_IN stSockAddr;

  char ip_addr[64];
  int ret;
  int task_status;

  CHAR apn[32], apnUser[16], apnPwd[16];

  void *myUserdata = NULL;

  do
  {
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


    /* First step: check network registration and activate PDP context */

    retVal = m2mb_net_init(&h, NetCallback, myUserdata);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_DEBUG( "m2mb_net_init returned M2MB_RESULT_SUCCESS\r\n");
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_net_init not returned M2MB_RESULT_SUCCESS\r\n" );
    }

    AZX_LOG_INFO("Waiting for registration...\r\n");

    retVal = m2mb_net_get_reg_status_info(h);
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
    }

    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);


    AZX_LOG_DEBUG("Pdp context activation\r\n");
    retVal = m2mb_pdp_init(&pdpHandle, PdpCallback, myUserdata);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_DEBUG( "m2mb_pdp_init returned M2MB_RESULT_SUCCESS\r\n");
    }
    else
    {
      AZX_LOG_DEBUG( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
    }

    azx_sleep_ms(2000);

    memset( apn, 0x00, sizeof(apn));
    memset( apnUser, 0x00, sizeof(apnUser) );
    memset( apnPwd, 0x00, sizeof(apnPwd) );

    strcat( apn, APN );

    AZX_LOG_INFO("Activate PDP with APN %s....\r\n", apn);
    retVal = m2mb_pdp_activate(pdpHandle, PDP_CTX, apn, apnUser, apnPwd, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("cannot activate pdp context. Trying deactivating and reactivating again\r\n");
      m2mb_pdp_deactivate(pdpHandle, PDP_CTX);
      azx_sleep_ms(1000);
      retVal = m2mb_pdp_activate(pdpHandle, PDP_CTX, apn, apnUser, apnPwd, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
      if ( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("cannot activate pdp context. Quitting...\r\n");
        return -1;
      }
    }

    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

    AZX_LOG_DEBUG("Creating Socket...\r\n");

    sock_client = m2mb_socket_bsd_socket(M2MB_SOCKET_BSD_AF_INET, M2MB_SOCKET_BSD_SOCK_STREAM, M2MB_SOCKET_BSD_IPPROTO_TCP);
    if (M2MB_SOCKET_BSD_INVALID_SOCKET == sock_client)
    {
      AZX_LOG_DEBUG("TCP Client Error\r\n");
      return 1;

    }
    else
    {
      AZX_LOG_INFO("Socket created\r\n");
    }


    if ( m2mb_socket_set_cid( sock_client, PDP_CTX ) != 0 )
    {
      AZX_LOG_ERROR( "Socket not set to ctx: %d\r\n", PDP_CTX );
      return -1;
    }

    AZX_LOG_DEBUG( "Socket ctx set to %d\r\n", PDP_CTX );

    memset(&stSockAddr, 0, sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN));

    if (0 == (stSockAddr.sin_addr.s_addr= get_host_ip_by_name(SERVER)))
    {
      AZX_LOG_ERROR("Cannot retrieve IP\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }
    else
    {
      m2mb_socket_bsd_inet_ntop(M2MB_SOCKET_BSD_AF_INET, (const void*) &(stSockAddr.sin_addr.s_addr), ip_addr, sizeof(ip_addr));
      AZX_LOG_INFO("Server IP address: %s \r\n", ip_addr);
    }

    stSockAddr.sin_family = M2MB_SOCKET_BSD_PF_INET;
    stSockAddr.sin_port = m2mb_socket_bsd_htons(SERVER_PORT);

    if( 0 != m2mb_socket_bsd_connect(sock_client, (struct M2MB_SOCKET_BSD_SOCKADDR*)&stSockAddr,
        sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN)))
    {
      task_status = APPLICATION_EXIT;
      break;

    }

    AZX_LOG_INFO("Socket Connected and ready to receive data!\r\n");
    break;

  } while (0);


  if (task_status == APPLICATION_EXIT)
  {
    AZX_LOG_DEBUG("application exit\r\n");
    m2mb_socket_bsd_close( sock_client );

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
    return -1;
  }
  else
  {
    return 0; //success
  }
}

int init_uart(void)
{

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

    uart_task_id = azx_tasks_createTask((char*) "uart_task", AZX_TASKS_STACK_XL, 5, AZX_TASKS_MBOX_S, M2M_msgUARTTask);

    AZX_LOG_TRACE("Task ID: %d.\r\n", uart_task_id);

    if (uart_task_id <= 0)
    {
      AZX_LOG_ERROR("cannot create uart task!\r\n");
      return -1;
    }

//for debug purposes:
    socket_recv_id = azx_tasks_createTask((char*) "socket_recv_t", AZX_TASKS_STACK_L, 8, AZX_TASKS_MBOX_S, socket_msgRecvTask);

    AZX_LOG_TRACE("Task ID: %d.\r\n", socket_recv_id);

    if (socket_recv_id <= 0)
    {
      AZX_LOG_ERROR("cannot create socket task!\r\n");
      return -1;
    }


    return 0;
  }
  else
  {
    return -1;
  }
}


/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;
  int res;
    
  AZX_LOG_CFG_T log_cfg;
  
  
  azx_tasks_init();

  azx_sleep_ms(3000);


  /*SET output channel */
/*Set log configuration */
  log_cfg.log_channel = LOG_CHANNEL; /* Defined in app_cfg.h */
  log_cfg.log_level = AZX_LOG_LEVEL_DEBUG; /*Enable debug messages*/
  log_cfg.log_colours = 0; /*Set to 1 to use coloured logs (not all terminals are compatible)*/


  azx_log_init(&log_cfg);

  
  AZX_LOG_INFO("Starting UART to Server demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);


  res = socket_init();
  if (res != 0)
  {
    AZX_LOG_ERROR("cannot connect socket, return..\r\n");
    return;
  }
  //open uart
  res = init_uart();
  if (res != 0)
  {
    AZX_LOG_ERROR("Cannot initialize uart, return..\r\n");
    return;
  }
  AZX_LOG_INFO("Waiting for data from uart.\r\n");
  memset(inBuf, 0, sizeof(inBuf));
  sprintf(inBuf,  "Ready to receive data and send to socket.\r\n");
  m2mb_uart_write( main_uart_fd, inBuf, strlen(inBuf));

}

