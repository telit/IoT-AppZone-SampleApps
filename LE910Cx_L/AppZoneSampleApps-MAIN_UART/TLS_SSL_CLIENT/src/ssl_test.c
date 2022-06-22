/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ssl_test.c

  @brief
    The file contains the TLS/SSL utilities

  @details

  @version
    1.1.8
  @note


  @author


  @date
    09/04/2018
*/
/* Include files ================================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "m2mb_types.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"

#include "m2mb_fs_posix.h"


#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"

#include "m2mb_ssl.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "ssl_test.h"

#include "app_cfg.h" /*FOR LOCALPATH define*/

/* Local defines ================================================================================*/

/* 0: test using HTTPS server with client authentication (port 20443)
   1: test using HTTPS server on port 443 with server authentication;
*/
#define HTTP_443 1

#if HTTP_443 //https with server authentication only
  #define SERVER_PORT 443 //https
  #define SERVER "modules.telit.com"
  #define CA_CERT_PATH LOCALPATH "/ssl_certs/modulesCA.crt"
  #define CLIENT_CERT_PATH ""
  #define CLIENT_KEY_PATH ""
  CHAR  queryBuf[] = "GET / HTTP/1.1\r\nHost: modules.telit.com\r\n\r\n";;
  M2MB_SSL_AUTH_TYPE_E SSL_AUTH_MODE  = M2MB_SSL_SERVER_AUTH;
#else //https with client cert
  #define SERVER "modules.telit.com"
  #define CA_CERT_PATH LOCALPATH "/ssl_certs/modulesCA.crt"

  #define CLIENT_CERT_PATH LOCALPATH "/ssl_certs/modulesClient.crt"
  #define CLIENT_KEY_PATH LOCALPATH "/ssl_certs/modulesClient_pkcs1.key"  //Only RSA Private keys are supported

  #define SERVER_PORT 20443 //echo client+server
  M2MB_SSL_AUTH_TYPE_E SSL_AUTH_MODE  = M2MB_SSL_SERVER_CLIENT_AUTH;
  CHAR  queryBuf[] = "GET / HTTP/1.1\r\nHost: modules.telit.com\r\n\r\n";
#endif


#define APN      "web.omnitel.it"

#define PDP_CTX   (UINT8)1

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

M2MB_PDP_HANDLE pdpHandle;
M2MB_SOCKET_BSD_SOCKET sock_client = M2MB_SOCKET_BSD_INVALID_SOCKET;

M2MB_SSL_CONFIG_T sslConfig;
M2MB_SSL_CONNECTION_HANDLE sslConnHndl;
M2MB_SSL_CIPHER_SUITE_E CipherSuite[M2MB_SSL_MAX_CIPHERSUITES];
M2MB_SSL_CONFIG_HANDLE sslConfigHndl;

M2MB_SSL_CTXT_HANDLE sslCtxtHndl;

M2MB_SSL_CA_INFO_T ca_Info[M2MB_SSL_MAX_CA_LIST];
UINT8 CA_BUF[2048];
UINT8 CLIENT_CERT_BUF[2048];
UINT8 CLIENT_KEY_BUF[2048];


CHAR  respBuf[2048];

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



    case M2MB_NET_GET_REG_STATUS_INFO_RESP:
      stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
      if  (stat_info->stat == 1 || stat_info->stat == 5)
      {
        AZX_LOG_DEBUG("Module is registered to cell 0x%X!\r\n", stat_info->cellID);
        m2mb_os_ev_set(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
      }
      else
      {
        m2mb_net_get_reg_status_info(h); //try again
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
      AZX_LOG_DEBUG ("Context deactivated!\r\n");
      break;
    default:
      AZX_LOG_DEBUG("unexpected pdp_event: %d\r\n", pdp_event);
      break;

  }
}

