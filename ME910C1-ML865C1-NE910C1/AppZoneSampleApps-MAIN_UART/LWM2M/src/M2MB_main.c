/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing TLS/SSL with client certificates usage with M2MB API. Debug prints on MAIN UART
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    27/05/2020
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
  AZX_LOG_INFO("Starting lwm2m demo. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  AZX_LOG_INFO("On OneEdge portal, be sure that observations are enabled for the following object resources:\r\n"
      "{35000/0/01} {35000/0/02} {35000/0/03} {35000/0/04} {35000/0/05} {35000/0/06} {35000/0/07}\r\n"
      "{35000/0/11} {35000/0/12} {35000/0/13} {35000/0/14} {35000/0/15} {35000/0/16} {35000/0/17}\r\n"
      "{35000/0/21} {35000/0/22} {35000/0/23} {35000/0/24} {35000/0/25} {35000/0/26} {35000/0/27}\r\n"
      "{35000/0/31} {35000/0/32} {35000/0/33} {35000/0/34} {35000/0/35} {35000/0/36} {35000/0/37}\r\n\r\n");

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

