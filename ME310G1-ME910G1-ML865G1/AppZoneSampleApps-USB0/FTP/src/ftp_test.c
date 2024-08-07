/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*
 * ftp_test.c
 *
 */

#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"
#include "m2mb_fs_posix.h" //for file stats
#include "m2mb_fs_stdio.h"
#include "m2mb_ssl.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "azx_ftp.h"

#include "ftp_test.h"
#include "app_cfg.h" /*FOR LOCALPATH define*/

#include "read_parameters.h"
/* Macro =============================================================================*/

/* Local defines =======================================================================*/
struct REMFILE {
  struct REMFILE *next;
  azx_ftp_fsz_t fsz;
  char *fnm;
};

/* Local statics ================================================================================*/

static char filename_buffer[128];

static M2MB_PDP_HANDLE pdpHandle;

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

static AZX_FTP_NET_BUF_T *ftp_client = NULL;

struct REMFILE rem_file;
struct REMFILE loc_file;

static char gFTP_Address[512];



M2MB_SSL_CONFIG_HANDLE sslConfigHndl = NULL;
M2MB_SSL_CTXT_HANDLE sslCtxtHndl = NULL;

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
static UINT32 get_uptime(void);
static const char* get_file_title(const CHAR* path);
static int log_progress(AZX_FTP_NET_BUF_T *ctl, azx_ftp_fsz_t xfered, void *arg);
static INT32 ftp_debug_hk(AZX_FTP_DEBUG_HOOK_LEVELS_E level, const CHAR *function,
    const CHAR *file, INT32 line, const CHAR *fmt, ...);

static void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata);
static void PdpCallback(M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata);

static void CleanSSLEnvironment(M2MB_SSL_CONFIG_HANDLE* p_hSSLConfig, M2MB_SSL_CTXT_HANDLE* p_hSSLCtx, M2MB_SSL_AUTH_TYPE_E ssl_auth_mode);
static INT32 PrepareSSLEnvironment(M2MB_SSL_CONFIG_HANDLE* p_hSSLConfig, M2MB_SSL_CTXT_HANDLE* p_hSSLCtx, M2MB_SSL_AUTH_TYPE_E ssl_auth_mode);

