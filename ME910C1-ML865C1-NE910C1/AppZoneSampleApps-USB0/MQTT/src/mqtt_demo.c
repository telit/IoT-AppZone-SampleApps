/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    mqtt_demo.c

  @brief
    The file contains the MQTT utilities and logic

  @details

  @version
    1.0.7
  @note


  @author


  @date
    09/04/2018
*/
/* Include files ============================================================*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "m2mb_types.h"

#include "m2mb_os_api.h"

#include "m2mb_fs_posix.h"

#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"
#include "m2mb_ssl.h"

#include "m2mb_mqtt.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "mqtt_demo.h"

#include "read_parameters.h"
/* Local defines =====================================================================*/



/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;
static M2MB_PDP_HANDLE pdpHandle;

static M2MB_MQTT_HANDLE mqttHandle = NULL;


#ifdef REDUCED_CYPHER_LIST
M2MB_SSL_CIPHER_SUITE_E CipherSuite[M2MB_SSL_MAX_CIPHERSUITES];
static const M2MB_SSL_CIPHER_SUITE_E s_cipher_suite[] = {
  M2MB_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_RSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
  M2MB_TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_RSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_RSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_RSA_WITH_AES_256_CBC_SHA256,
};
#else
static const M2MB_SSL_CIPHER_SUITE_E s_cipher_suite[] = {
  M2MB_TLS_PSK_WITH_RC4_128_SHA,
  M2MB_TLS_PSK_WITH_3DES_EDE_CBC_SHA,
  M2MB_TLS_PSK_WITH_AES_128_CBC_SHA,
  M2MB_TLS_PSK_WITH_AES_256_CBC_SHA,
  M2MB_TLS_PSK_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_PSK_WITH_AES_256_GCM_SHA384,
  M2MB_TLS_PSK_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_PSK_WITH_AES_256_CBC_SHA384,
  M2MB_TLS_RSA_WITH_AES_128_CBC_SHA,
  M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA,
  M2MB_TLS_RSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_DHE_RSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_RSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_RSA_WITH_AES_256_CBC_SHA256,
  M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256,
  M2MB_TLS_RSA_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_RSA_WITH_AES_256_GCM_SHA384,
  M2MB_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384,
  M2MB_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA,
  M2MB_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA,
  M2MB_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA,
  M2MB_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA,
  M2MB_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA,
  M2MB_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384,
  M2MB_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384,
  M2MB_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384,
  M2MB_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256,
  M2MB_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384,
  M2MB_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384,
  M2MB_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384,
  M2MB_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
  M2MB_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256,
  M2MB_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384,
  M2MB_TLS_RSA_WITH_AES_128_CCM_8,
  M2MB_TLS_RSA_WITH_AES_256_CCM_8,
  M2MB_TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
  M2MB_TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256,
  M2MB_TLS_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256,
  M2MB_TLS_AES_128_GCM_SHA256,
  M2MB_TLS_AES_256_GCM_SHA384,
  M2MB_TLS_CHACHA20_POLY1305_SHA256,
  M2MB_TLS_AES_128_CCM_SHA256,
  M2MB_TLS_AES_128_CCM_8_SHA256,
};
#endif
/* Local function prototypes ====================================================================*/
static void mqtt_topic_cb( M2MB_MQTT_HANDLE handle, void *arg, const CHAR *topic,
        UINT16 topic_length, const CHAR *msg, UINT32 msg_length, M2MB_MQTT_RX_STATUS_E status );

static void CleanSSLEnvironment(M2MB_SSL_CONFIG_HANDLE* p_hSSLConfig, M2MB_SSL_CTXT_HANDLE* p_hSSLCtx, M2MB_SSL_AUTH_TYPE_E ssl_auth_mode);
static INT32 PrepareSSLEnvironment(M2MB_SSL_CONFIG_HANDLE* p_hSSLConfig, M2MB_SSL_CTXT_HANDLE* p_hSSLCtx, M2MB_SSL_AUTH_TYPE_E ssl_auth_mode);


/* Static functions =============================================================================*/


