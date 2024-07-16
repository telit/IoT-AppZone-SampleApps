/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
   m2mb_ntp.c

  @brief
    The file contains the NTP task

  @details

  @description
    The file contains NTP task that handles NTP query and module internal clock setting
  @version
    1.0.4
  @note

  @author
	Roberta Galeazzo
  @date
    08/04/2021
*/
/* Include files ================================================================================*/

/* Include files ================================================================================*/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"
#include "m2mb_rtc.h"
#include "m2mb_ntp.h"
#include "m2mb_net.h"
#include "m2mb_pdp.h"
#include "m2mb_socket.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"

#include "m2mb_fs_posix.h"

#include "ntp_types.h"
#include "app_cfg.h"

#include "read_parameters.h"
/* Local defines ================================================================================*/
#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define EV_PDP_BIT         (UINT32)0x2    /*0x0000000000000010*/
#define EV_NTP_BIT         (UINT32)0x4    /*0x0000000000000100*/
/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

static M2MB_OS_EV_HANDLE net_pdp_evHandle = NULL;
M2MB_PDP_HANDLE pdpHandle;
M2MB_NTP_HANDLE hNtp;
M2MB_NET_HANDLE hNet;

static const char *Weekdays[] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
};

extern INT32 ntpTask;
/* Local function prototypes ====================================================================*/

/* Static functions =============================================================================*/

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

/* Global functions =============================================================================*/

void m2mb_ntp_ind_callback ( M2MB_NTP_HANDLE h,
        M2MB_NTP_EVENTS_E ntp_event,
        UINT16 resp_size,
        void *resp_struct,
        void *cb_args ){
  (void)resp_size;
  (void)h;
  (void)cb_args;

  struct M2MB_SOCKET_BSD_TIMEVAL *tv;
  time_t current_time;
  struct tm *curDate;

  switch(ntp_event)
  {
    case M2MB_NTP_VALID_TIME:
    {
      tv = (struct M2MB_SOCKET_BSD_TIMEVAL *) resp_struct;
      current_time = tv->m_tv_sec;
      AZX_LOG_TRACE("Received UNIX timestamp: %u\r\n", current_time);
      m2mb_os_ev_set(net_pdp_evHandle, EV_NTP_BIT, M2MB_OS_EV_SET);
      curDate = localtime(&current_time);
      if(curDate)
      {
        AZX_LOG_INFO("\r\nCurrent time is: %s %d-%02d-%02d, %02d:%02d:%02d\r\n",
                Weekdays[curDate->tm_wday],
                curDate->tm_year + 1900,
                curDate->tm_mon+1,
                curDate->tm_mday,
                curDate->tm_hour,
                curDate->tm_min,
                curDate->tm_sec);
      }
      /*set module RTC */
      azx_tasks_sendMessageToTask(ntpTask, SET_MODULE_RTC, (INT32)current_time, 0 );

    }
    break;

    default:
      AZX_LOG_ERROR("ntp error %d\r\n", ntp_event);
      break;
  }
}

/*-----------------------------------------------------------------------------------------------*/
void PdpCallback(M2MB_PDP_HANDLE h, M2MB_PDP_IND_E pdp_event, UINT8 cid, void *userdata)
{
  (void)userdata;
  struct M2MB_SOCKET_BSD_SOCKADDR_IN CBtmpAddress;

  CHAR CBtmpIPaddr[32];

  switch (pdp_event)
  {
    case M2MB_PDP_UP:
      m2mb_pdp_get_my_ip(h, cid, M2MB_PDP_IPV4, &CBtmpAddress.sin_addr.s_addr);
      m2mb_socket_bsd_inet_ntop( M2MB_SOCKET_BSD_AF_INET, &CBtmpAddress.sin_addr.s_addr, ( CHAR * )&( CBtmpIPaddr ), sizeof( CBtmpIPaddr ) );
      AZX_LOG_INFO( "Context activated, IP address: %s\r\n", CBtmpIPaddr);

      m2mb_os_ev_set(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_SET);
      break;

    case M2MB_PDP_DOWN:
      AZX_LOG_TRACE("Context successfully deactivated!\r\n");
      break;
    default:
      AZX_LOG_TRACE("unexpected pdp_event: %d\r\n", pdp_event);
      break;

  }
}
/*-----------------------------------------------------------------------------------------------*/

