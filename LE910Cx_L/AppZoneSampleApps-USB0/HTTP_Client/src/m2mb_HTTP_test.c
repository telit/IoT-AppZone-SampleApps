/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */

/**
  @file
    m2mb_HTTP_test.h

  @brief
    demo related definitions

  @details

  @version 
    1.0.2

  @note
    Dependencies:
    m2mb_types.h

  @author
    Cristina Desogus

  @date
    10/10/2022
 */

/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "stdarg.h"

#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_net.h"
#include "m2mb_pdp.h"

#include "m2mb_socket.h"
#include "m2mb_ssl.h"

#include "m2mb_fs_posix.h"

#include "azx_log.h"
#include "azx_tasks.h"
#include "azx_utils.h"

#include "azx_https.h"
#include "azx_base64.h" //for azx_base64encoder

#include "m2mb_HTTP_test.h"

#include "read_parameters.h"

/* Macro =============================================================================*/

#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define EV_PDP_UP_BIT      (UINT32)0x2    /*0x0000000000000010*/
#define EV_PDP_DOWN_BIT    (UINT32)0x4    /*0x0000000000000100*/


#define USER_SSL_AUTH      M2MB_SSL_NO_AUTH

/* Local defines ================================================================================*/
static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;

static M2MB_PDP_HANDLE pdpHandle;

/* Local statics ================================================================================*/


/* Local function prototypes ====================================================================*/


/* Static functions =============================================================================*/



/* Global functions =============================================================================*/
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
        AZX_LOG_DEBUG("Module is registered to cell 0x%X!\r\n", stat_info->cellID);
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

      m2mb_os_ev_set(net_pdp_evHandle, EV_PDP_UP_BIT, M2MB_OS_EV_SET);
      break;

    case M2MB_PDP_DOWN:
      AZX_LOG_DEBUG ("Context deactivated!\r\n");
      m2mb_os_ev_set(net_pdp_evHandle, EV_PDP_DOWN_BIT, M2MB_OS_EV_SET);
      break;
    default:
      AZX_LOG_DEBUG("unexpected pdp_event: %d\r\n", pdp_event);
      break;

  }
}

static int http_debug_hk(AZX_HTTP_LOG_HOOK_LEVELS_E level, const char *function, const char *file, int line, const char *fmt, ...)
{
  (void)file;

  char buf[512];
  int bufSize = sizeof(buf);
  va_list arg;
  INT32   offset = 0;
  UINT32  now = (UINT32) ( m2mb_os_getSysTicks() * m2mb_os_getSysTickDuration_ms() );

  memset(buf,0,bufSize);

  switch(level)
  {
    case AZX_HTTP_LOG_ERROR:
      offset = sprintf(buf, "%5u.%03u %6s %32s:%-4d -  ",
              now / 1000, now % 1000,
              "ERR - ",
              function,
              line
      );
      break;
    case AZX_HTTP_LOG_INFO:
      break;
    case AZX_HTTP_LOG_DEBUG:
      offset = sprintf(buf, "%5u.%03u %6s %32s:%-4d - ",
              now / 1000, now % 1000,
              "DBG - ",
              function,
              line
      );

      break;
    default:
      return 0;
  }
  va_start(arg, fmt);
  vsnprintf(buf + offset, bufSize-offset, fmt, arg);
  va_end(arg);

  return AZX_LOG_INFO("%s", buf);

}

static int is_printable(const char* buffer, int size)
{
  int i;
  for (i = 0; i < size; i++)
  {
    if (!isprint((unsigned char)(buffer[i])))
    {
      break;
    }
  }
  return !(i < size); /*if any character in the buffer is not printable, i will be < buffer size*/
}

/* Global functions =============================================================================*/
INT32 g_cbArg = 0;

INT32 DATA_CB(void *buffer, UINT32 size, void *cbArg)
{
  (void)cbArg;
  /****
  INT32 CB_COUNT = *(INT32*)cbArg;
  CB_COUNT++;

  // Stop http download on a specific user event
  if(CB_COUNT >= 3)
  {
    AZX_LOG_INFO("\r\nCB COUNT LIMIT REACHED\r\n");

    return 1;
  }
   */

  if(buffer != NULL)
  {
    AZX_LOG_TRACE("Received %d bytes from HTTP client.\r\n", size);

    if(is_printable((const char *)buffer, size))
    {
      AZX_LOG_INFO("Non-printable buffer\r\n");
    }
    else
    {
      AZX_LOG_INFO("%.*s", size, buffer);
    }
  }
  else
  {
    AZX_LOG_TRACE("Transmitted %d bytes to HTTP server\r\n", size);
  }
  /*return != 0 to force an abort of the transmission */
  return 0;
}



