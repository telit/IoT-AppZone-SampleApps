/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application that shows how to set and implement a watchdog. Task has been locked waitng for an event with a timeout longer than wd inactivity timeout. If no wd kick or no actions 
    are executed during system timeout a system reboot is performed. Debug prints on USB0
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Roberta Galeazzo

  @date
    05/11/2021
*/

/* Include files ================================================================================*/
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_gpio.h"
#include "m2mb_wDog.h"
#include "m2mb_rtc.h"
#include "m2mb_os_tmr.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "azx_tasks.h"

#include "app_cfg.h"

#include "WatchDog.h"




/* Local defines ================================================================================*/
#define WAKE_UP_TICKS 20       //to be used in m2mb_wDog_enable
#define CTRL_TICKS_TO_REBOOT 6 //to be used in m2mb_wDog_enable
#define WD_TOUT_COUNT 3        //to be used in m2mb_wDog_addTask

#define TIMER_TOUT 5000

/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
M2MB_OS_TASK_HANDLE Task1_H = NULL;
INT8 taskID_1;
M2MB_WDOG_HANDLE h_wDog;
M2MB_OS_EV_HANDLE evHandle;

/*Timer to perform wd kick*/
M2MB_OS_TMR_ATTR_HANDLE      tmrAttrHandle;
M2MB_OS_TMR_HANDLE           tmrHandle;
UINT32 timeval1, timeval2, diff;
UINT32 wd_tick_s;

/* Local function prototypes ====================================================================*/
void WDog_Init(void);

/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
static void TimerCb(M2MB_OS_TMR_HANDLE tHandle, void *argCb )
{
static unsigned int Count = 0;

	(void)tHandle;
	(void)argCb;

	AZX_LOG_TRACE("\r\nCallback Count: [%u]\r\n", Count++);
	if (Count < 5)
	{
		AZX_LOG_INFO("WD kick\r\n");
		azx_tasks_sendMessageToTask(taskID_1, WD_KICK, 0, 0);
	}
	else
	{
		AZX_LOG_INFO("\r\n...stop WD kicks and simulate infinite loop\r\n");
		azx_tasks_sendMessageToTask(taskID_1, LOOP, 0, 0);
		m2mb_os_tmr_stop(tmrHandle);
	}
}

void timer_Init(void)
{
M2MB_OS_RESULT_E res;

	res = m2mb_os_tmr_setAttrItem( &tmrAttrHandle,
	                                   CMDS_ARGS
	                                   (
	                                     M2MB_OS_TMR_SEL_CMD_CREATE_ATTR,  NULL,
	                                     M2MB_OS_TMR_SEL_CMD_NAME, "mytmr",
	                                     M2MB_OS_TMR_SEL_CMD_USRNAME, "myUsrtmr",
	                                     M2MB_OS_TMR_SEL_CMD_CB_FUNC, &TimerCb,
	                                     M2MB_OS_TMR_SEL_CMD_ARG_CB, &tmrHandle,
	                                     M2MB_OS_TMR_SEL_CMD_TICKS_PERIOD, M2MB_OS_MS2TICKS(TIMER_TOUT),
										 M2MB_OS_TMR_SEL_CMD_PERIODIC, M2MB_OS_TMR_PERIODIC_TMR
	                                   )
	                                 );
									 
	if( res != M2MB_OS_SUCCESS )
	{
		AZX_LOG_ERROR("Timer attribute creation fail, error: %d\r\n", res);
		return;
	}
	else
	{
		AZX_LOG_TRACE("Timer attribute creation OK\r\n" );
	}
	//Define the timer and pass the parameters structure to it.
	res = m2mb_os_tmr_init( &tmrHandle, &tmrAttrHandle );
	if( res != M2MB_OS_SUCCESS )
	{
		//delete the parameters structure
		m2mb_os_tmr_setAttrItem( &tmrAttrHandle, 1, M2MB_OS_TMR_SEL_CMD_DEL_ATTR, NULL );
		AZX_LOG_ERROR("Timer creation fail, error: %d\r\n", res);
	}
	else
	{
		AZX_LOG_INFO("Timer creation OK\r\n" );
	}

}

