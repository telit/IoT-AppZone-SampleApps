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


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "ftp_test.h"

#include "azx_ftp.h"

#include "app_cfg.h" /*FOR LOCALPATH define*/

/* Macro =============================================================================*/
#define FTP_ADDR "ftp.telit.com"
#define FTP_PORT AZX_FTP_DEFAULT_PORTNUM

#define FTP_USER "guest0181"
#define FTP_PASS "g2hs3tx6"

#define APN	    "web.omnitel.it"
#define PDP_CTX (UINT8)3

#define REMOTE_FOLDER "/samples"
/*Remote path of file on FTP server to be downloaded as a local file*/
#define DLFILE "pattern_big.txt"

/*Local path where the file will be downloaded is defined in app_cfg.h LOCALPATH variable*/

/*Remote path of file on FTP server to be downloaded in a buffer*/
#define DLTOBUF_FILE "pattern.txt"


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

/* Local function prototypes ====================================================================*/
static UINT32 get_uptime(void);
static const char* get_file_title(const CHAR* path);
static int log_progress(AZX_FTP_NET_BUF_T *ctl, azx_ftp_fsz_t xfered, void *arg);
static INT32 ftp_debug_hk(AZX_FTP_DEBUG_HOOK_LEVELS_E level, const CHAR *function,
    const CHAR *file, INT32 line, const CHAR *fmt, ...);

static void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata);
static void PdpCallback(M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata);

/* Static functions =============================================================================*/

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

    AZX_LOG_DEBUG("Activate PDP with APN %s on cid %d....\r\n", APN, PDP_CTX);
    retVal = m2mb_pdp_activate(pdpHandle, PDP_CTX, (CHAR *) APN, (CHAR *) NULL, (CHAR *) NULL, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
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

    strcpy(gFTP_Address, FTP_ADDR);
    
    /*manage custom server port*/
    if (FTP_PORT != AZX_FTP_DEFAULT_PORTNUM)
    {
      snprintf(gFTP_Address + strlen(gFTP_Address), sizeof(gFTP_Address) - strlen(gFTP_Address),":%u", FTP_PORT);
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
      task_status = APPLICATION_EXIT;
      break;
    }
    azx_sleep_ms(1000);

    ret = azx_ftp_login(FTP_USER,FTP_PASS, ftp_client);
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
     *  Remote SIZE
     *
     * ======================*/

    memset(filename_buffer, 0, sizeof(filename_buffer));
    sprintf(filename_buffer, "%s/%s", REMOTE_FOLDER, DLFILE);

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
      AZX_FTP_XFER_T local;

      azx_ftp_clearCallback(ftp_client);

      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      cb_opts.cbFunc = log_progress;
      cb_opts.cbArg = &rem_file;    /* Pass the file name and size as parameter */
      cb_opts.idleTime = 2000;      /* Call the callback function every 2 seconds if there were no data exchange */
      cb_opts.bytesXferred = 8*1024;  /* Call the callback function every 8 kBytes data exchange */

      local.type = AZX_FTP_XFER_FILE; /* Define the local recipient as file */
      local.payload.fileInfo.path = (char*) LOCALPATH "/_" DLFILE; /* Define the local file path */

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


      loc_file.fnm = (CHAR*) LOCALPATH "/_" DLFILE;
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

      AZX_LOG_INFO("\r\nStarting upload of local file %s\r\n", local.payload.fileInfo.path);
      ret = azx_ftp_put(&(local), REMOTE_FOLDER "/m2mb_test.txt", AZX_FTP_BINARY, ftp_client);

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

    m2mb_fs_unlink(LOCALPATH "/_" DLFILE);

    /* =======================
     *
     *  DOWNLOAD TO BUFFER
     *
     * ======================*/

    {
      AZX_FTP_XFER_T local;
      memset(&local, 0, sizeof(AZX_FTP_XFER_T));

      memset(filename_buffer, 0, sizeof(filename_buffer));
      sprintf(filename_buffer, "%s/%s", REMOTE_FOLDER, DLTOBUF_FILE);

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

    ret = m2mb_pdp_deactivate(pdpHandle, PDP_CTX);
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