/* Static functions =============================================================================*/


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
    res = m2mb_ssl_cert_delete( M2MB_SSL_CACERT, (CHAR*)gCA_CERT_PATH );
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
    res = m2mb_ssl_cert_delete( M2MB_SSL_CERT, (CHAR*)gCLIENT_CERT_PATH );
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
  SSLConfig.AuthType = ssl_auth_mode;

  *p_hSSLConfig = m2mb_ssl_create_config( SSLConfig, &res );

  if( (*p_hSSLConfig == NULL) || (res != 0))
  {
    AZX_LOG_ERROR("m2mb_ssl_create_config FAILED error %d\r\n",res );
    return -1;
  }

  if ((ssl_auth_mode == M2MB_SSL_SERVER_AUTH) || (ssl_auth_mode == M2MB_SSL_SERVER_CLIENT_AUTH))
  {
    AZX_LOG_DEBUG("ca cert file %s \r\n",gCA_CERT_PATH);

    if (0 == m2mb_fs_stat(gCA_CERT_PATH, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);
    }

    fd = m2mb_fs_open(gCA_CERT_PATH,M2MB_O_RDONLY   /*open in read only mode*/ );
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",gCA_CERT_PATH);
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

    if (0 != m2mb_ssl_cert_store( M2MB_SSL_CACERT,SSL_info,(CHAR*) "CAListTest" ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_store FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_NO_AUTH);
      return -1;
    }

    if (0 != m2mb_ssl_cert_load( *p_hSSLCtx, M2MB_SSL_CACERT,(CHAR*) "CAListTest" ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_load FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }

  }

  if (ssl_auth_mode == M2MB_SSL_SERVER_CLIENT_AUTH)
  {
    AZX_LOG_DEBUG("server + client authentication is chosen \r\n");
    AZX_LOG_DEBUG("ca cert file %s \r\n",gCLIENT_CERT_PATH);

    if (0 ==m2mb_fs_stat(gCLIENT_CERT_PATH, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);
    }

    fd = m2mb_fs_open(gCLIENT_CERT_PATH, M2MB_O_RDONLY);   /*open in read only mode*/
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",gCLIENT_CERT_PATH);
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


    AZX_LOG_DEBUG("Client Key file %s \r\n",gCLIENT_KEY_PATH);


    if (0 ==m2mb_fs_stat(gCLIENT_KEY_PATH, &st))
    {
      AZX_LOG_DEBUG("file size: %u\r\n",  st.st_size);

    }

    fd = m2mb_fs_open(gCLIENT_KEY_PATH, M2MB_O_RDONLY);   /*open in read only mode*/
    if (fd == -1 )
    {
      AZX_LOG_DEBUG("Cannot open file %s \r\n",gCLIENT_KEY_PATH);
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

    if (0 != m2mb_ssl_cert_store( M2MB_SSL_CERT,SSL_info,(CHAR*) "ClientCertTest" ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_store FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_AUTH);
      return -1;
    }

    if (0 != m2mb_ssl_cert_load( *p_hSSLCtx,M2MB_SSL_CERT,(CHAR*) "ClientCertTest" ))
    {
      AZX_LOG_ERROR("m2mb_ssl_cert_load FAILED\r\n" );
      CleanSSLEnvironment(p_hSSLConfig, p_hSSLCtx, M2MB_SSL_SERVER_CLIENT_AUTH);
      return -1;
    }
  }

  AZX_LOG_DEBUG("SSL env completed\r\n");
  return 0;
}



static UINT32 get_uptime(void)
{

  UINT32 sysTicks = m2mb_os_getSysTicks();

  FLOAT32 ms_per_tick = m2mb_os_getSysTickDuration_ms();

  return (UINT32) (sysTicks * ms_per_tick); //milliseconds
}

/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Removes the file path from the provided path, leaving only filename

  \param [in] path: the file path
  \return the filename(+ extension) extracted from the path

 */
/*-----------------------------------------------------------------------------------------------*/
static const char* get_file_title(const CHAR* path)
{
  const CHAR* p = path;

  while (*p) {
    if (*p == '/' || *p == '\\') {
      return p + 1;
    }

    p++;
  }
  return path;
}

static int log_progress(AZX_FTP_NET_BUF_T *ctl, azx_ftp_fsz_t xfered, void *arg)
{
  (void) ctl;
  struct REMFILE *f = (struct REMFILE *) arg;
  if ( f->fsz )
  {
    double pct = (xfered * 100.0) / f->fsz;
    AZX_LOG_INFO("%s %5.2f%% %u\r\n", f->fnm, pct, xfered);
  }
  else
  {
    AZX_LOG_INFO("%s %u\r\rn", f->fnm, xfered);
  }

  return 1;
}


INT32 buf_data_cb(CHAR *data, UINT32 datalen, INT32 ev)
{
  switch(ev)
  {
    case DATA_CB_START:
      /* Do some initialization before receiving data (e.g. open a file descriptor) */
      AZX_LOG_DEBUG("Received START event\r\n");
      break;
    case DATA_CB_DATA:
      /* Manage the data */
      AZX_LOG_DEBUG("Received DATA: %u bytes on buffer %p\r\n", datalen, data);
      break;
    case DATA_CB_END:
      /* Finalize */
      AZX_LOG_DEBUG("Received END event\r\n");
      break;
  }
  return 1;
}

static INT32 ftp_debug_hk(AZX_FTP_DEBUG_HOOK_LEVELS_E level, const CHAR *function,
    const CHAR *file, INT32 line, const CHAR *fmt, ...)
{
  char buf[512];
  int bufSize = sizeof(buf);
  va_list arg;
  INT32   offset = 0;
  UINT32  now = get_uptime();

  memset(buf,0,bufSize);

  switch(level)
  {
    case AZX_FTP_DEBUG_HOOK_ERROR:
      offset = sprintf(buf, "%5u.%03u %6s - %15s:%-4d - %32s - ",
              now / 1000, now % 1000,
              "ERR - ",
              get_file_title(file), line,
              function);
      break;
      break;
    case AZX_FTP_DEBUG_HOOK_INFO:
      break;
    case AZX_FTP_DEBUG_HOOK_DEBUG:
      offset = sprintf(buf, "%5u.%03u %6s - %15s:%-4d - %32s - ",
              now / 1000, now % 1000,
              "DBG - ",
              get_file_title(file), line,
              function);
      break;
    default:
      break;
  }
  va_start(arg, fmt);
  vsnprintf(buf + offset, bufSize-offset, fmt, arg);
  va_end(arg);

  return AZX_LOG_INFO("%s", buf);
}


static void checkNetStat(  M2MB_NET_REG_STATUS_T *stat_info)
{
  if  (stat_info->stat == 1 || stat_info->stat == 5)
  {
    AZX_LOG_DEBUG("Module is registered to network\r\n");
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

static void PdpCallback(M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata)
{
  (void)userdata;
  struct M2MB_SOCKET_BSD_SOCKADDR_IN CBtmpAddress;

  CHAR CBtmpIPaddr[32];

  switch (pdp_event)
  {
    case M2MB_PDP_UP:
      AZX_LOG_DEBUG ("Context active\r\n");
      m2mb_pdp_get_my_ip(h, cid, M2MB_PDP_IPV4, &CBtmpAddress.sin_addr.s_addr);
      m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET, &CBtmpAddress.sin_addr.s_addr, ( CHAR * )&( CBtmpIPaddr ), sizeof( CBtmpIPaddr ) );
      AZX_LOG_DEBUG( "IP address: %s\r\n", CBtmpIPaddr);

      m2mb_os_ev_set(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_SET);
      break;

    case M2MB_PDP_DOWN:
      AZX_LOG_DEBUG ("Context deactive\r\n");
      break;
    default:
      AZX_LOG_DEBUG("unexpected pdp_event: %d\r\n", pdp_event);
      break;

  }
}

/* Global functions =============================================================================*/


INT32 msgFTPTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

  M2MB_NET_HANDLE h;

  int ret;
  int task_status;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;

  AZX_FTP_OPTIONS_T opts;
  AZX_FTP_CALLBACK_OPTIONS_T cb_opts;
  UINT32 file_size;


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
      AZX_LOG_DEBUG( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
    }

    azx_sleep_ms(2000);

    AZX_LOG_DEBUG("Activate PDP with APN %s on cid %d....\r\n", gAPN, gPDP_CTX);
    retVal = m2mb_pdp_activate(pdpHandle, gPDP_CTX, (CHAR *) gAPN, (CHAR *) gAPN_UserName, (CHAR *) gAPN_Password, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("cannot activate pdp context.\r\n");
    }

    /*Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);



    AZX_LOG_INFO("Start ftp client...\r\n");

    opts.level = AZX_FTP_DEBUG_HOOK_ERROR;
    opts.cbFunc = ftp_debug_hk;
    opts.cid = gPDP_CTX;
    opts.ssl = gENABLE_TLS;

    if(gENABLE_TLS)
    {
      ret = PrepareSSLEnvironment(&sslConfigHndl, &sslCtxtHndl, (M2MB_SSL_AUTH_TYPE_E)gAUTH_TYPE);
      if(ret < 0)
      {
        AZX_LOG_ERROR("failed initializing SSL session... \r\n");
        return -1;
      }
      else
      {
        opts.sslConfigH = sslConfigHndl;
        opts.sslCtxtH = sslCtxtHndl;
      }
    }

    ret = azx_ftp_init(&opts);
    if (ret != 1)
    {
      AZX_LOG_ERROR("failed initializing ftp_client... \r\n");
      return -1;
    }

    strcpy(gFTP_Address, gFTP_ADDR);

    /*manage custom server port*/
    if (gFTP_PORT != AZX_FTP_DEFAULT_PORTNUM)
    {
      snprintf(gFTP_Address + strlen(gFTP_Address), sizeof(gFTP_Address) - strlen(gFTP_Address),":%u", gFTP_PORT);
    }
    AZX_LOG_INFO("Connecting to host: <%s>\r\n", gFTP_Address);


    ret = azx_ftp_connect(gFTP_Address, &ftp_client);
    if (ret == 1)
    {
      AZX_LOG_DEBUG("Connected.\r\n");
    }
    else
    {
      AZX_LOG_ERROR("failed connecting.. \r\n");
      if(gENABLE_TLS)
      {
        CleanSSLEnvironment(&sslConfigHndl, &sslCtxtHndl, (M2MB_SSL_AUTH_TYPE_E)gAUTH_TYPE);
      }
      task_status = APPLICATION_EXIT;
      break;
    }
    azx_sleep_ms(1000);

    if(gENABLE_TLS)
    {
      ret = azx_ftp_sslCfg('P', '0', ftp_client);
      if (ret == 1)
      {
        AZX_LOG_DEBUG("Protocol private and BS set.\r\n");
      }
      else
      {
        AZX_LOG_ERROR("failed \r\n");
        task_status = APPLICATION_EXIT;
        break;
      }
    }

    ret = azx_ftp_login(gFTP_USER, gFTP_PASS, ftp_client);
    if (ret == 1)
    {
      AZX_LOG_DEBUG("FTP login successful.\r\n");
    }
    else
    {
      AZX_LOG_ERROR("failed login.. \r\n");
      task_status = APPLICATION_EXIT;
      break;
    }

#if 0
    AZX_LOG_DEBUG("set option with CONN MODE Passive.\r\n");

    ret = azx_ftp_options(AZX_FTP_CONNMODE, (long)AZX_FTP_PASSIVE,ftp_client );
    if (ret == 1)
    {
      AZX_LOG_DEBUG("Done.\r\n");
    }
    else
    {
      AZX_LOG_ERROR("Failed set option.\r\n");
      task_status = APPLICATION_EXIT;
      break;
    }
    //AZX_LOG_DEBUG("last server response: <%s>\r\n", FtpLastResponse(ftp_client));
#endif

    /* =======================
     *
     *  LIST
     *
     * ======================*/
     AZX_FTP_XFER_T local;
     int filesize = 2*1024;  //to be changed according with the size
    {

      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      memset(filename_buffer, 0, sizeof(filename_buffer));
      sprintf(filename_buffer, "%s", gREMOTE_FOLDER);

      rem_file.fnm = (char*)filename_buffer;


      azx_ftp_clearCallback(ftp_client);

      cb_opts.cbFunc = log_progress;
      cb_opts.cbArg = &rem_file;
      cb_opts.idleTime = 1000;
      cb_opts.bytesXferred = 1024;

      local.type = AZX_FTP_XFER_BUFF;                                        /* Define the local recipient as buffer */
      local.payload.buffInfo.buffer = (CHAR*) m2mb_os_malloc(filesize + 1); /* Set the local buffer reference that will hold the data */
      local.payload.buffInfo.bufferSize = filesize + 1;                     /* Set the local buffer size */

      local.payload.buffInfo.buf_cb = buf_data_cb; /* The data callback to be executed. Set as NULL if not required */

      if(! local.payload.buffInfo.buffer)
      {
        AZX_LOG_ERROR("failed allocating buffer.. \r\n");
        task_status = APPLICATION_EXIT;
        break;
      }

      memset(local.payload.buffInfo.buffer, 0, local.payload.buffInfo.bufferSize); /*file_size + 2*/

      azx_ftp_setCallback(&cb_opts, ftp_client);

      AZX_LOG_INFO("Get file list of %s \r\n", rem_file.fnm);


      ret = azx_ftp_dir(&(local), rem_file.fnm, ftp_client);
      if (ret > 0)
      {
        AZX_LOG_DEBUG("Done. File list\r\n");
      }
      else
      {
        AZX_LOG_ERROR("File list.. \r\n");
        task_status = APPLICATION_EXIT;
        break;
      }
    }


    /* =======================
     *
     *  Remote SIZE
     *
     * ======================*/

    memset(filename_buffer, 0, sizeof(filename_buffer));
    sprintf(filename_buffer, "%s/%s", gREMOTE_FOLDER, gDLFILE);

    rem_file.fnm = (char*)filename_buffer;
    rem_file.fsz = 0;

    AZX_LOG_INFO("Get remote file %s size\r\n", rem_file.fnm);
    ret = azx_ftp_size(rem_file.fnm, &file_size, AZX_FTP_BINARY, ftp_client);
    if (ret == 1)
    {
      AZX_LOG_DEBUG("Done. File size: %u.\r\n", file_size);
      rem_file.fsz = file_size;
    }
    else
    {
      AZX_LOG_ERROR("failed file size.. error: %s \r\n", azx_ftp_lastResponse(ftp_client));
      task_status = APPLICATION_EXIT;
      break;
    }

    /* =======================
     *
     *  MODTIME
     *
     * ======================*/

    {
      CHAR tmp[128];
      AZX_LOG_INFO("Get remote file %s last modification date\r\n", rem_file.fnm);
      ret = azx_ftp_modDate((const CHAR*) rem_file.fnm, tmp, (INT32) sizeof(tmp), ftp_client);
      if (ret == 1)
      {
        AZX_LOG_DEBUG("Done. File last mod date: %s.\r\n", tmp);
      }
      else
      {
        AZX_LOG_ERROR("failed file last mod date.. \r\n");
        task_status = APPLICATION_EXIT;
        break;
      }
    }

    /* =======================
     *
     *  DOWNLOAD
     *
     * ======================*/
    {
      //AZX_FTP_XFER_T local;

      azx_ftp_clearCallback(ftp_client);

      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      cb_opts.cbFunc = log_progress;
      cb_opts.cbArg = &rem_file;    /* Pass the file name and size as parameter */
      cb_opts.idleTime = 2000;      /* Call the callback function every 2 seconds if there were no data exchange */
      cb_opts.bytesXferred = 8*1024;  /* Call the callback function every 8 kBytes data exchange */

      local.type = AZX_FTP_XFER_FILE; /* Define the local recipient as file */

      char path[100];
      sprintf(path,"%s/_%s", LOCALPATH, gDLFILE);
      local.payload.fileInfo.path = (char*) path;

      azx_ftp_setCallback(&cb_opts,ftp_client);

      AZX_LOG_INFO("\r\nStarting download of remote file %s into local %s\r\n", rem_file.fnm, local.payload.fileInfo.path);

      ret = azx_ftp_get(&(local), rem_file.fnm, AZX_FTP_BINARY, ftp_client);

      if (ret == 1)
      {
        AZX_LOG_DEBUG("download successful.\r\n");
      }
      else
      {
        AZX_LOG_ERROR("failed download.. \r\n");
        task_status = APPLICATION_EXIT;
        break;
      }

      azx_ftp_clearCallback(ftp_client);
    }


    /* =======================
     *
     *  UPLOAD
     *
     * ======================*/
    {
      AZX_FTP_XFER_T local;
      struct M2MB_STAT st;
      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      azx_ftp_clearCallback(ftp_client);

      char fnm[100];
      sprintf(fnm, "%s/_%s",LOCALPATH, gDLFILE);
      loc_file.fnm = (char*) fnm;
      loc_file.fsz = 0;

      if (0 ==m2mb_fs_stat(loc_file.fnm, &st))
      {
        AZX_LOG_DEBUG("\r\nLocal file %s size: %u\r\n", loc_file.fnm,  st.st_size);
        loc_file.fsz = st.st_size;
      }
      else
      {
        AZX_LOG_ERROR("Cannot get local file %s size.\r\n", loc_file.fnm);
        task_status = APPLICATION_EXIT;
        break;
      }


      cb_opts.cbFunc = log_progress;
      cb_opts.cbArg = &loc_file;    /* Pass the file name and size as parameter */
      cb_opts.idleTime = 2000;      /* Call the callback function every 2 seconds if there were no data exchange */
      cb_opts.bytesXferred = 8*1024;  /* Call the callback function every 8 kBytes data exchange */

      azx_ftp_setCallback(&cb_opts, ftp_client);

      local.type = AZX_FTP_XFER_FILE;             /* Define the local recipient as file */
      local.payload.fileInfo.path = loc_file.fnm; /* Define the local file path */

      memset(filename_buffer, 0, sizeof(filename_buffer));
      sprintf(filename_buffer, "%s/m2mb_test.txt", gREMOTE_FOLDER);

      AZX_LOG_INFO("\r\nStarting upload of local file %s\r\n", local.payload.fileInfo.path);
      
      ret = azx_ftp_put(&(local), filename_buffer, AZX_FTP_BINARY, ftp_client);

      if (ret == 1)
      {
        AZX_LOG_INFO("Upload successful.\r\n");
      }
      else
      {
        AZX_LOG_ERROR("Failed upload.. \r\n");
        task_status = APPLICATION_EXIT;
        break;
      }
      azx_ftp_clearCallback(ftp_client);

    }

    char localPath[100];
    sprintf(localPath, "%s/_%s",LOCALPATH,gDLFILE);
    m2mb_fs_unlink(localPath);

    /* =======================
     *
     *  DOWNLOAD TO BUFFER
     *
     * ======================*/

    {
      AZX_FTP_XFER_T local;
      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      memset(filename_buffer, 0, sizeof(filename_buffer));
      sprintf(filename_buffer, "%s/%s", gREMOTE_FOLDER, gDLTOBUF_FILE);

      rem_file.fnm = (char*)filename_buffer;

      AZX_LOG_INFO("\r\nStarting download of remote file %s into local buffer\r\n", rem_file.fnm);

      AZX_LOG_INFO("Getting remote file %s size..\r\n", rem_file.fnm);
      ret = azx_ftp_size(rem_file.fnm, &file_size, AZX_FTP_BINARY, ftp_client);
      if (ret == 1)
      {
        AZX_LOG_DEBUG("Done. File size: %u.\r\n", file_size);
      }
      else
      {
        AZX_LOG_ERROR("failed file size.. error: %s \r\n", azx_ftp_lastResponse(ftp_client));
        task_status = APPLICATION_EXIT;
        break;
      }
      {
        M2MB_OS_RESULT_E OSres;
        MEM_W bytes_available;
        OSres = m2mb_os_memInfo( M2MB_OS_MEMINFO_BYTES_AVAILABLE, &bytes_available );
        if ( OSres != M2MB_OS_SUCCESS )
        {
          AZX_LOG_ERROR("Memory Information bytes available error %d\r\n", OSres);
          task_status = APPLICATION_EXIT;
          break;
        }
        
        if( (file_size + 1)  >= bytes_available)
        {
          AZX_LOG_ERROR("File size cannot be handled! Not enough RAM for buffer\r\n", OSres);
          task_status = APPLICATION_EXIT;
          break;
        }
      }
      azx_ftp_clearCallback(ftp_client);

      cb_opts.cbFunc = log_progress;
      cb_opts.cbArg = &rem_file;
      cb_opts.idleTime = 1000;
      cb_opts.bytesXferred = 1024;

      local.type = AZX_FTP_XFER_BUFF;                                        /* Define the local recipient as buffer */
      local.payload.buffInfo.buffer = (CHAR*) m2mb_os_malloc(file_size + 1); /* Set the local buffer reference that will hold the data */
      local.payload.buffInfo.bufferSize = file_size + 1;                     /* Set the local buffer size */

      local.payload.buffInfo.buf_cb = buf_data_cb; /* The data callback to be executed. Set as NULL if not required */

      if(! local.payload.buffInfo.buffer)
      {
        AZX_LOG_ERROR("failed allocating buffer.. \r\n");
        task_status = APPLICATION_EXIT;
        break;
      }

      memset(local.payload.buffInfo.buffer, 0, local.payload.buffInfo.bufferSize); /*file_size + 2*/

      azx_ftp_setCallback(&cb_opts, ftp_client);

      AZX_LOG_INFO("Starting download of remote file %s to buffer\r\n", rem_file.fnm );

      ret = azx_ftp_get(&(local), rem_file.fnm , AZX_FTP_BINARY, ftp_client);

      if (ret > 0)
      {
        int i = 0;
        AZX_LOG_DEBUG("Download successful. Received %d bytes<<<\r\n", ret);

        while (i < ret)
        {
          int toprint = (ret - i < 1024)? ret - i: 1024;
          AZX_LOG_INFO("%.*s", toprint, local.payload.buffInfo.buffer + i);
          i +=toprint;
        }
        AZX_LOG_INFO(">>>\r\n");
        m2mb_os_free(local.payload.buffInfo.buffer);
      }
      else
      {
        AZX_LOG_ERROR("Failed download...\r\n");
        task_status = APPLICATION_EXIT;
        m2mb_os_free(local.payload.buffInfo.buffer);
        break;
      }
    }

    AZX_LOG_INFO("\r\nFTP quit...\r\n");
    azx_ftp_quit(ftp_client);

    task_status = APPLICATION_EXIT;
    break;

  } while (0);

  if (task_status == APPLICATION_EXIT)
  {
    AZX_LOG_TRACE("Application exit\r\n");

    ret = m2mb_pdp_deactivate(pdpHandle, gPDP_CTX);
    if(ret != M2MB_RESULT_SUCCESS)
    {
      AZX_LOG_ERROR("CANNOT DEACTIVATE PDP\r\n");
      return -1;
    }
    else
    {
      AZX_LOG_TRACE("m2mb_pdp_deactivate returned success \r\n");
    }

    AZX_LOG_DEBUG("Application complete.\r\n");
  }

  return 0;
}
