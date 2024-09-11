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
#include "m2mb_info.h"
#include "m2mb_fota.h"
#include "m2mb_power.h"

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

#define FOTA

struct REMFILE {
  struct REMFILE *next;
  azx_ftp_fsz_t fsz;
  char *fnm;
};

typedef enum {

  FOTA_ERROR = -1,
  FOTA_OK = 1,
  FOTA_DELTA_OK = 2,
  FOTA_DELTA_ERROR = 3


} FOTA_RESULT_E;

typedef struct {
  UINT8 fotaFlag;
  CHAR* fwVer;
} FOTA_UP_INFO;

#define DL_SIZE 16384


/* Local statics ================================================================================*/

static char filename_buffer[128];

static M2MB_PDP_HANDLE pdpHandle;

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

static AZX_FTP_NET_BUF_T *ftp_client = NULL;

struct REMFILE rem_file;
struct REMFILE loc_file;

static char gFTP_Address[512];


//SSL variables
M2MB_SSL_CONFIG_HANDLE sslConfigHndl = NULL;
M2MB_SSL_CTXT_HANDLE sslCtxtHndl = NULL;

//FOTA variables
static M2MB_FOTA_HANDLE g_FotaHandle = NULL;
static UINT32 gBlockSize;
static UINT8 *fBuffer;
int nBlocks = 0;
static FOTA_UP_INFO fotaUPInfo;
CHAR currFWver[150];
static UINT32 fwVerSize = 100;
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

static FOTA_RESULT_E getFotaInfo(void)
{
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  M2MB_FOTA_RESULT_CODE_E res_code;
  UINT32 partitionSize;

  retVal = m2mb_fota_update_package_info_get(g_FotaHandle, &gBlockSize, &partitionSize);
  if(retVal != M2MB_RESULT_SUCCESS)
  {
    m2mb_fota_result_code_get(g_FotaHandle, &res_code);
    AZX_LOG_ERROR("Info get failed with code %d\r\n", res_code);
    return FOTA_ERROR;
  }


  AZX_LOG_INFO("OTA blockSize: %d\r\n", gBlockSize);
  AZX_LOG_INFO("OTA partitionSize: %d\r\n", partitionSize);
  return FOTA_OK;
}

M2MB_RESULT_E getModuleFwVersion(void)
{
  M2MB_INFO_HANDLE hInfo;
  M2MB_RESULT_E ret;
  CHAR *tempFWver;

  if(m2mb_info_init(&hInfo) != M2MB_RESULT_SUCCESS)
  {
    AZX_LOG_ERROR("m2mb_info_init FAIL\r\n");
    return M2MB_RESULT_FAIL;
  }

  ret = m2mb_info_get(hInfo, M2MB_INFO_GET_SW_VERSION, &tempFWver);
  if(ret != M2MB_RESULT_SUCCESS)
  {
    AZX_LOG_ERROR("m2mb_info_get FAIL\r\n");
    return M2MB_RESULT_FAIL;
  }
  else
  {
    AZX_LOG_INFO("Module current fw version is: \r\n%s\r\n", tempFWver);
    strcpy(currFWver, tempFWver);
    azx_sleep_ms(500);
    m2mb_info_deinit(hInfo);
  }
  return M2MB_RESULT_SUCCESS;
}

M2MB_RESULT_E readFOTAStatusFile(const CHAR *path)
{
  CHAR tmpBuff[150];
  CHAR *start;
  CHAR* end;
  SSIZE_T nBytes;
  UINT8 nVer;
  INT32 fd;

  AZX_LOG_TRACE("Read info file\r\n");

  memset(tmpBuff,0,sizeof(tmpBuff));
  fd = m2mb_fs_open(path, M2MB_O_RDWR);
  if(fd >= 0)
  {
    nBytes = m2mb_fs_read(fd, tmpBuff, sizeof(tmpBuff));
    if (nBytes < 0)
    {
      AZX_LOG_ERROR("reading failure\r\n");
      return M2MB_RESULT_FAIL;
    }
    if ( tmpBuff[0] >= '0' && tmpBuff[0] <='9')
    {
      fotaUPInfo.fotaFlag = tmpBuff[0] - '0';
      //AZX_LOG_INFO("******* fotaUPInfo.fotaFlag: %d    ******\r\n", fotaUPInfo.fotaFlag);
    }
    else
    {
      AZX_LOG_ERROR("No valid Flag value\r\n");
    }

    start = strstr(tmpBuff, "*");
    if (start == NULL)
    {
      AZX_LOG_ERROR("no fw version found\r\n");
    }
    else
    {
      end = strstr(start + 1, "*");
      if (end != NULL)
      {
        nVer= end - (start + 1);
        strncpy(fotaUPInfo.fwVer, start + 1, nVer);
        AZX_LOG_INFO("\r\nFOTA Flag: %d Module previous fw version: \r\n%s\r\n", fotaUPInfo.fotaFlag, fotaUPInfo.fwVer);
      }
      else
      {
        AZX_LOG_ERROR("fw version not correctly stored\r\n");
      }
    }

    m2mb_fs_close(fd);
    return M2MB_RESULT_SUCCESS;
  }
  else
  {
    AZX_LOG_INFO("\r\nFile not present FOTA never done\r\n");
    return M2MB_RESULT_FAIL;
  }
}


