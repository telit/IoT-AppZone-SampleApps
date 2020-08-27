/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*
 * ftp_utils.c
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

#include "ftp_utils.h"
#include "app_utils.h"

#include "azx_ftp.h"



/* Macro =============================================================================*/
#define APN     "internet"
#define PDP_CTX (UINT8)3

#define FTP_ADDR "server"
#define FTP_PORT 21

#define FTP_USER ""
#define FTP_PASS ""

/*Remote path of file on FTP server to be downloaded as a local file */
#define NEW_APP_REM_PATH "/samples/APP_OTA/"

/*name of remote file to be downloaded from server*/
#define NEW_APP_FILE_NAME "m2mapz_v2.bin"




#define CONFIG_FILE "/mod/ota_config.txt"

/*define a the current version app name in the /mod folder*/
#define CURRENT_APP_LOCAL_NAME "m2mapz.bin"

/*define a custom name for the final local file (it might be the same as NEW_APP_FILE_NAME)*/
#define NEW_APP_LOCAL_NAME "m2mapz_v2.bin"




/* Local defines =======================================================================*/
struct REMFILE {
  struct REMFILE *next;
  azx_ftp_fsz_t fsz;
  char *fnm;
};



/* Local statics ================================================================================*/
static char gAPN[100];
static char gFTP_Address[512];
static char gFTP_UserName[100];
static char gFTP_PassWord[100];

static char gRemoteFileURI[256];                /*This is the remote path on the FTP server for the new app file*/

static char gLocalOriginalAppName[128];         /*This is the original running app file name (before OTA)*/
static char gLocalDestinationAppName[128];      /*This is the local file name to be used for downloaded file*/





static char localFilePath[128];


static M2MB_PDP_HANDLE pdpHandle;
static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

static AZX_FTP_NET_BUF_T *ftp_client = NULL;

struct REMFILE rem_file;
struct REMFILE loc_file;

/* Local function prototypes ====================================================================*/
static UINT32 get_uptime(void);
static const char* get_file_title(const CHAR* path);
static int log_progress(AZX_FTP_NET_BUF_T *ctl, azx_ftp_fsz_t xfered, void *arg);
static INT32 ftp_debug_hk(AZX_FTP_DEBUG_HOOK_LEVELS_E level, const CHAR *function,
    const CHAR *file, INT32 line, const CHAR *fmt, ...);

static void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata);
static void PdpCallback(M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata);

static int readConfigFromFile(void);
static char *mystrtok(char **m, char *s, char c);
/* Static functions =============================================================================*/


static char *mystrtok(char **m, char *s, char c)
{
  char *p = s?s:*m;
  if( !*p )
    return 0;
  *m = strchr(p,c);
  if( *m )
    *(*m)++=0;
  else
    *m = p+strlen(p);
  return p;
}


