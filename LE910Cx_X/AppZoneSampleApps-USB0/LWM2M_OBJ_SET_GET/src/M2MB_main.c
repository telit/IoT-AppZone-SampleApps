/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showcasing LWM2M client m2mb_lwm2m_objget and m2mb_lwm2m_objset M2MB APIs usage. Debug prints on USB0
  @version
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    22/11/2021
*/

/* Include files ================================================================================*/
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_lwm2m.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

#include "lwm2m_demo.h"


/* Local defines ================================================================================*/
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

  INT32 taskID;

  azx_tasks_init();

  azx_sleep_ms(5000);


  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting lwm2m demo. This is v%s built on %s %s.\r\n",VERSION, __DATE__, __TIME__);

  AZX_LOG_INFO("On OneEdge portal, be sure that the 'lwm2m resources objset - objget demo' Device Profile has been uploaded on the IoT Portal\r\n\r\n");

  taskID = azx_tasks_createTask((char*) "LWM2M_TASK", AZX_TASKS_STACK_XL, 1, AZX_TASKS_MBOX_M, msgLWM2MTask);

  AZX_LOG_TRACE("Task ID: %d.\r\n", taskID);

  azx_sleep_ms(1000);

  if (taskID > 0)
  {

    azx_tasks_sendMessageToTask( taskID, INIT, 0, 0);
  }
  else
  {
    AZX_LOG_ERROR("Cannot create task!\r\n");
    return;
  }
}

