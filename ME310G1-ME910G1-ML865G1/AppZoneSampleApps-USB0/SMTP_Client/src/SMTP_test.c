/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    SMTP_test.c

  @brief
    The file contains test functionalities for SMTP client

  @details

  @description
    Sample application showing SMTP echo demo with M2MB API. Debug prints on $OUTPUT
  @version 
    1.0.1
  @note

  @author
    Cristina Desogus

  @date
    21/12/2022
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

#include "app_cfg.h"

#include "SMTP_test.h"

#include "smtp.h"

#include "app_cfg.h" /*FOR LOCALPATH define*/

#include "read_parameters.h"


/* Local defines ================================================================================*/



//#define MAIL_CONNECTION_SECURITY SMTP_SECURITY_STARTTLS

#define MAIL_FLAGS               (/*SMTP_DEBUG         |*/ \
    SMTP_NO_CERT_VERIFY) /* Do not verify cert. */
#define MAIL_CAFILE              NULL
#define MAIL_AUTH                SMTP_AUTH_PLAIN



static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;


M2MB_PDP_HANDLE pdpHandle;


/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
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

INT32 M2M_msgSMTPTask(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;


  struct smtp *smtp;
  enum smtp_status_code rc;

  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;

  M2MB_NET_HANDLE h;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;

  int ret;

  void *myUserdata = NULL;

  AZX_LOG_DEBUG("INIT\r\n");

  /*
     Configure APP's parameter with the default value and read the current one from configuration file (if it is present)
   */
  configureParameters();
  readConfigFromFile();

  /*
     Init events handler
   */
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

  AZX_LOG_INFO("Waiting for registration...\r\n");

  /*
     Wait for network registration event to occur (released in NetCallback function).
     Activate PDP context after network registration event.
   */

  retVal = m2mb_net_get_reg_status_info(h);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
  }

  m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY, &curEvBits, M2MB_OS_WAIT_FOREVER);


  AZX_LOG_INFO("Pdp context activation\r\n");
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

  AZX_LOG_DEBUG("Activate PDP with APN %s....\r\n", gAPN);
  retVal = m2mb_pdp_activate(pdpHandle, gPDP_CTX, gAPN, gAPN_UserName, gAPN_Password, M2MB_PDP_IPV4);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR("cannot activate pdp context.\r\n");
  }

  /*Wait for pdp activation event to occur (released in PDPCallback function) */
  m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

  AZX_LOG_INFO("Connecting to SMTP Server...\r\n");
  /*
     Open SMTP connection and start authentication
   */
  azx_gnu_setgCID(gPDP_CTX);
  /*-------------------------------------*/

  rc = smtp_open(gMAIL_SERVER, gMAIL_PORT, SMTP_SECURITY_NONE, (enum smtp_flag)MAIL_FLAGS, MAIL_CAFILE, &smtp);
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }
  azx_sleep_ms(2000);

  rc = smtp_auth(smtp, MAIL_AUTH,  gMAIL_USER, gMAIL_PASS);
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }
  AZX_LOG_INFO("SMTP Client connected!\r\n");

  /*
     Create example e-mail.
   */
  AZX_LOG_INFO("Preparing example's mail...\r\n");
  rc = smtp_address_add(smtp, SMTP_ADDRESS_FROM, gMAIL_FROM, gMAIL_FROM_NAME);
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }

  rc = smtp_address_add(smtp, SMTP_ADDRESS_TO, gMAIL_TO, gMAIL_TO_NAME);
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }

  rc = smtp_header_add(smtp, "Subject", gMAIL_SUBJECT);
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }

  rc = smtp_attachment_add_mem(smtp, "test.txt", "Test email attachment.", ((size_t)(-1)));
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }

  /*
     Send example e-mail and close SMTP connection
   */
  AZX_LOG_INFO("Sending example's mail...\r\n");
  rc = smtp_mail(smtp, gMAIL_BODY);
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }
  AZX_LOG_INFO("Example's mail sent!\r\n");
  rc = smtp_close(smtp);
  if(rc != SMTP_STATUS_OK)
  {
    AZX_LOG_ERROR("SMTP failed: %s\r\n", smtp_status_code_errstr(rc));
    return 1;
  }
  AZX_LOG_INFO("SMTP client closed\r\n");
  /*----------------------------------------*/

  AZX_LOG_DEBUG("Application exit\r\n");

  /*
     Deactivate PDP context
   */
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

  AZX_LOG_INFO("Application complete.\r\n");


  return 0;
}