INT32 msgHTTPSTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;

  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;

  M2MB_NET_HANDLE h;

  struct M2MB_SOCKET_BSD_SOCKADDR_IN stSockAddr;

  char ip_addr[64];
  int ret;
  int task_status;

  CHAR apn[32], apnUser[16], apnPwd[16];

  //M2MB_SSL_PROTOCOL_VERSION_E ProtVers;
  //M2MB_SSL_AUTH_TYPE_E AuthType;
  INT32 sslRes;
  INT32 len;
  INT32 fs_res;
  INT32 fd = -1;
  struct M2MB_STAT st;
  INT32 pending;

  M2MB_SSL_SEC_INFO_U sslCertInfo;

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

    /* First step: Init SSL and load certificates */
    AZX_LOG_DEBUG("Init SSL session test app\r\n");

    sslConfig.ProtVers = M2MB_SSL_PROTOCOL_TLS_1_2;

    sslConfig.CipherSuites = CipherSuite; //(M2MB_SSL_CIPHER_SUITE_E *)malloc( 4*sizeof( M2MB_SSL_CIPHER_SUITE_E ) );

    sslConfig.CipherSuites[0] = M2MB_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256;
    sslConfig.CipherSuites[1] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA;
    sslConfig.CipherSuites[2] = M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA;
    sslConfig.CipherSuites[3] = M2MB_TLS_DHE_RSA_WITH_AES_256_CBC_SHA;
    sslConfig.CipherSuites[4] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA;
    sslConfig.CipherSuites[5] = M2MB_TLS_RSA_WITH_AES_128_CBC_SHA256;
    sslConfig.CipherSuites[6] = M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256;
    sslConfig.CipherSuites[7] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA256;
    sslConfig.CipherSuitesNum = 8;

    sslConfig.AuthType = SSL_AUTH_MODE;
    sslConfigHndl = m2mb_ssl_create_config( sslConfig,&sslRes );

    AZX_LOG_DEBUG("m2mb_ssl_create_config sslConfigHndl = %p, sslRes= %d \r\n",sslConfigHndl,sslRes );

    if( sslConfigHndl == NULL )
    {
      AZX_LOG_ERROR("m2mb_ssl_create_config FAILED error %d \r\n",sslRes );
      return -1;
    }
    else
    {
      AZX_LOG_DEBUG("m2mb_ssl_create_config PASSED \r\n");
    }

    sslRes = m2mb_ssl_config( sslConfigHndl, M2MB_SSL_NAME_SNI, (void*)SERVER );
    if(sslRes != 0)
    {
      AZX_LOG_ERROR("m2mb_ssl_config SNI failed\r\n");
    }
    else
    {
      AZX_LOG_DEBUG("m2mb_ssl_config SNI succeeded\r\n");
    }

    sslCtxtHndl = m2mb_ssl_create_ctxt();

    if( sslCtxtHndl == NULL )
    {
      AZX_LOG_ERROR("m2mb_ssl_create_ctxt FAILED error \r\n" );
      return -1 ;
    }
    else
    {
      AZX_LOG_DEBUG("m2mb_ssl_create_ctxt PASSED \r\n");
    }

    if(SSL_AUTH_MODE == M2MB_SSL_SERVER_AUTH || SSL_AUTH_MODE == M2MB_SSL_SERVER_CLIENT_AUTH)
    {
      AZX_LOG_DEBUG("loading CA CERT from file %s\r\n", CA_CERT_PATH);

      if (0 ==m2mb_fs_stat(CA_CERT_PATH, &st))
      {
        AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);

      }

      fd = m2mb_fs_open(CA_CERT_PATH,
                M2MB_O_RDONLY   /*open in read only mode*/
                );
      if (fd == -1 )
      {
        AZX_LOG_ERROR("Cannot open file\r\n");
        return  -1;
      }

      AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
      fs_res = m2mb_fs_read(fd, CA_BUF, st.st_size);

      if (fs_res != (INT32)st.st_size)
      {
        AZX_LOG_ERROR("Failed reading buffer into file.\r\n");
      }
      else
      {
        AZX_LOG_INFO("Buffer successfully received from file. %d bytes were loaded.\r\n", fs_res);
      }

      AZX_LOG_INFO("Closing file.\r\n");
      m2mb_fs_close(fd);


      //-----------------------------------------------------------------------


      sslCertInfo.ca_List.ca_Cnt = 1;
      sslCertInfo.ca_List.ca_Info[0] = &ca_Info[0];
      sslCertInfo.ca_List.ca_Info[0]->ca_Size =  st.st_size;
      sslCertInfo.ca_List.ca_Info[0]->ca_Buf = CA_BUF;

      m2mb_ssl_cert_store( M2MB_SSL_CACERT,sslCertInfo,(CHAR*) "CAListTest" );
      m2mb_ssl_cert_load( sslCtxtHndl,M2MB_SSL_CACERT,(CHAR*) "CAListTest" );
    }


    if (SSL_AUTH_MODE == M2MB_SSL_SERVER_CLIENT_AUTH) //if client cert, load also certificate and key
    {

      /*Load certificate for client*/
      AZX_LOG_DEBUG("loading client CERT from file %s\r\n", CLIENT_CERT_PATH);

      if (0 ==m2mb_fs_stat(CLIENT_CERT_PATH, &st))
      {
        AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);

      }

      fd = m2mb_fs_open(CLIENT_CERT_PATH,
                M2MB_O_RDONLY   /*open in read only mode*/
                );
      if (fd == -1 )
      {
        AZX_LOG_ERROR("Cannot open file\r\n");
        return  -1;
      }

      AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
      fs_res = m2mb_fs_read(fd, CLIENT_CERT_BUF, st.st_size);

      if (fs_res != (INT32)st.st_size)
      {
        AZX_LOG_ERROR("Failed reading buffer into file.\r\n");
      }
      else
      {
        AZX_LOG_INFO("Buffer successfully received from file. %d bytes were loaded.\r\n", fs_res);
      }

      AZX_LOG_INFO("Closing file.\r\n");
      m2mb_fs_close(fd);


      sslCertInfo.cert.cert_Buf = CLIENT_CERT_BUF;
      sslCertInfo.cert.cert_Size = st.st_size;


      /*Now load client key*/


      AZX_LOG_DEBUG("loading client KEY from file %s\r\n", CLIENT_KEY_PATH);

      if (0 ==m2mb_fs_stat(CLIENT_KEY_PATH, &st))
      {
        AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);

      }

      fd = m2mb_fs_open(CLIENT_KEY_PATH,
                M2MB_O_RDONLY   /*open in read only mode*/
                );
      if (fd == -1 )
      {
        AZX_LOG_ERROR("Cannot open file\r\n");
        return  -1;
      }

      AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
      fs_res = m2mb_fs_read(fd, CLIENT_KEY_BUF, st.st_size);

      if (fs_res != (INT32)st.st_size)
      {
        AZX_LOG_ERROR("Failed reading buffer into file.\r\n");
      }
      else
      {
        AZX_LOG_INFO("Buffer successfully received from file. %d bytes were loaded.\r\n", fs_res);
      }

      AZX_LOG_INFO("Closing file.\r\n");
      m2mb_fs_close(fd);


      sslCertInfo.cert.key_Buf = CLIENT_KEY_BUF;
      sslCertInfo.cert.key_Size = st.st_size;

      if (0 != m2mb_ssl_cert_store( M2MB_SSL_CERT,sslCertInfo,(CHAR*) "ClientCertTest" ))
      {
        AZX_LOG_ERROR("cannot store client certificate + key!\r\n");
        return -1;
      }
      if (0 != m2mb_ssl_cert_load( sslCtxtHndl,M2MB_SSL_CERT,(CHAR*) "ClientCertTest" ))
      {
        AZX_LOG_ERROR("cannot load client certificate + key into context!\r\n");
        return -1;
      }
    }

    if(SSL_AUTH_MODE == M2MB_SSL_SERVER_AUTH || SSL_AUTH_MODE == M2MB_SSL_SERVER_CLIENT_AUTH)
    {
      AZX_LOG_DEBUG("certificates successfully stored!\r\n");
    }

    /* Second step: Certificates were loaded, now check network registration and activate PDP context */

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
      task_status = APPLICATION_EXIT;
      break;

    }

    AZX_LOG_DEBUG("Socket Connected!\r\n");


    //-----------------------------------------------------------------------

    sslConnHndl = m2mb_ssl_secure_socket( sslConfigHndl,sslCtxtHndl,sock_client,&sslRes );

    if( sslConnHndl == 0 )
    {
       AZX_LOG_ERROR("m2mb_ssl_secure_socket FAILED error %d \r\n",sslRes );
     return -1;
    }
    else
    {
      sslRes = m2mb_ssl_connect( sslConnHndl );
    }

    if( sslRes != 0 )
    {
       AZX_LOG_ERROR("m2mb_ssl_connect FAILED error %d. Please verify module clock with AT+CCLK? command \r\n.",sslRes );
     return -1;
    }

    else
    {
       AZX_LOG_DEBUG("m2mb_ssl_connect ret %d \r\n",sslRes );
    }


    azx_sleep_ms(2000);

    AZX_LOG_DEBUG("Sending bytes..\r\n");
    sslRes = m2mb_ssl_write( sslConnHndl, queryBuf, sizeof( queryBuf ) );

    AZX_LOG_DEBUG("SSL write result = %d\r\n",sslRes );

    azx_sleep_ms(4000);
    ret = get_pending_bytes( sock_client, &pending );
    if( 0 != ret)
    {
      AZX_LOG_ERROR("cannot retrieve the pending bytes...ret: %d; errno: %d\r\n", ret, m2mb_socket_errno());
      task_status = APPLICATION_EXIT;
      break;
    }
    else
    {
      AZX_LOG_DEBUG("pending bytes: %i\r\n", pending);
    }
    if(pending >0)
    {
      AZX_LOG_DEBUG("trying to receive %d bytes..\r\n", pending);
      memset(respBuf,0, sizeof(respBuf));
      len = m2mb_ssl_read( sslConnHndl, respBuf, pending );

      if( len > 0 )
      {
        AZX_LOG_DEBUG("Server response: (%d)<%s>\r\n", len, respBuf );
      }
      else
      {
        AZX_LOG_DEBUG("Server response no data: (%d) \r\n", len );
      }

      ret = get_pending_bytes( sock_client, &pending );
      if( 0 != ret)
      {
        AZX_LOG_ERROR("cannot retrieve the pending bytes...ret: %d; errno: %d\r\n", ret, m2mb_socket_errno());
        task_status = APPLICATION_EXIT;
        break;
      }
      else
      {
        AZX_LOG_DEBUG("pending bytes: %i\r\n", pending);
      }
      if(pending >0)
      {
        AZX_LOG_DEBUG("trying to receive remaining %d bytes..\r\n", pending);
        memset(respBuf,0, sizeof(respBuf));
        len = m2mb_ssl_read( sslConnHndl, respBuf, pending );

        if( len > 0 )
        {
          AZX_LOG_DEBUG("Server response: (%d)<%s>\r\n", len, respBuf );
        }
        else
        {
          AZX_LOG_DEBUG("Server response no data: (%d) \r\n", len );
        }
      }
    }
    m2mb_ssl_shutdown( sslConnHndl );
    m2mb_socket_bsd_close( sock_client );
    task_status = APPLICATION_EXIT;
    break;

  } while (0);


  if (task_status == APPLICATION_EXIT)
  {
    AZX_LOG_DEBUG("application exit\r\n");


  m2mb_ssl_delete_config( sslConfigHndl );

  if(SSL_AUTH_MODE == M2MB_SSL_SERVER_AUTH || SSL_AUTH_MODE == M2MB_SSL_SERVER_CLIENT_AUTH)
  {
    m2mb_ssl_cert_delete( M2MB_SSL_CACERT, (CHAR*) "CAListTest" );
  }

  if(SSL_AUTH_MODE == M2MB_SSL_SERVER_CLIENT_AUTH)
  {
    m2mb_ssl_cert_delete( M2MB_SSL_CERT, (CHAR*) "ClientCertTest" );
  }

  m2mb_ssl_delete_ctxt( sslCtxtHndl );

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

