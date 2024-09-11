/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application that shows how to set an alarm to wake-up module. Debug prints on AUX UART
  @version 
    1.0.0 <fill here>
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Roberta Galeazzo

  @date
    09/09/20024
*/

/* Include files ================================================================================*/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "m2mb_os_api.h"
#include "m2mb_net.h"
#include "m2mb_rtc.h"
#include "m2mb_power.h"
#include "app_cfg.h"

/* Local defines ================================================================================*/
#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define NEW_REG
#define TIME_WAKEUP 120   //in seconds
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
static M2MB_OS_EV_HANDLE net_evHandle = NULL;
M2MB_NET_HANDLE hNet;
UINT32                  curEvBits;
INT32 rtcfd;
INT32 isRegistered=0;

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

#ifdef NEW_REG
void wait_for_registration(void)
{
M2MB_RESULT_E res;

  AZX_LOG_INFO("\r\nWaiting for registration...\r\n");
  do
  {
    res = m2mb_net_get_reg_status_info(hNet);
    if ( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
    }
    /*Wait for network registration event to occur (released in NetCallback function) */
    m2mb_os_ev_get(net_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
    if(isRegistered)
    {
      break;
    }
    else
    {
      azx_sleep_ms(15000);
    }
  }while (1);

  return;
}
#endif
/*-----------------------------------------------------------------------------------------------*/
void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
  (void)resp_size;
  (void)myUserdata;

  M2MB_NET_REG_STATUS_T *stat_info;


  AZX_LOG_TRACE("net_event: %d\r\n", net_event);
  switch (net_event)
  {

  case M2MB_NET_GET_REG_STATUS_INFO_RESP:
    stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
    if  (stat_info->stat == 1 || stat_info->stat == 5)
    {
      AZX_LOG_INFO("Module is registered!\r\n");
      m2mb_net_get_current_operator_info(h);

      //m2mb_os_ev_set(net_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
    }
    else
    {
#ifdef NEW_REG
      m2mb_os_ev_set(net_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
#else
      azx_sleep_ms(1000);
      m2mb_net_get_reg_status_info(h); //call it again
#endif
    }
    break;

  case M2MB_NET_GET_CURRENT_OPERATOR_INFO_RESP:
    {
      M2MB_NET_GET_CURRENT_OPERATOR_INFO_RESP_T *resp = (M2MB_NET_GET_CURRENT_OPERATOR_INFO_RESP_T*)resp_struct;
      AZX_LOG_INFO("NETWORK OPERATOR (mcc mnc): %d %02d, Rat: %d\r\n", resp->mcc, resp->mnc, resp->rat);
      isRegistered=1;
      m2mb_os_ev_set(net_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
    }
    break;


  default:
    AZX_LOG_TRACE("unexpected net_event: %d\r\n", net_event);
    break;

  }
}
int set_alarm(void)
{
  M2MB_RTC_TIMEVAL_T timeval;
  struct tm * ptm;
  time_t time_val_t;
  M2MB_RTC_TIME_T rtc_time;

  INT32 ret = 0;
  AZX_LOG_INFO("Setting alarm in %d minutes\r\n", TIME_WAKEUP/60);

  ret = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_TIMEVAL, &timeval);
  ret |= m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_SYSTEM_TIME, &rtc_time);


  /*Adding the TIME_WAKEUP seconds to time to set the alarm*/
  timeval.sec += TIME_WAKEUP;


  time_val_t = timeval.sec;
  ptm = gmtime(&time_val_t);
if (ptm != NULL)
{
  rtc_time.year = ptm->tm_year + 1900;
  rtc_time.mon = ptm->tm_mon + 1;
  rtc_time.day = ptm->tm_mday;
  rtc_time.hour = ptm->tm_hour;
  rtc_time.min = ptm->tm_min;
  rtc_time.sec = ptm->tm_sec;

  AZX_LOG_INFO("Alarm will be set at: %d-%02d-%02d, %02d:%02d:%02d\r\n", rtc_time.year, rtc_time.mon, rtc_time.day, rtc_time.hour, rtc_time.min, rtc_time.sec);

  ret |= m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_SET_ALARM_TIME, &rtc_time, 0x01);
  AZX_LOG_TRACE("ret: %d\r\n", ret);
}
else
{ 
   AZX_LOG_ERROR("Impossible to set an alarm\r\n");
   ret = -1;
}
  
  return ret;
}
/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
**************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;
M2MB_OS_RESULT_E        osRes;
M2MB_RESULT_E res;
INT32 retVal;
M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
void *myUserdata = NULL;


M2MB_RTC_TIME_T newTime;

  AZX_LOG_INIT();
	azx_sleep_ms(5000);
	AZX_LOG_INFO( "\r\n Start Alarm demo application. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

	osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "net_pdp_ev"));
	osRes = m2mb_os_ev_init( &net_evHandle, &evAttrHandle );
	if ( osRes != M2MB_OS_SUCCESS )
	{
	  m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
	  AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
	  return;
	}
	else
	{
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
#ifdef NEW_REG
	wait_for_registration();
#else
	AZX_LOG_INFO("\r\nWaiting for registration...\r\n");

	res = m2mb_net_get_reg_status_info(hNet);
	if ( res != M2MB_RESULT_SUCCESS )
	{
	  AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
	}

	/*Wait for network registration event to occur (released in NetCallback function) */
	m2mb_os_ev_get(net_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
#endif
	rtcfd = m2mb_rtc_open("/dev/rtc0",0);
	if (rtcfd != -1)
	{
		AZX_LOG_TRACE( "RTC opened\r\n");
	}
	else
	{

		AZX_LOG_ERROR("Cannot open RTC!");
		return;
	}


	retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_SYSTEM_TIME, &newTime);
	if(retVal == 0)
	{
		AZX_LOG_INFO("Module system time is: %d-%02d-%02d, %02d:%02d:%02d\r\n", newTime.year, newTime.mon, newTime.day, newTime.hour, newTime.min, newTime.sec);
	}
	else
	{
		AZX_LOG_ERROR("Cannot get module System time!");
		return;
	}

  /*
   * Set alarm
   */
	if(newTime.year == 1980)
	{
	  AZX_LOG_ERROR("Default time...please set current date and time manually!\r\n");
	  return;
	}

  set_alarm();

  AZX_LOG_INFO("\r\nWait 5 seconds and then Turn off module...\r\n");
  azx_sleep_ms(5000);

	M2MB_POWER_HANDLE h = NULL;

	  retVal = m2mb_power_init(&h, (m2mb_power_ind_callback) NULL, NULL );
	  if ( retVal == M2MB_RESULT_SUCCESS )
	  {
	    AZX_LOG_INFO( "Power off module");
	    m2mb_rtc_close(rtcfd);
	    m2mb_power_shutdown(h);
	    m2mb_power_deinit(h);
	  }
	  else
	  {
	    AZX_LOG_ERROR("Cannot init power apis \r\n!");
	    return;
	  }
}