static void mqtt_topic_cb( M2MB_MQTT_HANDLE handle, void *arg, const CHAR *topic,
                           UINT16 topic_length, const CHAR *msg, UINT32 msg_length, M2MB_MQTT_RX_STATUS_E status )
{
  ( void ) handle;
  ( void ) arg;
  UINT8 buf[1024];

  memcpy( buf, topic, topic_length );
  buf[topic_length] = '\0'; /* Make sure its null terminated */
  AZX_LOG_DEBUG( "MQTT Message on Topic %s; data len: %d\r\n", buf, msg_length );

  memcpy( buf, msg, msg_length );
  buf[msg_length] = '\0'; /* Make sure its null terminated*/
  AZX_LOG_DEBUG( "Message: <%s>\r\n", buf );

  if( status == M2MB_MQTT_RX_MORE_DATA )
  {
    AZX_LOG_DEBUG( "More data to be received\r\n" );
  }
}


static void CleanSSLEnvironment(M2MB_SSL_CONFIG_HANDLE* p_hSSLConfig, M2MB_SSL_CTXT_HANDLE* p_hSSLCtx, M2MB_SSL_AUTH_TYPE_E ssl_auth_mode )
{

  INT32 res;

  if (p_hSSLConfig == NULL || p_hSSLCtx == NULL)
  {
    return;
  }

  if(ssl_auth_mode >= M2MB_SSL_SERVER_AUTH)
  {
    /* clean everything */
    res = m2mb_ssl_cert_delete( M2MB_SSL_CACERT, (CHAR*)gSSL_CERT_CA_NAME );
    if(res==0)
    {
      AZX_LOG_DEBUG("m2mb_ssl_cert_delete PASS\r\n");
    }
    else
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_delete failed with code %d\r\n",res);
    }
  }

  if(ssl_auth_mode >= M2MB_SSL_SERVER_CLIENT_AUTH)
  {
    res = m2mb_ssl_cert_delete( M2MB_SSL_CERT, (CHAR*)gSSL_CLIENT_NAME );
    if(res==0)
    {
      AZX_LOG_TRACE("m2mb_ssl_cert_delete PASS\r\n");
    }
    else
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_delete failed with code %d\r\n",res);
    }
  }

  res = m2mb_ssl_delete_config(*p_hSSLConfig);
  if(res==0)
  {
    AZX_LOG_DEBUG("m2mb_ssl_delete_config PASS\r\n");
  }
  else
  {
    AZX_LOG_ERROR("m2mb_ssl_delete_config failed with code %d\r\n",res);
  }

  m2mb_ssl_delete_ctxt( *p_hSSLCtx);
}