INT32 NTP_task(INT32 type, INT32 param1, INT32 param2)
{
  M2MB_RESULT_E res;
  INT32 retVal;
  void *userContext = NULL; //user provided context data

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;
  void *myUserdata = NULL;


  (void)type;
  (void)param1;
  (void)param2;

  configureParameters(); /*set default values first*/
  readConfigFromFile(); /*try to read configuration from file (if present)*/


  switch(type)
  {
    case INIT:
    {
      osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "net_pdp_ev"));
      osRes = m2mb_os_ev_init( &net_pdp_evHandle, &evAttrHandle );

      if ( osRes != M2MB_OS_SUCCESS ){
        m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
        AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
        return -1;
      } else {

        AZX_LOG_TRACE("m2mb_os_ev_init success\r\n");

      }

      res = m2mb_net_init(&hNet, NetCallback, myUserdata);
      if ( res == M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_TRACE( "m2mb_net_init returned M2MB_RESULT_SUCCESS\r\n");
      }
      else
      {
        AZX_LOG_ERROR( "m2mb_net_init did not return M2MB_RESULT_SUCCESS\r\n" );
      }


      retVal = m2mb_net_enable_ind(hNet, M2MB_NET_REG_STATUS_IND, 1);
      if ( retVal != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR( "m2mb_net_enable_ind failed\r\n" );
        return 1;
      }

      AZX_LOG_INFO("\r\nWaiting for registration...\r\n");

      res = m2mb_net_get_reg_status_info(hNet);
      if ( res != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
      }

      /*Wait for network registration event to occur (released in NetCallback function) */
      m2mb_os_ev_get(net_pdp_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY, &curEvBits, M2MB_OS_WAIT_FOREVER);

      /*Init NTP*/
      res = m2mb_ntp_init(&hNtp, m2mb_ntp_ind_callback, (void*)userContext);
      if(M2MB_RESULT_SUCCESS != res){
        AZX_LOG_ERROR("NTP init failed\r\n");
        return -1;
      }

      res = m2mb_ntp_setCid(hNtp, gPDP_CTX);
      if(M2MB_RESULT_SUCCESS != res) {

        AZX_LOG_ERROR("Set cid failed.\r\n");

        res = m2mb_ntp_deinit(hNtp);
        if(res == M2MB_RESULT_SUCCESS){
          AZX_LOG_TRACE("Deinit OK\r\n");
        }
        return -1;
      }

      res = m2mb_ntp_setCfg(hNtp, M2MB_NTP_CFG_SERVER, (void*)gNTP_SERVER);
      if(M2MB_RESULT_SUCCESS != res){
        AZX_LOG_ERROR("Set server failed.\r\n");

        res = m2mb_ntp_deinit(hNtp);

        if(res == M2MB_RESULT_SUCCESS){
          AZX_LOG_TRACE("Deinit OK\r\n");
        }
        return -1;
      }

      res = m2mb_ntp_setCfg(hNtp, M2MB_NTP_CFG_PORT, (void*) &(gNTP_PORT));
      if(M2MB_RESULT_SUCCESS != res){
        AZX_LOG_ERROR("set server port failed.\r\n");

        res = m2mb_ntp_deinit(hNtp);

        if(res == M2MB_RESULT_SUCCESS){
          AZX_LOG_TRACE("Deinit OK\r\n");
        }
        return -1;
      }

      res = m2mb_ntp_setCfg(hNtp, M2MB_NTP_CFG_TIMEOUT, (void*) &(gTIMEOUT));
      if(M2MB_RESULT_SUCCESS != res){
        AZX_LOG_ERROR("set timeout failed.\r\n");

        res = m2mb_ntp_deinit(hNtp);

        if(res == M2MB_RESULT_SUCCESS){
          AZX_LOG_TRACE("Deinit OK\r\n");
        }
        return -1;
      }

      /*CID activation*/

      res = m2mb_pdp_init(&pdpHandle, PdpCallback, myUserdata);
      if (M2MB_RESULT_SUCCESS != res){

        AZX_LOG_ERROR("PDP init failed\r\n");
        return -1;
      }

      azx_sleep_ms(2000);


      AZX_LOG_INFO("\r\nActivate PDP context with APN %s on CID %d\r\n", gAPN, gPDP_CTX);
      res = m2mb_pdp_activate(pdpHandle, gPDP_CTX, (CHAR*)gAPN, (CHAR*)gAPN_UserName, (CHAR*)gAPN_Password, M2MB_PDP_IPV4); //activates PDP_CTX with APN and IP type IPV4
      if ( res != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("cannot activate pdp context.\r\n");
      }

      /*Wait for pdp activation event to occur (released in PDPCallback function) */
      m2mb_os_ev_get(net_pdp_evHandle, EV_PDP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

      AZX_LOG_INFO("Get current time from server %s, PORT: %d\r\n", gNTP_SERVER, gNTP_PORT);
      res = m2mb_ntp_queryServerTime( hNtp );

      if(M2MB_RESULT_SUCCESS != res) {

        AZX_LOG_ERROR("NTP server query failed.\r\n");
        return -1;
      }
      /* Wait for ntp callback to complete released in NTPcllback function*/
      m2mb_os_ev_get(net_pdp_evHandle, EV_NTP_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

      res = m2mb_ntp_deinit(hNtp);
      if(res == M2MB_RESULT_SUCCESS){

        AZX_LOG_TRACE("Deinit OK\r\n");
      }else{

        AZX_LOG_ERROR("Cannot deinit! retVal: %d\r\n", res);
      }


    }
    break;

    case SET_MODULE_RTC:
    {
      time_t currTime;
      INT32 rtcfd;
      M2MB_RTC_TIME_T time;

      currTime = (time_t)param1;
      rtcfd = m2mb_rtc_open("/dev/rtc0",0);

      if (rtcfd != -1){

        retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_SET_TIMEVAL, &currTime);
        if(retVal == 0){

          AZX_LOG_INFO("\r\nCurrent time correctly set on module\r\n");

          retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_SYSTEM_TIME, &time);
          if(retVal == 0){

            AZX_LOG_INFO("Module system time is: %d-%02d-%02d, %02d:%02d:%02d\r\n", time.year, time.mon, time.day, time.hour, time.min, time.sec);

          }else{

            AZX_LOG_ERROR("Cannot get module System time!");
          }

        }else{

          AZX_LOG_ERROR("Cannot set current time!");
        }

      } else {

        AZX_LOG_ERROR("Cannot open RTC!");
      }

      m2mb_rtc_close(rtcfd);
    }
    break;

    default:
      break;

  }


  return 0;
}