M2MB_RESULT_E writeFOTAStatusFile(const CHAR *path, BOOLEAN fotaFlag, CHAR *fwVer)
{
  CHAR tmpBuff[100];
  SSIZE_T nBytes;
  INT32 fd;

  fd = m2mb_fs_open(path, M2MB_O_RDWR);
  if(fd >= 0)
  {
    memset(tmpBuff,0,sizeof(tmpBuff));
    sprintf(tmpBuff, "%d*%s*", fotaFlag, fwVer);
    nBytes = m2mb_fs_write(fd, tmpBuff, strlen(tmpBuff));
    if (nBytes < 0)
    {
      AZX_LOG_ERROR("Writing failure\r\n");
      m2mb_fs_close(fd);
      return M2MB_RESULT_FAIL;
    }
    m2mb_fs_close(fd);
    return M2MB_RESULT_SUCCESS;
  }
  else
  {
    return M2MB_RESULT_FAIL;
  }
}

M2MB_RESULT_E FOTAStatusInit(const CHAR *path)
{
  INT32 fd;
  //AZX_LOG_INFO("\r\nCheck FOTA upgrade status in file system\r\n\r\n");

  memset(currFWver,0,sizeof(currFWver));
  getModuleFwVersion(); //get current module fw version

  fotaUPInfo.fwVer = (CHAR*) m2mb_os_malloc(fwVerSize + 1);
  memset(fotaUPInfo.fwVer,0,fwVerSize + 1);

  fd = m2mb_fs_open(path, M2MB_O_RDWR); /*Open in read only mode*/
  if(fd == -1)
  {
    AZX_LOG_WARN("File doesn't exist create it, first app execution\r\n");
    //create file
    fd = m2mb_fs_open(path, M2MB_O_CREAT | M2MB_O_RDWR);
    if(fd == -1)
    {
      AZX_LOG_ERROR("File creation failed!\r\n");
      return M2MB_RESULT_FAIL;
    }
    else
    {
      AZX_LOG_INFO("File created, store current fw version and fota upgrade flag=0\r\n");
      //set default parameters
      fotaUPInfo.fotaFlag=0;
      strcpy(fotaUPInfo.fwVer, currFWver);
      //close file
      m2mb_fs_close(fd);
      //write content
      azx_sleep_ms(500);
      writeFOTAStatusFile(path, fotaUPInfo.fotaFlag, fotaUPInfo.fwVer);

    }

  }
  else
  {
    AZX_LOG_INFO("File exists\r\n");
    m2mb_fs_close(fd);
  }

  return M2MB_RESULT_SUCCESS;
}