INT32 get_timeval(UINT32 *pOutTime)
{
INT32 fd;
M2MB_RTC_TIMEVAL_T timeval;

	fd = m2mb_rtc_open("/dev/rtc0", 0);
	/* protection from invalid handle */
	if (fd == -1)
		return -3;

	if (m2mb_rtc_ioctl(fd, M2MB_RTC_IOCTL_GET_TIMEVAL, &timeval) == -1)
	{
		m2mb_rtc_close(fd);
		return -1;
	}
	(*pOutTime) = timeval.msec + (timeval.sec * 1000);

	if (m2mb_rtc_close(fd) == -1)
		return -2;
	return 0;
}

void WDcallback(M2MB_WDOG_HANDLE hDog, M2MB_WDOG_IND_E wDog_event,UINT16 resp_size, void *resp_struct, void *userdata)
{
	(void)hDog;
	(void)resp_size;
	(void)resp_struct;
	(void)userdata;
	
	switch (wDog_event)
	{
		case M2MB_WDOG_TIMEOUT_IND:
		{
			AZX_LOG_INFO("Watchdog expired!\r\n");
			/*release the event to unlock the task*/
			get_timeval(&timeval2);
			m2mb_os_ev_set(evHandle, EV_WDOG_TEST, M2MB_OS_EV_SET);
		}
		break;
		
		default:
			break;
	}
}

void WDog_Init(void)
{
M2MB_RESULT_E res;
MEM_W time_ms;

	AZX_LOG_INFO("\r\nInit WatchDog\r\n");
	res = m2mb_wDog_init(&h_wDog, WDcallback, 0);
	if (res == M2MB_RESULT_SUCCESS)
	{
		AZX_LOG_INFO("m2mb_wDog_init OK\r\n");
	}
	else
	{
		AZX_LOG_ERROR("m2mb_wDog_init Fail, error: %d\r\n", res);
		return;
	}

	/* Verifying tick duration */
	res = m2mb_wDog_getItem(h_wDog, M2MB_WDOG_SELECT_CMD_TICK_DURATION_MS, 0, &time_ms);
	if (res == M2MB_RESULT_SUCCESS)
	{
		wd_tick_s = time_ms/1000;
		AZX_LOG_INFO("Tick duration: %ds\r\n", wd_tick_s);
	}
	else
	{
		AZX_LOG_ERROR("Get tick duration Fail, error: %d\r\n", res);
	}


	AZX_LOG_INFO("Adding Task under WD control with inactivity timeout of %ds\r\n", WD_TOUT_COUNT * WAKE_UP_TICKS * wd_tick_s);
	/* wdTimeout (inactivity timeout of the task) is set to WD_TOUT_COUNT (3 in this case).
	 * This counter is decreased every time a control is done and no kick have been received. Control is done every WAKE_UP_TICKS.
	 * When the counter reaches 0 a further control is done and if it's still 0 then callback is called,
	 * so task inactivity timeout will be more or less  WD_TOUT_COUNT * WAKE_UP_TICKS * 1s
	*/
	res = m2mb_wDog_addTask(h_wDog, Task1_H, WD_TOUT_COUNT);

	if (res == M2MB_RESULT_SUCCESS)
	{
		AZX_LOG_INFO("m2mb_wDog_addTask OK\r\n");
	}
	else
	{
		AZX_LOG_ERROR("m2mb_wDog_addTask Fail\r\n");
	}

	AZX_LOG_INFO("Enabling the WatchDog\r\n");
	/* WAKE_UP_TICKS defines the number of ticks of every control, by default the tick is every 1s
	 * CTRL_TICKS_TO_REBOOT this defines the number of controls the wd does before rebooting the app if no kick are received (or no action is done in watchdog callback )
	 * so timeout to reboot is  WAKE_UP_TICKS * CTRL_TICKS_TO_REBOOT * 1s
	 */
	res = m2mb_wDog_enable(h_wDog, WAKE_UP_TICKS, CTRL_TICKS_TO_REBOOT);
	if (res == M2MB_RESULT_SUCCESS)
	{
		AZX_LOG_INFO("m2mb_wDog_enable OK\r\n");
	}
	else
	{
		AZX_LOG_ERROR("m2mb_wDog_enable Fail, error: %d\r\n", res);
	}

}


