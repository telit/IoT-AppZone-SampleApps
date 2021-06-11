/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    mqtt_demo.c

  @brief
    The file contains the MQTT utilities and logic

  @details

  @version
    1.0.3
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


/* Local defines =====================================================================*/

#define SSL_CERT_CA_NAME "ca-cert-pool"
#define SSL_CLIENT_NAME "SSL-Client"


/* SSL */
/*Note: A possible root path for certificates could be LOCALPATH define in app_cfg.h file */
#define CACERTFILE        ""   /* Root CA file path in module filesystem (if needed) */
#define CLIENTCERTFILE    ""   /* Client certificate file path in module filesystem (if needed) */
#define CLIENTKEYFILE     ""   /* Client private key file path in module filesystem (if needed) */


#define USER_SSL_AUTH      M2MB_SSL_NO_AUTH

/* Server configuration */
#define MQTT_BROKER_ADDRESS "api-dev.devicewise.com"

#define MQTT_BROKER_PORT        (UINT32)1883
#define MQTT_BROKER_PORT_SSL    (UINT32)8883

/* Client Configuration */
#define CLIENT_ID "test_m2mb_mqtt_id"
#define CLIENT_USERNAME "test_m2mb_mqtt"
#define CLIENT_PASSWORD "q3XYKetChZRdGuKF"


#define CLIENT_TIMEOUT_SEC 60 /*operations timeout*/

#define CLIENT_KEEPALIVE_SEC 60 /*KeepAlive timeout*/

#define SUB_TOPIC "test_topic"
#define SUB_TOPIC2 "test_topic2"

#define PUB_MESSAGE "Hello from M2MB MQTT!"

/* PDP configuration */
#define APN      "web.omnitel.it"

#define PDP_CTX   3






/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;
static M2MB_PDP_HANDLE pdpHandle;

