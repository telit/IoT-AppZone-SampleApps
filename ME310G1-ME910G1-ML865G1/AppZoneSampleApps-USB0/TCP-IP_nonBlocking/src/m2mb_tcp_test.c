/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    m2m_tcp_test.c

  @brief
    The file contains the TCP utilities
	
  @details
  
  @description
    Sample application that shows how to configure and connect a TCP-IP non blocking socket. Debug prints on $OUTPUT
  @version 
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
	Roberta Galeazzo

  @date
    04/04/2022
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

#include "m2mb_tcp_test.h"

#include "read_parameters.h"

/* Macro =============================================================================*/

/* Local defines ================================================================================*/

/** The number of milliseconds in one millisecond. */
#define OS_MILLISECONDS          (1)

/** The number of milliseconds in one second. */
#define OS_SECONDS               (1000 * OS_MILLISECONDS)

#define OPEN_SOCKET_TIMEOUT_MS (80 * OS_SECONDS) //default timeout is 80 seconds, 80000 ms


#define M2M_SOCKET_BSD_SOCKET M2MB_SOCKET_BSD_SOCKET
/* Local typedefs ===============================================================================*/
typedef enum
{
  M2M_SOCKET_EVENT_SOCKET_BREAK, /* Connection closed by the server */
  M2M_SOCKET_EVENT_SOCKET_FAIL, /* Connection error */
} M2M_NETWORK_EVENT;

/* Local statics ================================================================================*/
static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

