/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    m2m_tcp_test.c

  @brief
    The file contains the TCP utilities

  @details

  @version 
    1.0.6

  @note

  @author


  @date
    02/03/2019
*/
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"

#include "m2mb_fs_posix.h"

#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "m2m_tcp_test.h"

#include "read_parameters.h"

/* Macro =============================================================================*/

/* Local statics ================================================================================*/
static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;
static M2MB_PDP_HANDLE pdpHandle;

struct M2MB_SOCKET_BSD_SOCKADDR sock_addr;
static M2MB_SOCKET_BSD_SOCKET SockID;
struct M2MB_SOCKET_BSD_SOCKADDR_IN stSockAddrServer;
struct M2MB_SOCKET_BSD_SOCKADDR_IN tmpAddress;
M2MB_SOCKET_BSD_FD_SET_T pReadfds;
M2MB_SOCKET_BSD_SOCKET ConnectFD;

INT32 server_started = -1;
static INT32 connection_number = 0;
CHAR datasend[128], DataInBuf[4096];


/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

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
  if ( ( ( m2mb_socket_bsd_get_host_by_name_2_r_cid( host, M2MB_SOCKET_BSD_AF_INET,  &he, tmpbuf, (SIZE_T) 1024, &phe, (INT32*) &herr, gPDP_CTX) ) != 0 ) ||
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


static void checkNetStat(  M2MB_NET_REG_STATUS_T *stat_info)
{
  if  (stat_info->stat == 1 || stat_info->stat == 5)
  {
    AZX_LOG_DEBUG("Module is registered to cell 0x%X!\r\n", (unsigned int)stat_info->cellID);
    m2mb_os_ev_set(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
  }
  else
  {
    m2mb_os_ev_set(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_CLEAR);
  }
}

static void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
  UNUSED_3( h, resp_size, myUserdata);

  M2MB_NET_REG_STATUS_T *stat_info;

  switch (net_event)
  {
  case M2MB_NET_GET_REG_STATUS_INFO_RESP:
    stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
    checkNetStat(stat_info);
    break;

  case M2MB_NET_REG_STATUS_IND:
    stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
    AZX_LOG_DEBUG("Net Stat IND is %d, %d, %d, %d, %ld\r\n",
        stat_info->stat, stat_info->rat, stat_info->srvDomain,
        stat_info->areaCode, stat_info->cellID);
    checkNetStat(stat_info);
    break;

  default:
    AZX_LOG_TRACE("Unexpected net_event: %d\r\n", net_event);
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

INT32 M2M_msgTCPTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;

  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

  M2MB_NET_HANDLE h;
  struct M2MB_SOCKET_BSD_TIMEVAL timeOutVal = {3, 0};  // 3 sec time out.

  struct M2MB_SOCKET_BSD_SOCKADDR_IN stSockAddr;

  INT32 Res, j, ResTot;

  int task_status;
  int ret;
  int numSock;
  INT32 addrlen;
  CHAR clientIPaddr[32];

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

    configureParameters(); /*set default values first*/
    readConfigFromFile(); /*try to read configuration from file (if present)*/


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

    retVal = m2mb_net_enable_ind(h, M2MB_NET_REG_STATUS_IND, 1);
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_enable_ind failed\r\n" );
      return 1;
    }

    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY, &curEvBits, M2MB_OS_WAIT_FOREVER);



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

    strcat( apn, gAPN );
    strcat( apnUser, gAPN_UserName);
    strcat( apnPwd, gAPN_Password);

    AZX_LOG_DEBUG("Activate PDP with APN %s....\r\n", apn);
    retVal = m2mb_pdp_activate(pdpHandle, gPDP_CTX, apn, apnUser, apnPwd, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("cannot activate pdp context.\r\n");
    }
    
    /*Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);


    AZX_LOG_INFO( "\r\n\r\n--------------------------------------------\r\n" );
    AZX_LOG_INFO( "|                                               |\r\n" );
    AZX_LOG_INFO( "|  Start TCP server                             |\r\n"  );
    AZX_LOG_INFO( "|                                               |\r\n" );
    AZX_LOG_INFO( "--------------------------------------------\r\n\r\n\r\n" );

    azx_sleep_ms(2000);

    AZX_LOG_DEBUG("Creating Socket...\r\n");

    SockID = m2mb_socket_bsd_socket(M2MB_SOCKET_BSD_AF_INET, M2MB_SOCKET_BSD_SOCK_STREAM, M2MB_SOCKET_BSD_IPPROTO_TCP);
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
      AZX_LOG_DEBUG("m2mb_socket_bsd_ioctl Error\r\n");
      return 1;
    }

    memset(&stSockAddrServer, 0, sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN));
    stSockAddrServer.sin_family = M2MB_SOCKET_BSD_PF_INET;
    stSockAddrServer.sin_port = m2mb_socket_bsd_htons(gServer_Listen_Port);
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

    /*
     * Socket listen (#SL)
     */
    AZX_LOG_INFO( "\r\n\r\nStart TCP listening on port %d...\r\n\r\n", gServer_Listen_Port );

    if ( M2MB_SOCKET_BSD_INVALID_SOCKET == m2mb_socket_bsd_listen(SockID, 2) )
    {
      AZX_LOG_DEBUG("Server listen Error\r\n");
      return -1;
    }

    while(restart_listening)
    {
      /*Waiting for connections on listening socket... */
      m2mb_socket_bsd_fd_zero_func(&pReadfds);
      numSock = 0;
      while (!numSock)
      {
        azx_sleep_ms(1000);
        m2mb_socket_bsd_fd_set_func( SockID, &pReadfds );
        AZX_LOG_DEBUG("select...\r\n");
        numSock = m2mb_socket_bsd_select(M2MB_SOCKET_BSD_FD_SETSIZE, &pReadfds, NULL, NULL, &timeOutVal);

        AZX_LOG_INFO("Select result: %d\r\n", numSock);
        m2mb_socket_bsd_fd_isset_func( SockID, &pReadfds );
        if (numSock > 0)
        {
          AZX_LOG_INFO("\r\nTCP Server Coming Connection\r\n\r\n");
        }
      }

      azx_sleep_ms(1000);


      AZX_LOG_INFO("--> Accept\r\n");

      ConnectFD = m2mb_socket_bsd_accept(SockID,(struct M2MB_SOCKET_BSD_SOCKADDR *)&stSockAddr,  &addrlen);
      if (0 > ConnectFD)
      {
        AZX_LOG_DEBUG("Socket Accept Error\r\n");
        AZX_LOG_ERROR("Socket error number: %d\r\n", m2mb_socket_errno() );
        //
        m2mb_socket_bsd_close( SockID );
        azx_sleep_ms(3000);
        task_status = APPLICATION_EXIT;
        break;
      }
      else
      {
        AZX_LOG_DEBUG("Socket Accept Pass\r\n");
      }

      AZX_LOG_INFO("\r\n\r\nConnected! (socket dial n.%d)\r\n\r\n", connection_number+1 );

      m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET, &stSockAddr.sin_addr.s_addr, ( CHAR * )&( clientIPaddr ), sizeof( clientIPaddr ) );
      AZX_LOG_DEBUG("Client Source Address: %s\r\n", clientIPaddr );
      AZX_LOG_DEBUG("Client Port: %d\r\n", m2mb_socket_bsd_ntohs(stSockAddr.sin_port));
      AZX_LOG_DEBUG("Client Family: %d\r\n",stSockAddr.sin_family );

      azx_sleep_ms(1000);


      AZX_LOG_DEBUG( "\r\n-------------------------------\r\n" );
      AZX_LOG_DEBUG( "|    Send/receive data test    |\r\n" );
      AZX_LOG_DEBUG( "-------------------------------\r\n\r\n" );
      azx_sleep_ms(1000);

      // TX
      AZX_LOG_DEBUG("\r\n--> issuing m2mb_socket_bsd_send(): transmit \"__START____\" packet...\r\n");
      memset( datasend, 0x00, 128 );
      strcpy(datasend, "\r\nSTART\r\n\r\n");
      Res = m2mb_socket_bsd_send( ConnectFD, datasend, 11, 0) ;
      if ( Res == -1 )
        AZX_LOG_DEBUG("m2mb_socket_bsd_send() - FAIL\r\n", Res);
      else {
        AZX_LOG_DEBUG(" --> done (%d have been transmitted)\r\n", Res);
        if ( Res == 11 )
          AZX_LOG_DEBUG("     ALL data transmitted - PASS\r\n");
        else
          AZX_LOG_DEBUG("     NOT all data transmitted - FAIL\r\n");
      }

      AZX_LOG_DEBUG("\r\n--> issuing m2mb_socket_bsd_send(): transmit 58 bytes...\r\n");
      memset( datasend, 0x00, 128 );
      strcpy(datasend, "aaaaaaaaaa-bbbbbbbbbb-cccccccccc-dddddddddd-eeeeeeeeee\r\n\r\n");
      Res = m2mb_socket_bsd_send( ConnectFD, datasend, 58, 0) ;
      if ( Res == -1 )
        AZX_LOG_DEBUG("m2mb_socket_bsd_send() - FAIL\r\n", Res);
      else {
        AZX_LOG_DEBUG(" --> done (%d have been transmitted)\r\n", Res);
        if ( Res == 58 )
        {
          AZX_LOG_DEBUG("     ALL data transmitted - PASS\r\n");
        }
        else
        {
          AZX_LOG_DEBUG("     NOT all data transmitted - FAIL\r\n");
        }
      }

      AZX_LOG_DEBUG("\r\nWaiting for data...\r\n\r\n");
      ResTot=0;
      azx_sleep_ms(7000);
      memset( DataInBuf, 0x00, 4096 );
      for ( j=0; j<30; j++)
      {
        Res = m2mb_socket_bsd_recv( ConnectFD, (void*)DataInBuf, 30, 0 );
        if ( Res>0 )
        {
          AZX_LOG_DEBUG( "%s", DataInBuf );
          ResTot += Res;
          memset( DataInBuf, 0x00, 4096 );
        }
        // -1 is "false" error 35 = M2MB_SOCKET_BSD_EWOULDBLOCK (no incoming data)
        //if ( Res == -1 ) AZX_LOG_ERROR( "\r\n--> Socket error number: %d\r\n", m2mb_socket_errno() );
        azx_sleep_ms(2000);
      }
      AZX_LOG_DEBUG( "\r\nm2mb_socket_bsd_recv() has received %d bytes\r\n\r\n", ResTot );
      azx_sleep_ms(3000);


      AZX_LOG_DEBUG("\r\nServer TCP is closing the current connection ...\r\n\r\n");
      m2mb_socket_bsd_close( ConnectFD );

      connection_number++;
      if ( connection_number == 2 )
      {

        restart_listening = 0;
        task_status = APPLICATION_EXIT;

        break;
      }

    }

    task_status = APPLICATION_EXIT;
    break;

  } while (0);

  if (task_status == APPLICATION_EXIT)
  {
    AZX_LOG_DEBUG("application exit\r\n");

    m2mb_socket_bsd_close( SockID );

    ret = m2mb_pdp_deactivate(pdpHandle, gPDP_CTX);
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
