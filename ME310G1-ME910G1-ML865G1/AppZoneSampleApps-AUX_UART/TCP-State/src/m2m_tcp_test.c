/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    m2m_tcp_test.c

  @brief
    The file contains the TCP utilities

  @details

  @version 
    1.0.2

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


/* Local defines ================================================================================*/
#define SERVER "modules.telit.com"
#define SERVER_PORT 80

#define APN      "internetm2m.air.com"
#define PDP_CTX   (UINT8)3


static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;


M2MB_PDP_HANDLE pdpHandle;
M2MB_SOCKET_BSD_SOCKET sock_client = M2MB_SOCKET_BSD_INVALID_SOCKET;


/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/


/*
 * returns

 * */

/**
 * @brief Checks if the provided socket is still valid and has pending data
 *
 * This function will check if a socket is connected, has had errors or has  data available.
 * it will wait at most for timeout milliseconds (the select timeout)
 *
 *
 * @param[in] socket the already connected TCP socket handle
 * @param[out] status the variable to be filled with the operation result detail
 * @param[in] timeout the timeout in milliseconds for the internal m2mb_socket_bsd_select operation
 *
 * @return
 *   0 with status < 0 in case of socket failure
 *   0 with status = 0 if socket received a a FIN
 *   0 with status > 0 if data is available on socket (the amount is == status)
 *   a negative value in case of failure
 *   1 if socket does not have any event (so retry later)
 *
 * @see azx_log_init()
 * @see AZX_LOG_LEVEL_E
 */
INT32 adv_select(M2MB_SOCKET_BSD_SOCKET socket, INT32 *status, UINT32 timeout)
{
  /* Prepare set */
  M2MB_SOCKET_BSD_FD_SET_T read;
  M2MB_SOCKET_BSD_FD_SET_T err;
  struct M2MB_SOCKET_BSD_TIMEVAL t;
  INT32 result;
  INT32 pending;
  INT32 optlen=sizeof(int);

  *status = 0;

  m2mb_socket_bsd_fd_zero_func(&read);
  m2mb_socket_bsd_fd_zero_func(&err);
  m2mb_socket_bsd_fd_set_func(socket, &read);
  m2mb_socket_bsd_fd_set_func(socket, &err);

  /* Wait for data */
  t.m_tv_sec = timeout / 1000;
  t.m_tv_usec = (timeout % 1000) * 1000;

  do
  {
    result = m2mb_socket_bsd_select(socket + 1, &read, NULL, &err, &t);

    if(result < 0)
    {
      AZX_LOG_ERROR("Failure in m2mb_socket_bsd_select. Error <%d> \r\n", result);
      result = -1;
      break;
    }

    /*Check if the socket is in the ERROR descriptors*/
    result = m2mb_socket_bsd_fd_isset_func(socket, &err);
    if(result == 1)
    {
      AZX_LOG_DEBUG("Error <%d> on socket <%d>\r\n", m2mb_socket_errno(), socket);
      *status = -1;
      result = 0; //success, even on "failure"
      break;
    }
    else if(result < 0)
    {
      AZX_LOG_ERROR("Error on m2mb_socket_bsd_fd_isset_func reading <error> file descriptor\r\n");
      result = -2;
      break;
    }
    else
    {
      AZX_LOG_TRACE("Socket not in err fd (result =%d)\r\n", result);
    }

    /*Check if the socket is in the READ descriptor. if so, some data have been received*/
    result = m2mb_socket_bsd_fd_isset_func(socket, &read);

    if(1 == result)
    {
      AZX_LOG_DEBUG("Data is available on socket <%p>\r\n", socket);

      result = m2mb_socket_bsd_get_sock_opt(socket, M2MB_SOCKET_BSD_SOL_SOCKET, M2MB_SOCKET_BSD_SO_RXDATA, (void*) &pending, &optlen );

      if(0 == result)
      {
        AZX_LOG_DEBUG("There are <%d> pending bytes on socket\r\n", pending);

        if (pending == 0) /* a FIN was received */
        {
          *status = 0;
        }
        else /* payload is actually available */
        {
          *status = pending;
        }
        result = 0; /*SUCCESS*/
      }
      else
      {
        AZX_LOG_ERROR("Failure in m2mb_socket_bsd_get_sock_opt\r\n");
        result = -3;
        break;
      }
    }
    else if(result < 0)
    {
      AZX_LOG_ERROR("Error on m2mb_socket_bsd_fd_isset_func reading <read> file descriptor\r\n");
      result = -4;
      break;
    }
    else
    {
      result = 1;
      break;
    }

  } while (0);

  return result;
}


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
      AZX_LOG_TRACE("Module is registered to cell 0x%X!\r\n", stat_info->cellID);
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

    AZX_LOG_DEBUG("Creating Socket...\r\n");

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
      AZX_LOG_DEBUG("Retrieved IP: %s \r\n", ip_addr);
    }

    stSockAddr.sin_family = M2MB_SOCKET_BSD_PF_INET;
    stSockAddr.sin_port = m2mb_socket_bsd_htons(SERVER_PORT);

    if( 0 != m2mb_socket_bsd_connect(sock_client, (struct M2MB_SOCKET_BSD_SOCKADDR*)&stSockAddr,
        sizeof(struct M2MB_SOCKET_BSD_SOCKADDR_IN)))
    {
      AZX_LOG_ERROR("Cannot connect socket!\r\n");
      task_status = APPLICATION_EXIT;
      break;

    }

    AZX_LOG_DEBUG("Socket Connected!\r\n");


    {
      INT32 status = 0;
      int err =0;

      while (1)
      {
        err = adv_select(sock_client, &status, 2000 /*wait at most 2000 ms*/ );
        if (err < 0)
        {
          AZX_LOG_ERROR("Failed adv_select: %d\n", err);
          ret = -1;
          break;
        }
        else if (err == 1)
        {
          AZX_LOG_DEBUG("Socket does not have any event, try again...\r\n");
        }
        else // err == 0
        {
          /* Process incoming data if available */
          if(status == 0)
          {
            //SOCKET CLOSED BY REMOTE
            AZX_LOG_INFO("Socket was closed by remote!\r\n");
            break;
          }
          else if (status < 0)
          {
            /*error in socket.*/
            AZX_LOG_ERROR("There was an error on socket");
          }
          else /*status > 0*/
          {
            /* get available bytes and use it*/
            AZX_LOG_DEBUG("there are %d available bytes in the socket\r\n", status);
          }
        }
      }

    }

    task_status = APPLICATION_EXIT;
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

    AZX_LOG_DEBUG("Application complete.\r\n");
  }

  return 0;
}
