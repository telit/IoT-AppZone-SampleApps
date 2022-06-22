/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    m2mb_udp_test.c

  @brief
    The file contains the UDP utilities

  @details

  @version 
    1.0.0

  @note

  @author
    Roberta Galeazzo

  @date
    03/03/2022
*/
/* Include files ================================================================================*/


#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"


#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "m2mb_udp_test.h"

/* Local defines ================================================================================*/
#define MY_SERVER_LISTEN_PORT 	(UINT16)6500

#define APN	    "apn"  //to be filled with network operator APN
#define PDP_CTX 	(UINT8)1



/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;
static M2MB_PDP_HANDLE pdpHandle;

struct M2MB_SOCKET_BSD_SOCKADDR sock_addr;
static M2MB_SOCKET_BSD_SOCKET SockID;
struct M2MB_SOCKET_BSD_SOCKADDR_IN stSockAddrServer;
struct M2MB_SOCKET_BSD_SOCKADDR_IN tmpAddress;
M2MB_SOCKET_BSD_FD_SET_T pReadfds;
INT32 server_started = -1;
static INT32 connection_number=0;
M2MB_SOCKET_BSD_SOCKET ConnectFD;

static UINT32 count;

CHAR datasend[128], DataInBuf[4096];

CHAR CBtmpIPaddr[32];

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/


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

