/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    The application connects to an NTP server, gets current date and time and updates module's internal clock. Debug prints on AUX UART
  @version 
    1.0.0 
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Roberta Galeazzo

  @date
    14/04/2021
*/

/* Include files ================================================================================*/
#include "m2mb_types.h"

#include "ntp_types.h"
#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
INT8 ntpTask;
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
**************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;

	AZX_LOG_INIT();
	azx_tasks_init();

	azx_sleep_ms(5000);


	AZX_LOG_INFO( "\r\n Start NTP demo application. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

	ntpTask = azx_tasks_createTask((char*) "myNTPTask", AZX_TASKS_STACK_XL, 1, AZX_TASKS_MBOX_S, NTP_task);
	azx_sleep_ms(1000);

	if (ntpTask > 0){

		azx_tasks_sendMessageToTask(ntpTask, INIT, 0, 0 );

	} else{

		AZX_LOG_ERROR("Cannot create tasks!\r\n");
		return;

	}
	azx_sleep_ms(2000);
}