INT32 M2MB_msgTask1(INT32 type, INT32 param1, INT32 param2)
{
M2MB_OS_RESULT_E osRes;
UINT32 ev_bits;

	(void)param1;
	(void)param2;
	//AZX_LOG_INFO(".\r\n");
	switch (type)
	{
	case TASK_START:
		AZX_LOG_INFO("\r\nTask started\r\n");
		Task1_H = m2mb_os_taskGetId();

		WDog_Init();
		timer_Init();

		AZX_LOG_INFO("\r\nStart WD kick every %ds...\r\n", TIMER_TOUT/1000);
		m2mb_os_tmr_start(tmrHandle);

		break;

	case LOOP:
		/*
		 * Simulate an infinite loop waiting an event for WDOG_WAIT_TO that should be bigger than WD_TOUT_COUNT * WAKE_UP_TICKS * 1s
		*/
		AZX_LOG_INFO("Waiting for EV_WDOG_TEST event to simulate task lock\r\n");
		get_timeval(&timeval1);
		osRes = m2mb_os_ev_get(evHandle, EV_WDOG_TEST,
				M2MB_OS_EV_GET_ALL_AND_CLEAR, &ev_bits,
				WDOG_WAIT_TO);
		if (M2MB_OS_SUCCESS == osRes)
		{
			AZX_LOG_INFO("Event EV_WDOG_TEST received - watchdog performed as expected after %d ms\r\n",(timeval2-timeval1));
		}
		else
		{
			AZX_LOG_ERROR("Event EV_WDOG_TEST not received in time %d, code %d\r\n",
					WDOG_WAIT_TO, osRes);
		}
		AZX_LOG_INFO("Task doing nothing and no kicks, waiting for app restart in %ds", CTRL_TICKS_TO_REBOOT * WAKE_UP_TICKS * wd_tick_s);
		break;

	case WD_KICK:
		m2mb_wDog_kick(h_wDog, Task1_H);
		AZX_LOG_INFO("WD kick done\r\n");
		break;

	}

	return 0;
}


/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
**************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
M2MB_OS_RESULT_E os_res;
M2MB_OS_EV_ATTR_HANDLE evAttrHandle;

	(void)argc;
	(void)argv;

	azx_tasks_init();

	azx_sleep_ms(5000);

	/*SET output channel */
	AZX_LOG_INIT();

	AZX_LOG_INFO("\r\n\r\nStarting WATCHDOG demo app. This is v%s built on %s %s.\r\n\r\n",VERSION, __DATE__, __TIME__);

	// *********** event initialization **************

	m2mb_os_ev_setAttrItem(&evAttrHandle, 2, M2MB_OS_EV_SEL_CMD_CREATE_ATTR,
		M2MB_OS_EV_SEL_CMD_NAME, NULL, "MyEv");

	os_res = m2mb_os_ev_init(&evHandle, &evAttrHandle);
	if (os_res != M2MB_OS_SUCCESS)
	{
		m2mb_os_ev_setAttrItem(&evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR,
			NULL);
	}
	else
	{
		AZX_LOG_INFO("Event correctly initialized \r\n");
	}

	////************** TASK INITIALIZATION *************
	taskID_1 = azx_tasks_createTask((char*) "TASK_1", AZX_TASKS_STACK_L, 1, AZX_TASKS_MBOX_M, M2MB_msgTask1);

	AZX_LOG_TRACE("Task ID: %d\r\n", taskID_1);

	if (taskID_1 > 0)
	{
		azx_tasks_sendMessageToTask( taskID_1, TASK_START, 0, 0);
	}
	else
	{
		AZX_LOG_ERROR("cannot create task!\r\n");
		return;
	}

	azx_sleep_ms(2000);

}