static M2MB_PDP_HANDLE pdpHandle;
static M2MB_SOCKET_BSD_SOCKET sock_client = M2MB_SOCKET_BSD_INVALID_SOCKET;

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

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;

  struct M2MB_SOCKET_BSD_SOCKADDR_IN stSockAddr;
  char ip_addr[64];
  char send_buf[64];
  char recv_buf[64];
  int ret = 0;
  int is_connected = 0;
  int task_status = INIT;
  static int  toSend;

  CHAR apn[32], apnUser[16], apnPwd[16];

  void *myUserdata = NULL;

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

    retVal = m2mb_net_enable_ind(h, M2MB_NET_REG_STATUS_IND, 1);
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_enable_ind failed\r\n" );
      return 1;
    }

    AZX_LOG_DEBUG("Waiting for registration...\r\n");

    retVal = m2mb_net_get_reg_status_info(h);
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
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
      AZX_LOG_ERROR( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
    }

    azx_sleep_ms(2000);

    memset( apn, 0x00, sizeof(apn));
    memset( apnUser, 0x00, sizeof(apnUser) );
    memset( apnPwd, 0x00, sizeof(apnPwd) );

    strcat(apn, gAPN);
    strcat(apnUser, gAPN_UserName);
    strcat(apnPwd, gAPN_Password);

    AZX_LOG_DEBUG("Activate PDP with APN %s....\r\n", apn);
    retVal = m2mb_pdp_activate(pdpHandle, gPDP_CTX, apn, apnUser, apnPwd, M2MB_PDP_IPV4); /*activates PDP_CTX with APN and IP type IPV4*/
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("cannot activate pdp context.\r\n");
    }

    /*Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

    AZX_LOG_DEBUG("Creating Socket...\r\n");

    /*Create socket*/
    sock_client = m2mb_socket_bsd_socket(M2MB_SOCKET_BSD_AF_INET, M2MB_SOCKET_BSD_SOCK_STREAM, M2MB_SOCKET_BSD_IPPROTO_TCP);
    if (M2MB_SOCKET_BSD_INVALID_SOCKET == sock_client)
    {
      AZX_LOG_DEBUG("TCP Client Error\r\n");
      return 1;

    }
    else
    {
      AZX_LOG_DEBUG("Socket created\r\n");
    }

    /*Associate socket to CID defined in PDP_CTX*/
    if ( m2mb_socket_set_cid( sock_client, gPDP_CTX ) != 0 )
    {
      AZX_LOG_ERROR( "Socket not set to ctx: %d\r\n", gPDP_CTX );
      return -1;
    }

    AZX_LOG_DEBUG( "Socket ctx set to %d\r\n", gPDP_CTX );

    memset(&stSockAddr, 0, sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN));

    if (0 == (stSockAddr.sin_addr.s_addr= get_host_ip_by_name(gSERVER)))
    {
      AZX_LOG_ERROR("Cannot retrieve IP\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }
    else
    {
      m2mb_socket_bsd_inet_ntop(M2MB_SOCKET_BSD_AF_INET, (const void*) &(stSockAddr.sin_addr.s_addr), ip_addr, sizeof(ip_addr));
      AZX_LOG_DEBUG("Retrieved IP: %s \r\n", ip_addr);
    }

    stSockAddr.sin_family = M2MB_SOCKET_BSD_PF_INET;
    stSockAddr.sin_port = m2mb_socket_bsd_htons(gSERVER_PORT);


    INT32 nonBlock = 1;
    if(0 == m2mb_socket_bsd_ioctl(sock_client, M2MB_SOCKET_BSD_FIONBIO, &nonBlock)){

      AZX_LOG_DEBUG("Socket set to nonBlocking\r\n");

    } else {

      AZX_LOG_ERROR("Impossible to set non blocking socket!\r\n");
      return -1;

    }

    if( 0 != m2mb_socket_bsd_connect(sock_client, (struct M2MB_SOCKET_BSD_SOCKADDR*)&stSockAddr, /*Connect function exits immediately with M2MB_SOCKET_BSD_EINPROGRESS*/
            sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN)))
    {
      INT32 errCode = m2mb_socket_errno();
      if(errCode == M2MB_SOCKET_BSD_EINPROGRESS)
      {
        M2MB_SOCKET_BSD_FD_SET_T write;
        struct M2MB_SOCKET_BSD_TIMEVAL t;


        AZX_LOG_DEBUG("Connection in progress...\r\n");
        m2mb_socket_bsd_fd_zero_func(&write);

        m2mb_socket_bsd_fd_set_func(sock_client, &write);
        /* Wait for write availability */
        t.m_tv_sec = OPEN_SOCKET_TIMEOUT_MS / 1000;
        t.m_tv_usec = (OPEN_SOCKET_TIMEOUT_MS % 1000) * 1000;

        ret = m2mb_socket_bsd_select(sock_client + 1, NULL, &write, NULL, &t); /*function waits OPEN_SOCKET_TIMEOUT_MS timeout seconds, as soon as write flag is set socket is connected*/

        if(ret < 0)
        {
          AZX_LOG_ERROR("Failure in m2mb_socket_bsd_select. Error <%d> \r\n", ret);
          is_connected = -1;
          break;
        }
        else if(ret == 0)
        {
          AZX_LOG_WARN("Select timeout!\r\n");
          break;
        }

        /*Check if the socket is in the write descriptors*/
        ret = m2mb_socket_bsd_fd_isset_func(sock_client, &write);
        if(ret == 1)
        {
          AZX_LOG_DEBUG("Socket <%p> is connected!\r\n", sock_client);
          is_connected = 1;
        }
        else if(ret < 0)
        {
          AZX_LOG_ERROR("Error on m2mb_socket_bsd_fd_isset_func reading <write> file descriptor\r\n");
          is_connected = -2;
        }
        else
        {
          AZX_LOG_DEBUG("Socket not in write fd (result =%d)\r\n", ret);
        }
      }
      else
      {
        AZX_LOG_ERROR("errno:%d\r\n", errCode);
        return -1;
      }
    }

    /* is_connected value is assigned depending on the select return above */
    if(is_connected == 1)
    {
      AZX_LOG_DEBUG("Socket Connected!\r\n");
    }
    else
    {
      AZX_LOG_ERROR("Socket not connected after timeout!\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }
    memset(send_buf, 0, sizeof(send_buf));
    strcpy(send_buf, "hello from m2mb!");

    toSend = strlen(send_buf);

    AZX_LOG_DEBUG("Sending data over socket..\r\n");

    ret = m2mb_socket_bsd_send(sock_client, send_buf, toSend,0);
    if (ret == -1)
    {
      AZX_LOG_ERROR("CANNOT SEND DATA OVER SOCKET, Error: %d\r\n", m2mb_socket_errno());
      task_status = APPLICATION_EXIT;
      break;
    }
    else
    {
      if (ret == toSend)
      {
        AZX_LOG_DEBUG("Data send successfully (%d bytes)\r\n", toSend);

      }
      else
      {
        AZX_LOG_ERROR("Only %d bytes sent\r\n", ret);
        task_status = APPLICATION_EXIT;
        break;
      }
    }

    azx_sleep_ms(2000);
    memset(recv_buf, 0, sizeof(recv_buf));

    AZX_LOG_DEBUG("trying to receive %d bytes..\r\n", toSend);
    ret = m2mb_socket_bsd_recv(sock_client, recv_buf, toSend,0);
    if (ret ==-1)
    {
      AZX_LOG_ERROR("CANNOT RECEIVE DATA OVER SOCKET, errno: %d\r\n", m2mb_socket_errno());
    }
    else
    {
      AZX_LOG_DEBUG("Data received (%d): <%s>\r\n", ret, recv_buf );
    }

    task_status = APPLICATION_EXIT;
    break;

  } while (0);

  if (task_status == APPLICATION_EXIT)
  {
    AZX_LOG_DEBUG("application exit\r\n");

    m2mb_socket_bsd_close( sock_client );

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
