/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application that shows RTC apis functionalities: how to get/set moudle system time and timestamp. Debug prints on AUX UART
  @version
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
     Roberta Galeazzo

  @date
    11/10/2021
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
#include "app_cfg.h"

/* Local defines ================================================================================*/
#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
static M2MB_OS_EV_HANDLE net_evHandle = NULL;
M2MB_NET_HANDLE hNet;


/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/
void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
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
      AZX_LOG_INFO("Module is registered!\r\n");
      m2mb_os_ev_set(net_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
    }
    else
    {
      m2mb_net_get_reg_status_info(h); //call it again
    }
    break;


  default:
    AZX_LOG_TRACE("unexpected net_event: %d\r\n", net_event);
    break;

  }
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
UINT32                  curEvBits;
void *myUserdata = NULL;


struct tm dateTime;
time_t currTime;
INT32 rtcfd;
M2MB_RTC_TIME_T newTime;
INT16 timeZone;



	AZX_LOG_INIT();
	azx_sleep_ms(5000);

	AZX_LOG_INFO( "\r\n Start RTC demo application. This is v%s built on %s %s.\r\n",
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

	AZX_LOG_INFO("\r\nWaiting for registration...\r\n");

	res = m2mb_net_get_reg_status_info(hNet);
	if ( res != M2MB_RESULT_SUCCESS )
	{
	  AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
	}

	/*Wait for network registration event to occur (released in NetCallback function) */
	m2mb_os_ev_get(net_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

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

	/*
	 * Get time and date in timestamp and date/time format and print them
	 */
	retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_TIMEVAL, &currTime);
	if(retVal == 0)
	{

		AZX_LOG_INFO("\r\nCurrent time in seconds from the epoch: %d\r\n", currTime);

	}
	else
	{

		AZX_LOG_ERROR("Cannot get current time!");
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

	if (newTime.tz != 255)
	{
		timeZone = newTime.tz;
	}
	else
	{
		timeZone = 0;
	}

	/*
	 * Get time and date in timestamp format, add one hour, convert it into date/time format, setthis new value
	 */
	AZX_LOG_INFO("\r\nGet current time and add an hour\r\n");
	retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_TIMEVAL, &currTime);
	if(retVal == 0)
	{
		AZX_LOG_INFO("\r\nCurrent time in seconds from the epoch: %d\r\n", currTime);
	}
	else
	{
		AZX_LOG_ERROR("Cannot get current time!");
		return;
	}

	//Add 1 hour -> 3600 sec

	currTime += 3600;
	{
		struct tm *tmp = localtime(&currTime);
		if (tmp != NULL)
		{
			dateTime = *tmp;
		}
		else
		{			
			AZX_LOG_ERROR("Cannot get local time!");
			return;
		}
	}
	
	AZX_LOG_TRACE("Current time converted: %d-%02d-%02d, %02d:%02d:%02d\r\n", dateTime.tm_year + 1900, dateTime.tm_mon + 1, dateTime.tm_mday, dateTime.tm_hour, dateTime.tm_min, dateTime.tm_sec);

	newTime.hour = dateTime.tm_hour;
	newTime.min = dateTime.tm_min;
	newTime.sec = dateTime.tm_sec;
	newTime.day = dateTime.tm_mday;
	newTime.mon = dateTime.tm_mon + 1; //January is 0
	newTime.year = dateTime.tm_year + 1900;
	newTime.dlst = (UINT8)dateTime.tm_isdst;
	newTime.tz = timeZone;

	AZX_LOG_INFO("New time to be set: %d-%02d-%02d, %02d:%02d:%02d, tz:%d, dlst:%d\r\n", newTime.year, newTime.mon, newTime.day, newTime.hour, newTime.min, newTime.sec, newTime.tz, newTime.dlst);


	//set module system time
	AZX_LOG_INFO("\r\nSet new time and check the setting\r\n");
	retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_SET_SYSTEM_TIME, &newTime);
	if(retVal == 0)
	{
		AZX_LOG_TRACE("\r\nSet new system time done\r\n");
	}
	else
	{
		AZX_LOG_ERROR("Cannot set new system time!");
		return;
	}

	//check if value has changed
	retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_SYSTEM_TIME, &newTime);
	if(retVal == 0)
	{
		AZX_LOG_INFO("NEW module system time is: %d-%02d-%02d, %02d:%02d:%02d\r\n", newTime.year, newTime.mon, newTime.day, newTime.hour, newTime.min, newTime.sec);
	}
	else
	{
		AZX_LOG_ERROR("Cannot get module System time!");
		return;
	}

	azx_sleep_ms(2000);

	m2mb_rtc_close(rtcfd);
}