static void FOTAIndCallBack(M2MB_FOTA_HANDLE h, M2MB_FOTA_IND_E fota_event, UINT16 resp_size, void *resp_struct, void *userdata)
{
  (void) h;
  (void) resp_size;
  (void) resp_struct;
  (void) userdata;

  AZX_LOG_DEBUG(">>>>>>>>>>>>>>fota_event: %d<<<<<<<<<<<<<<<\r\n", fota_event);

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

INT32 buf_data_cb_OTA(CHAR *data, UINT32 datalen, INT32 ev)
{
  static UINT32 totData=0;
  static UINT8 n=0;
  M2MB_FOTA_RESULT_CODE_E res_code;
  M2MB_RESULT_E retVal;

  switch(ev)
  {
    case DATA_CB_START:
      /* Do some initialization before receiving data (e.g. open a file descriptor) */
      AZX_LOG_DEBUG("Received START event\r\n");
      break;
    case DATA_CB_DATA:
      /* Manage the data */
      AZX_LOG_DEBUG("Received DATA: %u bytes\r\n", datalen);
      if ((totData + datalen) < gBlockSize)
      {
        memcpy(fBuffer + totData, data, datalen);
        totData+=datalen;
      }
      else
      {
        memcpy(fBuffer + totData, data, (gBlockSize - totData));
        AZX_LOG_DEBUG("Write on FOTA partition, block n. %d\r\n", n);

        retVal = m2mb_fota_update_package_write(g_FotaHandle, n * gBlockSize, fBuffer);
        if(retVal != M2MB_RESULT_SUCCESS)
        {
          m2mb_fota_result_code_get(g_FotaHandle, &res_code);
          AZX_LOG_ERROR("Write partition failed with code %d at block %d\r\n", res_code, n);
          return FOTA_ERROR;
        }

        n++;
        memset(fBuffer,0,gBlockSize);
        memcpy(fBuffer, data + (gBlockSize - totData), totData + datalen - gBlockSize);
        totData = totData + datalen - gBlockSize;
      }

      AZX_LOG_DEBUG("Total data: %u r\n", totData);

      break;
    case DATA_CB_END:
      /* Finalize */
      AZX_LOG_DEBUG("Received END event\r\n");
      UINT32 remainingBytes;
      remainingBytes = datalen - (gBlockSize * nBlocks);
      AZX_LOG_DEBUG("remainingBytes: %u r\n", remainingBytes);

      retVal = m2mb_fota_update_package_write(g_FotaHandle, n * gBlockSize, fBuffer);
      if(retVal != M2MB_RESULT_SUCCESS)
      {
        m2mb_fota_result_code_get(g_FotaHandle, &res_code);
        AZX_LOG_ERROR("Write partition failed with code %d at block %d\r\n", res_code, n);
        return FOTA_ERROR;
      }
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

static void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
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
        AZX_LOG_DEBUG("Module is registered to network\r\n");
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


    AZX_LOG_DEBUG("Waiting for registration...\r\n");

    retVal = m2mb_net_get_reg_status_info(h);
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
    }

    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

#ifdef FOTA
    AZX_LOG_INFO("\r\nCheck FOTA upgrade status in file system\r\n\r\n");
    if (FOTAStatusInit(gFOTA_STATUS_FILE) == M2MB_RESULT_FAIL)
    {
      AZX_LOG_ERROR("Impossible to init FOTA upgrade status file\r\n");
    }
    else
    {
      azx_sleep_ms(500);
      AZX_LOG_INFO("\r\nRead stored data\r\n");
      readFOTAStatusFile(gFOTA_STATUS_FILE);

      if(fotaUPInfo.fotaFlag == 0)
      {
        UINT8 i;
        for(i=0; i < strlen(currFWver); i++)
        {
          //AZX_LOG_INFO("fotaUPInfo.fwVer[%d] = %c, currFWver[%d] = %c\r\n", i, fotaUPInfo.fwVer[i], i, currFWver[i]);
          if(fotaUPInfo.fwVer[i] != currFWver[i])
          {
            AZX_LOG_INFO("Already upgraded\r\n");
            break;
          }
        }

        if (i == strlen(currFWver))
        {
          AZX_LOG_INFO("Fw to be upgraded...\r\n");
        }

      }
      else if(fotaUPInfo.fotaFlag == 1)
      {
        if(strcmp(fotaUPInfo.fwVer, currFWver) == 0)
        {
          AZX_LOG_INFO("FOTA not successful, still old fw verison...\r\n");
        }
        else
        {
          AZX_LOG_INFO("FOTA process successful!\r\n");

          fotaUPInfo.fotaFlag = 0;
          writeFOTAStatusFile(gFOTA_STATUS_FILE, fotaUPInfo.fotaFlag, currFWver);
          m2mb_os_free(fotaUPInfo.fwVer);
          fotaUPInfo.fwVer = NULL;
          return 0;    //just do nothing
        }
      }
    }
#endif


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

    opts.level = AZX_FTP_DEBUG_HOOK_DEBUG;
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
     *  DOWNLOAD TO BUFFER
     *
     * ======================*/

    {
      AZX_FTP_XFER_T local;
      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      memset(filename_buffer, 0, sizeof(filename_buffer));
      sprintf(filename_buffer, "%s/%s", gREMOTE_FOLDER, gDLTOBUF_FILE);

      rem_file.fnm = (char*)filename_buffer;

      //AZX_LOG_INFO("\r\nStarting download of remote file %s into local buffer\r\n", rem_file.fnm);

      retVal = m2mb_fota_init(&g_FotaHandle, FOTAIndCallBack, NULL);
      if(retVal != M2MB_RESULT_SUCCESS)
        {
          AZX_LOG_ERROR("m2mb_fota_init FAIL\r\n");
          return FOTA_ERROR;
        }
      else
       {
         AZX_LOG_DEBUG("m2mb_fota_init success\r\n");
       }

      AZX_LOG_DEBUG("\r\nGet block and FOTA partition size\r\n");
      if(getFotaInfo() != FOTA_OK)
      {
        AZX_LOG_ERROR("Failed fota info!\r\n");
        m2mb_fota_deinit(g_FotaHandle);
        return FOTA_ERROR;
      }

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

        getFotaInfo();
        AZX_LOG_INFO("Fota Block size is %d...download block size", gBlockSize);
        fBuffer = (UINT8 *) m2mb_os_malloc(gBlockSize);
        AZX_LOG_ERROR("Split the download in chunks of %d Bytes\r\n", gBlockSize);
        memset(fBuffer, 0, gBlockSize);
        nBlocks = file_size / gBlockSize;

      }
      azx_ftp_clearCallback(ftp_client);

      cb_opts.cbFunc = log_progress;
      cb_opts.cbArg = &rem_file;
      cb_opts.idleTime = 1000;
      cb_opts.bytesXferred = 1024;

      local.type = AZX_FTP_XFER_BUFF;                                        /* Define the local recipient as buffer */
#ifdef FOTA
      local.payload.buffInfo.buffer = (CHAR*) m2mb_os_malloc(DL_SIZE + 1); /* Set the local buffer reference that will hold the data */
      local.payload.buffInfo.bufferSize = DL_SIZE + 1;
#else
      local.payload.buffInfo.buffer = (CHAR*) m2mb_os_malloc(file_size + 1); /* Set the local buffer reference that will hold the data */
      local.payload.buffInfo.bufferSize = file_size + 1;                     /* Set the local buffer size */
#endif
      local.payload.buffInfo.buf_cb = buf_data_cb_OTA; /* The data callback to be executed. Set as NULL if not required */

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

        AZX_LOG_DEBUG("Download successful. Received %d bytes<<<\r\n", ret);
#ifdef FOTA

        AZX_LOG_DEBUG("Check FOTA delta\r\n");
        if(g_FotaHandle != NULL)
        {
          if(m2mb_fota_update_package_check_setup( g_FotaHandle, M2MB_FOTA_CHECK_SETUP_SOURCE ) != M2MB_RESULT_SUCCESS)
          {
            AZX_LOG_ERROR("Fota check integrity FAIL\r\n");
            task_status = APPLICATION_EXIT;
            m2mb_fota_deinit(g_FotaHandle);
            m2mb_os_free(local.payload.buffInfo.buffer);
            m2mb_os_free(fBuffer);
            break;
          }
          else
          {
            AZX_LOG_DEBUG("Fota check integrity PASS\r\n");
          }
        }
        else
        {
          AZX_LOG_ERROR("Fota handle not valid!\r\n");
          m2mb_fota_deinit(g_FotaHandle);
          task_status = APPLICATION_EXIT;
          m2mb_os_free(local.payload.buffInfo.buffer);
          m2mb_os_free(fBuffer);
          break;
        }

        AZX_LOG_DEBUG("\r\n--> Start update...\r\n");

        retVal = m2mb_fota_start(g_FotaHandle);
        if(retVal != M2MB_RESULT_SUCCESS)
        {
          AZX_LOG_ERROR("m2mb_fota_start FAIL\r\n");
          m2mb_fota_deinit(g_FotaHandle);
          task_status = APPLICATION_EXIT;
          m2mb_os_free(local.payload.buffInfo.buffer);
          m2mb_os_free(fBuffer);
          break;
        }

        M2MB_POWER_HANDLE h = NULL;

        AZX_LOG_TRACE("m2mb_fota_start PASS\r\n");

        azx_sleep_ms(2000);

        retVal = m2mb_power_init(&h, NULL, NULL);
        if(retVal == M2MB_RESULT_SUCCESS)
        {
          AZX_LOG_DEBUG("\r\nReboot module to start delta deployment\r\n");
          fotaUPInfo.fotaFlag = 1;
          writeFOTAStatusFile(gFOTA_STATUS_FILE, fotaUPInfo.fotaFlag, currFWver);
          //m2mb_fs_close(fd);
          m2mb_os_free(fotaUPInfo.fwVer);
          fotaUPInfo.fwVer = NULL;
          azx_sleep_ms(3000);
          m2mb_power_reboot(h);
        }
        else
        {
          AZX_LOG_ERROR("m2mb_power_init FAIL\r\n");
          m2mb_fota_deinit(g_FotaHandle);
          task_status = APPLICATION_EXIT;
          m2mb_os_free(local.payload.buffInfo.buffer);
          m2mb_os_free(fBuffer);
          m2mb_os_free(fotaUPInfo.fwVer);
          fotaUPInfo.fwVer = NULL;
          break;
        }


#else
        int i = 0;
        while (i < ret)
        {
          int toprint = (ret - i < 1024)? ret - i: 1024;
          AZX_LOG_INFO("%.*s", toprint, local.payload.buffInfo.buffer + i);
          i +=toprint;
        }
        AZX_LOG_INFO(">>>\r\n");
#endif
        m2mb_os_free(local.payload.buffInfo.buffer);
        m2mb_os_free(fBuffer);
      }
      else
      {
        AZX_LOG_ERROR("Failed download...\r\n");
        task_status = APPLICATION_EXIT;
        m2mb_os_free(local.payload.buffInfo.buffer);
        m2mb_os_free(fBuffer);
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