int activatePdP(void)
{
  M2MB_NET_HANDLE h;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

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

  retVal = m2mb_net_init(&h, NetCallback, NULL);
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


  m2mb_net_deinit(h);

  AZX_LOG_DEBUG("Pdp context initialization\r\n");
  retVal = m2mb_pdp_init(&pdpHandle, PdpCallback, NULL);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
  }

  azx_sleep_ms(2000);


  AZX_LOG_DEBUG("Activate PDP with APN %s....\r\n", gAPN);
  retVal = m2mb_pdp_activate(pdpHandle, gCID, (char*) gAPN, (char*) gAPN_UserName, (char*) gAPN_Password, M2MB_PDP_IPV4); //activates cid 3 with APN "internet.wind.biz" and IP type IPV4
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("cannot activate pdp context.\r\n");
  }

  /*Wait for pdp activation event to occur (released in PDPCallback function) */
  m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_UP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

  return 0;

}

INT32 M2M_msgHTTPTask(INT32 type, INT32 param1, INT32 param2)
{
  (void) type;
  (void) param1;
  (void) param2;

  int ret;
  AZX_HTTP_OPTIONS opts;

  azx_sleep_ms(1000);

  configureParameters(); /*set default values first*/
  readConfigFromFile(); /*try to read configuration from file (if present)*/


  ret = activatePdP();

  if (ret != 0)
  {
    return -1;
  }

  /*HTTP client configuration*/

  AZX_HTTP_INFO hi;
  AZX_HTTP_SSL  tls;
  azx_httpCallbackOptions cbOpt;

  memset( &opts, 0l, sizeof(AZX_HTTP_OPTIONS) );

  opts.logFunc = http_debug_hk;
  opts.loglevel = AZX_HTTP_LOG_INFO;
  opts.cid = gCID;
  azx_http_initialize(&opts);

  memset(&hi, 0, sizeof(AZX_HTTP_INFO));
  memset(&tls, 0, sizeof(AZX_HTTP_SSL));

  tls.CA_CERT_FILEPATH = (CHAR *) gCACERTFILE;
  tls.CLIENT_CERT_FILEPATH = (CHAR *) gCLIENTCERTFILE;
  tls.CLIENT_KEY_FILEPATH = (CHAR *) gCLIENTKEYFILE;
  tls.sslAuthType = M2MB_SSL_NO_AUTH;

  if(-1 == (ret = azx_http_SSLInit(&tls)))
  {
    AZX_LOG_INFO("SSL init error \r\n");
  }


  cbOpt.user_cb_bytes_size = 1500;
  cbOpt.cbFunc = DATA_CB;
  cbOpt.cbData = m2mb_os_malloc(cbOpt.user_cb_bytes_size + 1); //one more element for \0
  cbOpt.cbArg = &g_cbArg;
  azx_http_setCB(&hi, cbOpt);

  hi.tls = tls;


  switch(gREQUEST_TYPE)
  {
    /******* GET *****/
    case HTTPS_AND_CHUNKED_GET:
    case HTTPS_SERVER_AUTH_GET:
    case HTTP_GET:
      AZX_LOG_INFO("Performing a GET request...\r\n");
      ret = azx_http_get(&hi, (char*) gSERVER);
      break;
    case HTTP_BASIC_AUTH_GET:
      AZX_LOG_INFO("Performing a GET request...\r\n");
      hi.request.auth_type = azx_AuthSchemaBasic;
      hi.user_b64encode = azx_base64Encoder;
      ret = azx_http_get(&hi, (char*) gSERVER);
      break;

    /*******GET RANGE **********/
    case HTTP_RANGE_GET:
    {
      AZX_LOG_INFO("Performing a GET RANGE request...\r\n");
      char respRange[100] = {0};
      hi.request.content_range = (char *)"50-100";
      hi.response.content_range = respRange;
      ret = azx_http_get(&hi, (char*) gSERVER);
      AZX_LOG_INFO("\r\n<%s>\r\n",hi.response.content_range);
      break;
    }
      /******* HEAD *****/
    case HTTP_HEAD:
      AZX_LOG_INFO("HEAD request \r\n");
      ret = azx_http_head(&hi, (char*) gSERVER);
      break;

      /******* POST *****/
    case HTTP_POST:
    {
      AZX_LOG_INFO("POST request \r\n");
      char *data = (char *) "This is expected to be sent";
      hi.request.post_data = (char *)m2mb_os_malloc(sizeof(char)* (strlen(data) + 1));
      memset(hi.request.post_data, 0, sizeof(sizeof(char)* (strlen(data) + 1)));
      strcpy(hi.request.post_data,data);
      hi.request.content_length = strlen(hi.request.post_data);
      ret = azx_http_post(&hi,(char *)gSERVER);
      m2mb_os_free(hi.request.post_data);
      break;
    }
    default:
      AZX_LOG_INFO("Performing a GET request...\r\n");
      ret = azx_http_get(&hi, (char*) gSERVER);
      break;
  }

  m2mb_os_free(cbOpt.cbData);

  AZX_LOG_INFO("\r\nResult: %d\r\n", ret);

  m2mb_pdp_deactivate(pdpHandle, gCID);

  {
    UINT32 curEvBits;
    /*Wait for pdp activation event to occur (released in PDPCallback function) */
    m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_DOWN_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS(10000));
  }

  m2mb_pdp_deinit(pdpHandle);

  return 0;
}