int readConfigFromFile(void)
{

  INT32 fd = -1;
  INT32 fs_res;
  //UINT32 fs_res;
  CHAR recv[512];

  UINT16  ftp_port;

  char *p;

  char *_APN = NULL;
  char *_FTP_URL = NULL;
  char *_FTP_PORT = NULL;
  char *_FTP_USER = NULL;
  char *_FTP_PASS = NULL;
  char *_NEW_APP_REMOTE_URI = NULL;
  char *_NEW_APP_LOCAL_NAME = NULL;
  char *_CURRENT_APP_NAME = NULL;

  AZX_LOG_DEBUG("Reading parameters from file\r\n");

  AZX_LOG_DEBUG("Opening %s in read mode..\r\n", CONFIG_FILE);

  fd = m2mb_fs_open(CONFIG_FILE,M2MB_O_RDONLY);

  if(fd != -1)
  {
    memset(recv, 0, sizeof(recv));
    fs_res = m2mb_fs_read(fd, recv, sizeof(recv));

    AZX_LOG_TRACE("Received %d bytes from file: \r\n<%.*s> \r\n", fs_res, fs_res, recv);
    azx_sleep_ms(200);

    AZX_LOG_TRACE("Closing file.\r\n");
    m2mb_fs_close(fd);
    azx_sleep_ms(2000);

    //Using mystrtok function to separate #tag from the setting file

    //APN
    _APN = mystrtok(&p, recv,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_URL
    _FTP_URL = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_PORT
    _FTP_PORT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_USER
    _FTP_USER = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_PASS
    _FTP_PASS = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //NEW_APP_REMOTE_URI
    _NEW_APP_REMOTE_URI = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //NEW_APP_LOCAL_NAME
    _NEW_APP_LOCAL_NAME = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description


    //CURRENT_APP_NAME
    _CURRENT_APP_NAME = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    strcpy(gAPN, _APN);
    strcpy(gFTP_Address, _FTP_URL);

    ftp_port = atoi(_FTP_PORT);
    if (ftp_port != AZX_FTP_DEFAULT_PORTNUM)
    {
      snprintf(gFTP_Address + strlen(gFTP_Address), sizeof(gFTP_Address) - strlen(gFTP_Address),":%u", ftp_port);
    }
    strcpy(gFTP_UserName, _FTP_USER);
    strcpy(gFTP_PassWord, _FTP_PASS);
    strcpy(gRemoteFileURI, _NEW_APP_REMOTE_URI);
    strcpy(gLocalDestinationAppName, _NEW_APP_LOCAL_NAME);
    strcpy(gLocalOriginalAppName, _CURRENT_APP_NAME);


    AZX_LOG_INFO("Set APN to: <<%s>>\r\n", gAPN);
    AZX_LOG_INFO("Set FTP URL to: <<%s>>\r\n", gFTP_Address);
    AZX_LOG_INFO("Set FTP PORT to: %u\r\n", ftp_port);
    AZX_LOG_INFO("Set FTP USER to: <<%s>>\r\n", gFTP_UserName);
    AZX_LOG_INFO("Set FTP PASS to: <<%s>>\r\n", gFTP_PassWord);
    AZX_LOG_INFO("Set FTP FILE URI to: <<%s>>\r\n", gRemoteFileURI);
    AZX_LOG_INFO("Set LOCAL FINAL APP NAME to: <<%s>>\r\n", gLocalDestinationAppName);
    AZX_LOG_INFO("Set LOCAL ORIGINAL APP NAME to: <<%s>>\r\n", gLocalOriginalAppName);
    return 1;
  }
  else
  {
    return -1;
  }
}



static void configureParameters(void)
{
  strcpy(gAPN, APN);
  strcpy(gFTP_Address, FTP_ADDR);

  if (FTP_PORT != AZX_FTP_DEFAULT_PORTNUM)
  {
    snprintf(gFTP_Address + strlen(gFTP_Address), sizeof(gFTP_Address) - strlen(gFTP_Address),":%u", FTP_PORT);
  }

  strcpy(gFTP_UserName, FTP_USER);
  strcpy(gFTP_PassWord, FTP_PASS);
  strcpy(gRemoteFileURI, NEW_APP_REM_PATH NEW_APP_FILE_NAME);
  strcpy(gLocalOriginalAppName, CURRENT_APP_LOCAL_NAME);
  strcpy(gLocalDestinationAppName, NEW_APP_LOCAL_NAME);
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
    offset = 0;
    break;
  case AZX_FTP_DEBUG_HOOK_DEBUG:
    offset = sprintf(buf, "%5u.%03u %6s - %15s:%-4d - %32s - ",
        now / 1000, now % 1000,
        "DBG - ",
        get_file_title(file), line,
        function);
    break;
  default:
    offset = 0;
    break;
  }
  va_start(arg, fmt);
  vsnprintf(buf + offset, bufSize-offset, fmt, arg);
  va_end(arg);

  return AZX_LOG_INFO(buf);
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

    AZX_LOG_DEBUG("Activate PDP with APN %s on cid %d....\r\n", gAPN, PDP_CTX);
    retVal = m2mb_pdp_activate(pdpHandle, PDP_CTX, (CHAR *) gAPN, (CHAR *) NULL, (CHAR *) NULL, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("cannot activate pdp context.\r\n");
    }

    /*Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);



    AZX_LOG_INFO("Start ftp client...\r\n");

    opts.level = AZX_FTP_DEBUG_HOOK_ERROR;
    opts.cbFunc = ftp_debug_hk;
    opts.cid = PDP_CTX;

    ret = azx_ftp_init(&opts);
    if (ret != 1)
    {
      AZX_LOG_ERROR("failed initializing ftp_client... \r\n");
      return -1;
    }


    ret = azx_ftp_connect(gFTP_Address, &ftp_client);
    if (ret == 1)
    {
      AZX_LOG_DEBUG("Connected.\r\n");
    }
    else
    {
      AZX_LOG_ERROR("failed connecting.. \r\n");
      task_status = APPLICATION_EXIT;
      break;
    }
    azx_sleep_ms(1000);

    ret = azx_ftp_login(gFTP_UserName, gFTP_PassWord, ftp_client);
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

    /* =======================
     *
     *  Remote SIZE
     *
     * ======================*/

    rem_file.fnm = (char*)gRemoteFileURI;
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
     *  DOWNLOAD
     *
     * ======================*/
    {
      AZX_FTP_XFER_T local;

      azx_ftp_clearCallback(ftp_client);

      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      cb_opts.cbFunc = log_progress;
      cb_opts.cbArg = &rem_file;    /* Pass the file name and size as parameter */
      cb_opts.idleTime = 1000;      /* Call the callback function every second if there were no data exchange */
      cb_opts.bytesXferred = 4096;  /* Call the callback function every 4kBytes data exchange */

      local.type = AZX_FTP_XFER_FILE; /* Define the local recipient as file */
      sprintf(localFilePath, "/mod/%s", gLocalDestinationAppName);
      local.payload.fileInfo.path = localFilePath; /* Define the local file path */

      azx_ftp_setCallback(&cb_opts,ftp_client);

      AZX_LOG_INFO("Starting download of remote file %s into local %s\r\n", gRemoteFileURI, local.payload.fileInfo.path);

      ret = azx_ftp_get(&(local), gRemoteFileURI, AZX_FTP_BINARY, ftp_client);

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



    AZX_LOG_INFO("FTP quit...\r\n");
    azx_ftp_quit(ftp_client);

    task_status = DOWNLOAD_EXIT;
    break;

  } while (0);

  if ((task_status == APPLICATION_EXIT) || (task_status == DOWNLOAD_EXIT))
  {
    AZX_LOG_DEBUG("Deactivating PDP\r\n");

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

  }

  if(task_status == DOWNLOAD_EXIT)
  {
    /*send AT command or m2mb_appMgr*/
    update_app(gLocalDestinationAppName, gLocalOriginalAppName);

  }

  return 0;
}
