/*Copyright (C) 2022 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
 @file
    azx_gnu_time.c

 @brief
    gnu definition

 @details
    Porting from gnu to azx

 @note
    Dependencies:
       m2mb_types.h
       m2mb_os_api.h

 @author Cristina Desogus
 @date
    11/08/2022
 */

/* Include files ================================================================================*/
#ifndef _STDIO_H_
#include <stdio.h>
#endif

#include <string.h>
#include <time.h>

#include "m2mb_types.h"
#include "m2mb_rtc.h"

#include "azx_log.h"



/* Function prototypes and Local defines ========================================================*/
#include "azx_gnu_time.h"

/* Function definition ==========================================================================*/

time_t azx_gnu_time(time_t *_timer)
{
	INT32 retVal;
	M2MB_RTC_TIMEVAL_T currTime;
	INT32 rtcfd;
	time_t t;

	rtcfd = m2mb_rtc_open("/dev/rtc0",0);
	if (rtcfd == -1)
	{
		return 0;
	}

	retVal = m2mb_rtc_ioctl(rtcfd, M2MB_RTC_IOCTL_GET_TIMEVAL, &currTime);
	if(retVal != 0)
	{

		m2mb_rtc_close(rtcfd);
		return 0;
	}

	t = (time_t) currTime.sec;

	m2mb_rtc_close(rtcfd);

	if(_timer != NULL)
	{
		*_timer = t;
	}
	return t;

}