INT32 M2M_msgUDPTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;
  
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  
  M2MB_NET_HANDLE h;
  struct M2MB_SOCKET_BSD_TIMEVAL timeOutVal = {3, 0};  // 3 sec time out.
  
  struct M2MB_SOCKET_BSD_SOCKADDR_IN stSockAddr;

  int task_status;
  int ret;
  int numSock;
  INT32 addrlen;
  CHAR clientIPaddr[32];
  static int	toSend;

  CHAR apn[32], apnUser[16], apnPwd[16];

  void *myUserdata = NULL;
  int argp=1; //to set non blocking socket
  
  int restart_listening = 1;
  
  

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;
  

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

    AZX_LOG_DEBUG("Activate PDP with APN %s....\r\n", apn);
    retVal = m2mb_pdp_activate(pdpHandle, PDP_CTX, apn, apnUser, apnPwd, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("cannot activate pdp context.\r\n");
    }
    
    /*Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);


    AZX_LOG_INFO( "\r\n----    Start UDP server  Listen IP: %s   PORT: %d    ----\r\n\r\n",CBtmpIPaddr, MY_SERVER_LISTEN_PORT  );

    azx_sleep_ms(2000);

    AZX_LOG_DEBUG("Creating Socket...\r\n");

    SockID = m2mb_socket_bsd_socket(M2MB_SOCKET_BSD_AF_INET, M2MB_SOCKET_BSD_SOCK_DGRAM, M2MB_SOCKET_BSD_IPPROTO_UDP);
    if (M2MB_SOCKET_BSD_INVALID_SOCKET == SockID)
    {
      AZX_LOG_DEBUG("TCP Error\r\n");
      return 1;

    }
    else
    {
      AZX_LOG_DEBUG("Socket created\r\n");
      AZX_LOG_DEBUG ("m2mb_socket_bsd_socket(): valid socket ID [0x%X] - PASS\r\n", SockID);
    }

    azx_sleep_ms(1000);

    /* Put the TCP socket in non-blocking mode. */
    AZX_LOG_DEBUG("issuing m2m_socket_bsd_ioctl() to set non-blocking mode ...\r\n");
    if ( m2mb_socket_bsd_ioctl(SockID, M2MB_SOCKET_BSD_FIONBIO, &argp) != 0 )
    {
      AZX_LOG_ERROR("m2mb_socket_bsd_ioctl Error\r\n");
      return 1;
    }

    memset(&stSockAddrServer, 0, sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN));
    stSockAddrServer.sin_family = M2MB_SOCKET_BSD_PF_INET;
    stSockAddrServer.sin_port = m2mb_socket_bsd_htons(MY_SERVER_LISTEN_PORT);
    stSockAddrServer.sin_addr.s_addr = M2MB_SOCKET_BSD_INADDR_ANY;

    AZX_LOG_DEBUG("Binding Socket...\r\n");
    azx_sleep_ms(2000);
    ret = m2mb_socket_bsd_bind(SockID,(struct M2MB_SOCKET_BSD_SOCKADDR*)&stSockAddrServer,sizeof(struct M2MB_SOCKET_BSD_SOCKADDR));

    if (M2MB_SOCKET_BSD_INVALID_SOCKET == ret)
    {
      AZX_LOG_ERROR("Socket Bind Error\r\n");
      return 1;

    }
    else
    {
      AZX_LOG_DEBUG("Socket Bind Pass\r\n");
    }

    azx_sleep_ms(1000);

    AZX_LOG_INFO("\r\nWaiting for Incoming UDP data, Listen IP: %s PORT: %d \r\n\r\n", CBtmpIPaddr, MY_SERVER_LISTEN_PORT);

    while(restart_listening)
    {
      /*Waiting for connections on listening socket... */
      count = 0;
      m2mb_socket_bsd_fd_zero_func(&pReadfds);
      numSock = 0;
      while (!numSock)
      {
        azx_sleep_ms(1000);
        m2mb_socket_bsd_fd_set_func( SockID, &pReadfds );

        AZX_LOG_DEBUG("select...\r\n");
        numSock = m2mb_socket_bsd_select(M2MB_SOCKET_BSD_FD_SETSIZE, &pReadfds, NULL, NULL, &timeOutVal);

        AZX_LOG_DEBUG("Select result: %d\r\n", numSock);
        m2mb_socket_bsd_fd_isset_func( SockID, &pReadfds );
        if (numSock > 0)
        {
          AZX_LOG_DEBUG("\r\nIncoming UDP data available");
        }
        else
        {
          //only for print purposes
          count++;
          if (count == 5)
          {
            count = 0;
            AZX_LOG_INFO("\r\nWaiting for Incoming UDP data, Listen IP: %s PORT: %d \r\n\r\n", CBtmpIPaddr, MY_SERVER_LISTEN_PORT);
          }
        }
      }

      azx_sleep_ms(1000);

      AZX_LOG_INFO( "\r\n----    Receive/send data test    ----\r\n\r\n" );

      memset( DataInBuf, 0x00, sizeof(DataInBuf) );
      AZX_LOG_INFO("trying to receive bytes..\r\n");

      ret = m2mb_socket_bsd_recv_from(SockID, DataInBuf, sizeof(DataInBuf), 0, (struct M2MB_SOCKET_BSD_SOCKADDR *)&stSockAddr, &addrlen);
	  if (ret ==-1)
	  {
	    AZX_LOG_ERROR("CANNOT RECEIVE DATA OVER SOCKET, errno: %d\r\n", m2mb_socket_errno());
	    task_status = APPLICATION_EXIT;
	    break;
	  }
	  else
	  {
	    m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET, &stSockAddr.sin_addr.s_addr, ( CHAR * )&( clientIPaddr ), sizeof( clientIPaddr ) );
	    AZX_LOG_DEBUG("Client Source Address: %s\r\n", clientIPaddr );
	    AZX_LOG_DEBUG("Client Port: %d\r\n", m2mb_socket_bsd_ntohs(stSockAddr.sin_port));
	    AZX_LOG_DEBUG("Client Family: %d\r\n",stSockAddr.sin_family );

	    AZX_LOG_INFO("Data received (%d): <%s>\r\n", ret, DataInBuf );

	  }


	  memset(datasend, 0, sizeof(datasend));
	  strcpy(datasend, "hello from m2mb!");
	  toSend = strlen(datasend);

	  ret = m2mb_socket_bsd_send_to(SockID, datasend, toSend, 0,(struct M2MB_SOCKET_BSD_SOCKADDR*)&stSockAddr,sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN));
	  if (ret ==-1)
	  {
	    AZX_LOG_ERROR("CANNOT SEND DATA OVER SOCKET, Error: %d\r\n", m2mb_socket_errno());
	    task_status = APPLICATION_EXIT;
	    break;
	  }
	  else
	  {
	   if (ret == toSend)
	    {
	      AZX_LOG_INFO("Data successfully sent (%d bytes)\r\n", toSend);
	    }
	    else
	    {
	      AZX_LOG_ERROR("Only %d bytes sent\r\n", ret);
	      task_status = APPLICATION_EXIT;
	      break;
	    }
	  }

	  connection_number++;
	  if ( connection_number == 2 )
	  {

	    restart_listening = 0;
	    task_status = APPLICATION_EXIT;
	    break;
	  }

    }


  } while (0);

  if (task_status == APPLICATION_EXIT)
  {
    AZX_LOG_DEBUG("application exit\r\n");

    m2mb_socket_bsd_close( SockID );

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

    AZX_LOG_DEBUG("Application complete.\r\n");
  }
  return 0;
}