static INT32 PrepareSSLEnvironment(M2MB_SSL_CONFIG_HANDLE* p_hSSLConfig, M2MB_SSL_CTXT_HANDLE* p_hSSLCtx, M2MB_SSL_AUTH_TYPE_E ssl_auth_mode)
{

  M2MB_SSL_CONFIG_T SSLConfig;

  UINT8 CA_BUF[4096];
  UINT8 client_cert_buf[4096];
  UINT8 client_key_buf[4096];


  M2MB_SSL_SEC_INFO_U SSL_info;
  //M2MB_SSL_CONNECTION_HANDLE hSSLConn;
  M2MB_SSL_CA_INFO_T ca_Info[1];

  INT32 fd = -1;
  struct M2MB_STAT st;
  INT32 res;


  if (p_hSSLConfig == NULL || p_hSSLCtx == NULL )
  {
    return -2;
  }

  AZX_LOG_DEBUG("m2mb_ssl_create_ctxt():  \r\n");
  *p_hSSLCtx =  m2mb_ssl_create_ctxt();

  if(*p_hSSLCtx == NULL)
  {
    AZX_LOG_ERROR("m2mb_ssl_create_ctxt() failed\r\n");
    return -1;
  }
  else
  {
    AZX_LOG_DEBUG("m2mb_ssl_create_ctxt() passed\r\n" );
  }

  SSLConfig.ProtVers = M2MB_SSL_PROTOCOL_TLS_1_2;
#ifndef REDUCED_CYPHER_LIST
  SSLConfig.CipherSuites = (M2MB_SSL_CIPHER_SUITE_E *)&s_cipher_suite[0];
  SSLConfig.CipherSuitesNum = (sizeof(s_cipher_suite) / sizeof(s_cipher_suite[0]));
#else
  SSLConfig.CipherSuites = CipherSuite;

  SSLConfig.CipherSuites[0] = M2MB_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256;
  SSLConfig.CipherSuites[1] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA;
  SSLConfig.CipherSuites[2] = M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA;
  SSLConfig.CipherSuites[3] = M2MB_TLS_DHE_RSA_WITH_AES_256_CBC_SHA;
  SSLConfig.CipherSuites[4] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA;
  SSLConfig.CipherSuites[5] = M2MB_TLS_RSA_WITH_AES_128_CBC_SHA256;
  SSLConfig.CipherSuites[6] = M2MB_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256;
  SSLConfig.CipherSuites[7] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA256;
  SSLConfig.CipherSuitesNum = 8;
#endif

  SSLConfig.AuthType =   ssl_auth_mode;

  *p_hSSLConfig = m2mb_ssl_create_config( SSLConfig, &res );

  if( (*p_hSSLConfig == NULL) || (res != 0))
  {
    AZX_LOG_ERROR("m2mb_ssl_create_config FAILED error %d\r\n",res );
    return -1;
  }

  if ((ssl_auth_mode ==   M2MB_SSL_SERVER_AUTH) || (ssl_auth_mode ==   M2MB_SSL_SERVER_CLIENT_AUTH))
  {
    AZX_LOG_DEBUG("ca cert file %s \r\n",gCACERTFILE);

    if (0 ==m2mb_fs_stat(gCACERTFILE, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);
    }

    fd = m2mb_fs_open(gCACERTFILE,M2MB_O_RDONLY   /*open in read only mode*/ );
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",gCACERTFILE);
      m2mb_ssl_delete_config(*p_hSSLConfig);
      return -1;
    }

    AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
    res = m2mb_fs_read(fd, CA_BUF, st.st_size);

    if (res != (INT32) st.st_size)
    {
      AZX_LOG_DEBUG("Failed reading buffer into file.\r\n");
      m2mb_fs_close(fd);
      m2mb_ssl_delete_config(*p_hSSLConfig);
      return -1;

    }
    AZX_LOG_DEBUG("Buffer successfully received from file. %d bytes were loaded.\r\n", res);


    AZX_LOG_DEBUG("Closing file.\r\n");
    m2mb_fs_close(fd);

    SSL_info.ca_List.ca_Cnt = 1;
    SSL_info.ca_List.ca_Info[0] = &ca_Info[0];
    SSL_info.ca_List.ca_Info[0]->ca_Size =  st.st_size;
    SSL_info.ca_List.ca_Info[0]->ca_Buf = CA_BUF;

    if (0 != m2mb_ssl_cert_store( M2MB_SSL_CACERT,SSL_info,(CHAR*) gSSL_CERT_CA_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_store FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_NO_AUTH);
      return -1;
    }

    if (0 != m2mb_ssl_cert_load( *p_hSSLCtx,M2MB_SSL_CACERT,(CHAR*) gSSL_CERT_CA_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_load FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }

  }

  if (ssl_auth_mode == M2MB_SSL_SERVER_CLIENT_AUTH)
  {
    AZX_LOG_DEBUG("server + client authentication is chosen \r\n");
    AZX_LOG_DEBUG("ca cert file %s \r\n",gCLIENTCERTFILE);

    if (0 ==m2mb_fs_stat(gCLIENTCERTFILE, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);
    }

    fd = m2mb_fs_open(gCLIENTCERTFILE, M2MB_O_RDONLY);   /*open in read only mode*/
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",gCLIENTCERTFILE);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }

    AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
    res = m2mb_fs_read(fd, client_cert_buf, st.st_size);

    if (res != (INT32)st.st_size)
    {
      AZX_LOG_DEBUG("Failed reading buffer into file.\r\n");
      m2mb_fs_close(fd);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }

    AZX_LOG_DEBUG("Buffer successfully received from file. %d bytes were loaded.\r\n", res);


    AZX_LOG_DEBUG("Closing file.\r\n");
    m2mb_fs_close(fd);


    memset(&SSL_info,0x00,sizeof(SSL_info)); //try to memset SSL_info buffer to remove CA entries
    SSL_info.cert.cert_Buf=client_cert_buf;
    SSL_info.cert.cert_Size=st.st_size;


    AZX_LOG_DEBUG("Client Key file %s \r\n",gCLIENTKEYFILE);


    if (0 ==m2mb_fs_stat(gCLIENTKEYFILE, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);

    }

    fd = m2mb_fs_open(gCLIENTKEYFILE, M2MB_O_RDONLY);   /*open in read only mode*/
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",gCLIENTKEYFILE);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }

    AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
    res = m2mb_fs_read(fd, client_key_buf, st.st_size);

    if (res != (INT32)st.st_size)
    {
      AZX_LOG_DEBUG("Failed reading buffer into file.\r\n");
      m2mb_fs_close(fd);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }
    else
    {
      AZX_LOG_DEBUG("Buffer successfully received from file. %d bytes were loaded.\r\n", res);
    }

    AZX_LOG_DEBUG("Closing file.\r\n");
    m2mb_fs_close(fd);


    SSL_info.cert.key_Buf=client_key_buf;
    SSL_info.cert.key_Size=st.st_size;

    if (0 != m2mb_ssl_cert_store( M2MB_SSL_CERT,SSL_info,(CHAR*) gSSL_CLIENT_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_store FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }

    if (0 != m2mb_ssl_cert_load( *p_hSSLCtx,M2MB_SSL_CERT,(CHAR*) gSSL_CLIENT_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_load FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_CLIENT_AUTH);
      return -1;
    }
  }

  AZX_LOG_DEBUG("SSL env completed\r\n");
  return 0;
}




/* Global functions =============================================================================*/
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

void PdpCallback( M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata )
{
  ( void )userdata;
  struct M2MB_SOCKET_BSD_SOCKADDR_IN CBtmpAddress;
  CHAR CBtmpIPaddr[32];

  switch( pdp_event )
  {
    case M2MB_PDP_UP:
      AZX_LOG_DEBUG( "Context activated!\r\n" );
      m2mb_pdp_get_my_ip( h, cid, M2MB_PDP_IPV4, &CBtmpAddress.sin_addr.s_addr );
      m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET, &CBtmpAddress.sin_addr.s_addr,
                                 ( CHAR * ) & ( CBtmpIPaddr ), sizeof( CBtmpIPaddr ) );
      AZX_LOG_DEBUG( "IP address: %s\r\n", CBtmpIPaddr );
      m2mb_os_ev_set( net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_SET );
      break;

    case M2MB_PDP_DOWN:
      AZX_LOG_DEBUG( "Context deactivated!\r\n" );
      break;

    default:
      AZX_LOG_DEBUG( "Unexpected pdp_event: %d\r\n", pdp_event );
      break;
  }
}

