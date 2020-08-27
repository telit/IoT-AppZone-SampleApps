/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing FOTA usage with M2MB API. Debug prints on AUX UART
  @version 
    1.0.3
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    02/03/2017
*/
/* Include files ================================================================================*/
#include <stdarg.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"


#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

#include "fota.h"



/* Local defines ================================================================================*/
INT32 fota_task_id;
INT32 main_task_id;

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
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

  azx_tasks_init();

  azx_sleep_ms(5000);


  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting FOTA demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  //Create tasks
  main_task_id = azx_tasks_createTask((char*) "MAIN_TASK", AZX_TASKS_STACK_L, 1, 5, mainTask);
  fota_task_id = azx_tasks_createTask((char*) "FOTA_TASK", AZX_TASKS_STACK_XL, 1, 5, fotaTask);

  AZX_LOG_TRACE("Task ID: %d.\r\n", main_task_id);
  AZX_LOG_TRACE("Fota Task ID: %d.\r\n", fota_task_id);
  
  azx_sleep_ms(5000);
  
  if(fota_task_id > 0)
  {
    azx_tasks_sendMessageToTask( fota_task_id, INITFOTA, 0, 0);
  }
  else
  {
    AZX_LOG_ERROR("Cannot create task!\r\n");
    return;
  }

  azx_sleep_ms(2000);

}