static M2MB_MQTT_HANDLE mqttHandle = NULL;

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

  if ((ssl_auth_mode ==   M2MB_SSL_SERVER_AUTH) || (ssl_auth_mode ==   M2MB_SSL_SERVER_CLIENT_AUTH))
  {

    /* clean everything */
    res = m2mb_ssl_cert_delete( M2MB_SSL_CACERT, (CHAR*)SSL_CERT_CA_NAME );
    if(res==0)
    {
      AZX_LOG_DEBUG("m2mb_ssl_cert_delete PASS\r\n");
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
  M2MB_SSL_CIPHER_SUITE_E CipherSuite[4];
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
  SSLConfig.CipherSuites = CipherSuite;

  SSLConfig.CipherSuites[0] = M2MB_TLS_RSA_WITH_AES_128_CBC_SHA;
  SSLConfig.CipherSuites[1] = M2MB_TLS_RSA_WITH_AES_256_CBC_SHA;
  SSLConfig.CipherSuitesNum = 2;

  SSLConfig.AuthType =   ssl_auth_mode;

  *p_hSSLConfig = m2mb_ssl_create_config( SSLConfig, &res );

  if( (*p_hSSLConfig == NULL) || (res != 0))
  {
    AZX_LOG_ERROR("m2mb_ssl_create_config FAILED error %d\r\n",res );
    return -1;
  }

  if ((ssl_auth_mode ==   M2MB_SSL_SERVER_AUTH) || (ssl_auth_mode ==   M2MB_SSL_SERVER_CLIENT_AUTH))
  {
    AZX_LOG_DEBUG("ca cert file %s \r\n",CACERTFILE);

    if (0 ==m2mb_fs_stat(CACERTFILE, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);
    }

    fd = m2mb_fs_open(CACERTFILE,M2MB_O_RDONLY   /*open in read only mode*/ );
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",CACERTFILE);
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

    if (0 != m2mb_ssl_cert_store( M2MB_SSL_CACERT,SSL_info,(CHAR*) SSL_CERT_CA_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_store FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }

    if (0 != m2mb_ssl_cert_load( *p_hSSLCtx,M2MB_SSL_CACERT,(CHAR*) SSL_CERT_CA_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_load FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }

  }

  if (ssl_auth_mode ==   M2MB_SSL_SERVER_CLIENT_AUTH)
  {
    AZX_LOG_DEBUG("server + client authentication is chosen \r\n");
    AZX_LOG_DEBUG("ca cert file %s \r\n",CLIENTCERTFILE);

    if (0 ==m2mb_fs_stat(CLIENTCERTFILE, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);

    }

    fd = m2mb_fs_open(CLIENTCERTFILE, M2MB_O_RDONLY);   /*open in read only mode*/
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",CLIENTCERTFILE);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }

    AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
    res = m2mb_fs_read(fd, client_cert_buf, st.st_size);

    if (res != (INT32)st.st_size)
    {
      AZX_LOG_DEBUG("Failed reading buffer into file.\r\n");
      m2mb_fs_close(fd);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }

    AZX_LOG_DEBUG("Buffer successfully received from file. %d bytes were loaded.\r\n", res);


    AZX_LOG_DEBUG("Closing file.\r\n");
    m2mb_fs_close(fd);


    memset(&SSL_info,0x00,sizeof(SSL_info)); //try to memset SSL_info buffer to remove CA entries
    SSL_info.cert.cert_Buf=client_cert_buf;
    SSL_info.cert.cert_Size=st.st_size;


    AZX_LOG_DEBUG("Client Key file %s \r\n",CLIENTKEYFILE);


    if (0 ==m2mb_fs_stat(CLIENTKEYFILE, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);

    }

    fd = m2mb_fs_open(CLIENTKEYFILE, M2MB_O_RDONLY);   /*open in read only mode*/
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",CLIENTKEYFILE);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }

    AZX_LOG_DEBUG("Reading content from file. Size: %u\r\n", st.st_size);
    res = m2mb_fs_read(fd, client_key_buf, st.st_size);

    if (res != (INT32)st.st_size)
    {
      AZX_LOG_DEBUG("Failed reading buffer into file.\r\n");
      m2mb_fs_close(fd);
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }
    else
    {
      AZX_LOG_DEBUG("Buffer successfully received from file. %d bytes were loaded.\r\n", res);
    }

    AZX_LOG_DEBUG("Closing file.\r\n");
    m2mb_fs_close(fd);


    SSL_info.cert.key_Buf=client_key_buf;
    SSL_info.cert.key_Size=st.st_size;

    if (0 != m2mb_ssl_cert_store( M2MB_SSL_CERT,SSL_info,(CHAR*) SSL_CLIENT_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_store FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }

    if (0 != m2mb_ssl_cert_load( *p_hSSLCtx,M2MB_SSL_CERT,(CHAR*) SSL_CLIENT_NAME ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_load FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, ssl_auth_mode);
    }
  }

  AZX_LOG_DEBUG("SSL env completed\r\n");
  return 0;
}




/* Global functions =============================================================================*/
void NetCallback( M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct,
                  void *myUserdata )
{
  ( void )resp_size;
  ( void )myUserdata;
  M2MB_NET_REG_STATUS_T *stat_info;

  switch( net_event )
  {
    case M2MB_NET_GET_REG_STATUS_INFO_RESP:
      stat_info = ( M2MB_NET_REG_STATUS_T * )resp_struct;

      if( stat_info->stat == 1 || stat_info->stat == 5 )
      {
        AZX_LOG_DEBUG( "Module is registered\r\n" );
        m2mb_os_ev_set( net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_SET );
      }
      else
      {
        m2mb_net_get_reg_status_info( h ); /* try again */
      }
      break;

    default:
      AZX_LOG_DEBUG( "unexpected net_event: %d\r\n", net_event );
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
  M2MB_NET_HANDLE h;
  M2MB_MQTT_RESPONSE_E result = M2MB_MQTT_SUCCESS;
  M2MB_MQTT_TOPIC_T topics[2] = {0};
  int ret;
  int task_status;
  void *myUserdata = NULL;
  int msgId = 1;

  M2MB_SSL_CONFIG_HANDLE sslConfigHndl;
  M2MB_SSL_CTXT_HANDLE sslCtxtHndl;

  do
  {
    AZX_LOG_DEBUG( "INIT\r\n" );


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
        M2MB_MQTT_SET_CLIENT_ID, ( char * ) CLIENT_ID ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set Timeout in milliseconds */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
        M2MB_MQTT_SET_TIMEOUT_MS, CLIENT_TIMEOUT_SEC * 1000 ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set Keepalive in seconds */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
        M2MB_MQTT_SET_KEEPALIVE_SEC, CLIENT_KEEPALIVE_SEC ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    /* Set Username */
    result = m2mb_mqtt_conf( mqttHandle, CMDS(
        M2MB_MQTT_SET_USERNAME, ( char * ) CLIENT_USERNAME ) );

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
        M2MB_MQTT_SET_PDP_CONTEXT, PDP_CTX ) );

    if( result != M2MB_MQTT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_mqtt_conf failed!! Result: %d\r\n", result );
      task_status = APPLICATION_EXIT;
      break;
    }

    if (0 == PrepareSSLEnvironment(&sslConfigHndl, &sslCtxtHndl, USER_SSL_AUTH))
    {
      result = m2mb_mqtt_conf(mqttHandle, CMDS(M2MB_MQTT_SECURE_OPT, sslConfigHndl, sslCtxtHndl));
      if( result != M2MB_MQTT_SUCCESS )
      {
        AZX_LOG_ERROR("m2mb_mqtt_conf() - configuring M2MB_MQTT_SECURE_OPT failed!\r\n");
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

    AZX_LOG_DEBUG( "Waiting for registration...\r\n" );
    retVal = m2mb_net_get_reg_status_info( h );

    if( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
    }

    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get( net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
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
    AZX_LOG_DEBUG( "Activate PDP with APN %s on CID %d....\r\n", APN, PDP_CTX );
    retVal = m2mb_pdp_activate( pdpHandle, ( UINT8 ) PDP_CTX, ( char * ) APN, ( char * ) NULL,
        ( char * ) NULL, M2MB_PDP_IPV4 ); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4

    if( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "Cannot activate pdp context. Trying deactivating and reactivating again\r\n" );
      m2mb_pdp_deactivate( pdpHandle, PDP_CTX );
      azx_sleep_ms( 1000 );
      retVal = m2mb_pdp_activate( pdpHandle, ( UINT8 ) PDP_CTX, ( char * ) APN, ( char * ) NULL,
          ( char * ) NULL, M2MB_PDP_IPV4 ); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4

      if( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR( "Cannot activate PDP context. Quitting...\r\n" );
        return -1;
      }
    }

    /* Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get( net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits,
        M2MB_OS_WAIT_FOREVER );

    AZX_LOG_INFO( "\r\nConnecting to broker <%s>:%u...\r\n", MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT_SSL );

    result = m2mb_mqtt_connect( mqttHandle, MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT_SSL );
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

    topics[0].topic_filter = SUB_TOPIC; /* Select the topic */
    topics[0].qos = M2MB_MQTT_QOS_0;          /* The Quality of Service */
    topics[0].cb = mqtt_topic_cb; /*And the callback function that will be called when data arrives.*/
    topics[1].topic_filter = SUB_TOPIC2; /*Select a second topic */
    topics[1].qos = M2MB_MQTT_QOS_0;          /*The Quality of Service*/
    topics[1].cb = mqtt_topic_cb; /*And the callback function that will be called when data arrives.*/
    AZX_LOG_INFO( "Subscribing to %s and %s..\r\n", SUB_TOPIC, SUB_TOPIC2 );
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
        topic = ( char * ) SUB_TOPIC;
      }
      else
      {
        topic = ( char * ) SUB_TOPIC2;
      }

      sprintf( msg, "%s ID: %d", PUB_MESSAGE, msgId );
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

    CleanSSLEnvironment(&sslConfigHndl, &sslCtxtHndl, USER_SSL_AUTH);

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
    ret = m2mb_pdp_deactivate( pdpHandle, PDP_CTX );

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