INT32 MQTT_Task( INT32 type, INT32 param1, INT32 param2 )
{
  ( void )type;
  ( void )param1;
  ( void )param2;
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;
  M2MB_NET_HANDLE h = NULL;
  M2MB_MQTT_RESPONSE_E result = M2MB_MQTT_SUCCESS;
  M2MB_MQTT_TOPIC_T topics[2] = {{0}};
  int ret;
  int task_status = 0;
  void *myUserdata = NULL;
  int msgId = 1;

  M2MB_SSL_CONFIG_HANDLE sslConfigHndl = NULL;
  M2MB_SSL_CTXT_HANDLE sslCtxtHndl = NULL;

  do
  {
    AZX_LOG_DEBUG( "INIT\r\n" );

    configureParameters(); /*set default values first*/
    readConfigFromFile(); /*try to read configuration from file (if present)*/

    AZX_LOG_DEBUG( "Init MQTT\r\n" );
    result = m2mb_mqtt_init( &mqttHandle, NULL, NULL );

    if( result != M2MB_MQTT_SUCCESS )
    {
      task_status = APPLICATION_EXIT;
      break;
    }
    else
    {
      AZX_LOG_DEBUG( "m2mb_mqtt_init succeeded\r\n" );
    }

    result = m2mb_mqtt_conf( mqttHandle, CMDS(
            /* Set Client ID */
            M2MB_MQTT_SET_CLIENT_ID, ( char * ) gCLIENT_ID ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set Timeout in milliseconds */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
            M2MB_MQTT_SET_TIMEOUT_MS, gCLIENT_TIMEOUT_SEC * 1000 ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set Keepalive in seconds */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
            M2MB_MQTT_SET_KEEPALIVE_SEC, gCLIENT_KEEPALIVE_SEC ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set Username */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
            M2MB_MQTT_SET_USERNAME, ( char * ) gCLIENT_USERNAME ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set Password */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
            M2MB_MQTT_SET_PASSWORD, ( char * ) CLIENT_PASSWORD ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set the PDP context to be used */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
            M2MB_MQTT_SET_PDP_CONTEXT, gPDP_CTX ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    if(gUSE_TLS)
    {
    if (0 == PrepareSSLEnvironment(&sslConfigHndl, &sslCtxtHndl, (M2MB_SSL_AUTH_TYPE_E)gUSER_SSL_AUTH))
    {
      result = m2mb_mqtt_conf(mqttHandle, CMDS(M2MB_MQTT_SECURE_OPT, sslConfigHndl, sslCtxtHndl));
      if( result != M2MB_MQTT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_mqtt_conf() - configuring M2MB_MQTT_SECURE_OPT failed!\r\n");
        break;
      }
    }
    else
    {
      AZX_LOG_ERROR("PrepareSSLEnvironment() failed!\r\n");
      break;
	    }
    }


    /* Init events handler */
    osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS( M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL,
        M2MB_OS_EV_SEL_CMD_NAME, "net_pdp_ev" ) );
    osRes = m2mb_os_ev_init( &net_pdp_evHandle, &evAttrHandle );

    if( osRes != M2MB_OS_SUCCESS )
    {
      m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
      AZX_LOG_CRITICAL( "m2mb_os_ev_init failed!\r\n" );
      return -1;
    }
    else
    {
      AZX_LOG_DEBUG( "m2mb_os_ev_init success\r\n" );
    }

    /* check network registration and activate PDP context */
    retVal = m2mb_net_init( &h, NetCallback, myUserdata );

    if( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_DEBUG( "m2mb_net_init returned M2MB_RESULT_SUCCESS\r\n" );
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

    AZX_LOG_DEBUG( "Waiting for registration...\r\n" );
    retVal = m2mb_net_get_reg_status_info( h );

    if( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
    }

    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get( net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY, &curEvBits,
        M2MB_OS_WAIT_FOREVER );

    AZX_LOG_DEBUG( "Pdp context activation\r\n" );
    retVal = m2mb_pdp_init( &pdpHandle, PdpCallback, myUserdata );

    if( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "m2mb_pdp_init returned M2MB_RESULT_SUCCESS\r\n" );
    }
    else
    {
      AZX_LOG_CRITICAL( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
      return -1;
    }

    azx_sleep_ms( 2000 );
    AZX_LOG_DEBUG( "Activate PDP with APN %s on CID %d....\r\n", gAPN, gPDP_CTX );
    retVal = m2mb_pdp_activate( pdpHandle, ( UINT8 ) gPDP_CTX, ( char * ) gAPN, ( char * ) gAPN_UserName,
            ( char * ) gAPN_Password, M2MB_PDP_IPV4 ); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4

    if( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "Cannot activate pdp context. Trying deactivating and reactivating again\r\n" );
      m2mb_pdp_deactivate( pdpHandle, gPDP_CTX );
      azx_sleep_ms( 1000 );
      retVal = m2mb_pdp_activate( pdpHandle, ( UINT8 ) gPDP_CTX, ( char * ) gAPN, ( char * ) gAPN_UserName,
              ( char * ) gAPN_Password, M2MB_PDP_IPV4 ); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4

      if( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR( "Cannot activate PDP context. Quitting...\r\n" );
        return -1;
      }
    }

    /* Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get( net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
        M2MB_OS_WAIT_FOREVER );

    if(gUSE_TLS)
    {
    AZX_LOG_INFO( "\r\nConnecting to broker <%s>:%u...\r\n", gMQTT_BROKER_ADDRESS, gMQTT_BROKER_PORT_SSL );

    result = m2mb_mqtt_connect( mqttHandle, gMQTT_BROKER_ADDRESS, gMQTT_BROKER_PORT_SSL );
    }
    else
    {
      AZX_LOG_INFO( "\r\nConnecting to broker <%s>:%u...\r\n", gMQTT_BROKER_ADDRESS, gMQTT_BROKER_PORT );
      result = m2mb_mqtt_connect( mqttHandle, gMQTT_BROKER_ADDRESS, gMQTT_BROKER_PORT );
    }
    if( result == M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_INFO( "Done.\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_mqtt_connect failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    topics[0].topic_filter = gSUB_TOPIC; /* Select the topic */
    topics[0].qos = M2MB_MQTT_QOS_0;          /* The Quality of Service */
    topics[0].cb = mqtt_topic_cb; /*And the callback function that will be called when data arrives.*/
    topics[1].topic_filter = gSUB_TOPIC2; /*Select a second topic */
    topics[1].qos = M2MB_MQTT_QOS_0;          /*The Quality of Service*/
    topics[1].cb = mqtt_topic_cb; /*And the callback function that will be called when data arrives.*/
    AZX_LOG_INFO( "Subscribing to %s and %s..\r\n", gSUB_TOPIC, gSUB_TOPIC2 );
    result = m2mb_mqtt_subscribe( mqttHandle, 1 /*Start from msg ID 1*/, 2, topics );

    if( result == M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_DEBUG( "Done.\r\n\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_mqtt_subscribe failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    while( msgId++ <= 10 )
    {
      UINT8 retain = 0;
      char *topic = NULL;
      char msg[64];

      if( msgId % 2 == 0 )
      {
        topic = ( char * ) gSUB_TOPIC;
      }
      else
      {
        topic = ( char * ) gSUB_TOPIC2;
      }

      sprintf( msg, "%s ID: %d", gPUB_MESSAGE, msgId );
      AZX_LOG_DEBUG( "PUBLISHING <%s> to topic %s\r\n", msg, topic );
      result = m2mb_mqtt_publish( mqttHandle, M2MB_MQTT_QOS_0, retain, msgId, topic, msg, strlen( msg ) );

      if( result == M2MB_MQTT_SUCCESS )
      {
        AZX_LOG_DEBUG( "Done.\r\n" );
      }
      else
      {
        AZX_LOG_ERROR( "m2mb_mqtt_publish failed! Result: %d\r\n", result );
        task_status = APPLICATION_EXIT;
        break;
      }

      azx_sleep_ms( 3000 );
    }

    AZX_LOG_INFO( "\r\nDisconnecting from MQTT broker..\r\n" );
    result = m2mb_mqtt_disconnect( mqttHandle );

    if( result == M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_DEBUG( "Done.\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_mqtt_disconnect failed! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    if(gUSE_TLS)
    {
    CleanSSLEnvironment(&sslConfigHndl, &sslCtxtHndl, (M2MB_SSL_AUTH_TYPE_E)gUSER_SSL_AUTH);
    }

    result = m2mb_mqtt_deinit( mqttHandle );
    if( result == M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_TRACE( "m2mb_mqtt_deinit succeeded\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_mqtt_deinit failed! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    task_status = APPLICATION_EXIT;
    break;
  }
  while( 0 );

  if( task_status == APPLICATION_EXIT )
  {
    AZX_LOG_DEBUG( "application exit\r\n" );
    ret = m2mb_pdp_deactivate( pdpHandle, gPDP_CTX );

    if( ret != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "CANNOT DEACTIVATE PDP\r\n" );
      return -1;
    }
    else
    {
      AZX_LOG_DEBUG( "m2mb_pdp_deactivate returned success\r\n" );
    }

    m2mb_os_ev_deinit( net_pdp_evHandle );
    AZX_LOG_DEBUG( "Application complete.\r\n" );
  }

  return 0;
}

