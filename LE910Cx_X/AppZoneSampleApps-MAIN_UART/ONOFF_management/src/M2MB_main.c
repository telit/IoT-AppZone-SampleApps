/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application on how to handle ON/OFF button. Debug prints on MAIN UART
  @version 
    1.0.0 <fill here>
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
	  Fabio Pintus, Roberta Galeazzo

  @date
    09/02/2023
*/

/* Include files ================================================================================*/
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_trace.h"
#include "m2mb_gpio.h"
#include "m2mb_rtc.h"
#include "m2mb_power.h"

#include "azx_log.h"
#include "app_cfg.h"

/* Local defines ================================================================================*/



#define ON_OFF_HOLD_TIME 3

#define M2MB_ON_OFF_RELEASE  1
#define M2MB_ON_OFF_PRESS  -1

/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
M2MB_POWER_HANDLE powerHandle;
M2MB_RTC_TIMEVAL_T time;

static UINT8 shutdownCalled = FALSE;

static INT8 button_state = M2MB_ON_OFF_RELEASE;
static UINT32 button_press_start_time = 0;
static UINT32 button_hold_time;


/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

void OnOff_trigg_cb(UINT32 fd, void *userdata )
{
(void)fd;
INT32 state = 0;

	if(userdata != NULL)
	{
		state = (INT32) userdata;
		switch(state)
		{

			case M2MB_ON_OFF_PRESS:
			/*button has been pressed*/
				AZX_LOG_INFO("onoff pressed!\r\n");
				/* Ignore this event if we've already handled it. */
				if (shutdownCalled)
				{
					return;
				}

				/* Start counting*/
				if (button_state == M2MB_ON_OFF_RELEASE)
				{
					button_state = M2MB_ON_OFF_PRESS;
					button_press_start_time = (UINT32)((m2mb_os_getSysTicks() * m2mb_os_getSysTickDuration_ms()) / 1000);
				}
				break;

			case M2MB_ON_OFF_RELEASE:
			/* Ignore this event if we've already handled it. */
				AZX_LOG_INFO("onoff released!\r\n");
				if (shutdownCalled)
				{
					return;
				}

				button_state = M2MB_ON_OFF_RELEASE;

				button_hold_time = (UINT32)((m2mb_os_getSysTicks() * m2mb_os_getSysTickDuration_ms()) / 1000);

				/* Has the key been held down for over 3 sec? */
				if ((button_hold_time - button_press_start_time) >= ON_OFF_HOLD_TIME)
				{
					//Turning off module
					AZX_LOG_INFO("ON/OFF pressed form more than %d sec, turn off module!\r\n", ON_OFF_HOLD_TIME);
					shutdownCalled = TRUE;
					m2mb_power_shutdown(powerHandle);
					m2mb_power_deinit(powerHandle);
				}
				break;

			default:
				AZX_LOG_ERROR("Unexpected event %d\r\n", state);
				break;
		}
	}
	else
	{
		AZX_LOG_ERROR("Unexpected  NULL userdata!\r\n");
	}
}


M2MB_RESULT_E OnOff_register(void)
{
M2MB_RESULT_E r = M2MB_RESULT_FAIL;
INT32 fd;
INT32 ret;

	fd = m2mb_gpio_open( "/dev/gpio_ON_OFF", 1 );

	ret =  m2mb_gpio_ioctl(fd,  M2MB_GPIO_IOCTL_SET_INTR_CB, (UINT32) OnOff_trigg_cb );
	if(ret == -1 )
	{
		AZX_LOG_ERROR("M2MB_GPIO_IOCTL_SET_INTR_CB\r\n");
	}
	else
	{
		ret =   m2mb_gpio_ioctl(fd,  M2MB_GPIO_IOCTL_SET_INTR_ARG, (UINT32)0 );
		if(ret == -1 )
		{
			AZX_LOG_ERROR("M2MB_GPIO_IOCTL_SET_INTR_CB_ARG\r\n");
		}
		else
		{
			r = M2MB_RESULT_SUCCESS;
		}
	}

	return r;
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
INT32 ret;
  
  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("\r\n\r\nStarting ON OFF button handling demo app. This is v%s built on %s %s.\r\n",
		  VERSION, __DATE__, __TIME__);

  m2mb_os_taskSleep(M2MB_OS_MS2TICKS(2000));

  /*Register ON/OFF pin*/
  if(M2MB_RESULT_SUCCESS != OnOff_register())
  {
	  AZX_LOG_ERROR("Can't create ON/OFF pin handle\r\n");
	  return;
  }

  /*Init power*/
  ret = m2mb_power_init(&powerHandle, (m2mb_power_ind_callback) NULL, NULL );
  if ( ret != M2MB_RESULT_SUCCESS )
  {
	  AZX_LOG_ERROR( "m2mb_power_init fails!\r\n");
  }

  AZX_LOG_INFO("Waiting for ON/OFF button to be pressed...\r\n");


}

