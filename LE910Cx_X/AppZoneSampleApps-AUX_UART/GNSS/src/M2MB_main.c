/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to use GNSS functionality. Debug prints on AUX UART
  @version 
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Roberta Galeazzo

  @date
    12/10/2021
 */
/* Include files ================================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_os_sem.h"

#include "gnss_task.h"


#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"
#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

INT32 gpsTask;






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
  
	azx_sleep_ms(2000);

	AZX_LOG_INIT();
	azx_tasks_init();

	AZX_LOG_INFO("\r\nStarting GNSS demo app. This is v%s built on %s %s.\r\n\r\n",
		VERSION, __DATE__, __TIME__);

	gpsTask = azx_tasks_createTask((char*) "myGPSTask", AZX_TASKS_STACK_L, 1, AZX_TASKS_MBOX_S, GPS_task);

	azx_sleep_ms(1000);

		if (gpsTask > 0){

			azx_tasks_sendMessageToTask(gpsTask, START_GPS, 0, 0 );
			AZX_LOG_TRACE("Tasks %d created!\r\n", gpsTask);

		} else{

			AZX_LOG_ERROR("Cannot create tasks!\r\n");
			return;

		}
		azx_sleep_ms(2000);
 

}

