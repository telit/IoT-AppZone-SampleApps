/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing how to use HTTPs client functionalities. Debug prints on AUX UART
  @version 
    1.1.6
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    05/05/2020
 */

/* Include files ================================================================================*/


#include "m2mb_types.h"
#include "m2mb_os_api.h"


#include "m2mb_net.h"
#include "m2mb_pdp.h"

#include "m2mb_socket.h"
#include "m2mb_ssl.h"

#include "m2mb_fs_posix.h"

#include "m2mb_HTTP_test.h"

#include "azx_log.h"
#include "azx_tasks.h"
#include "azx_utils.h"

#include "app_cfg.h"

#include "azx_https.h"
#include "azx_base64.h" //for azx_base64encoder

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

  azx_sleep_ms(2000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting HTTP(s) client demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);


  taskID = azx_tasks_createTask((char*) "HttpClient", AZX_TASKS_STACK_XL*2, 2, AZX_TASKS_MBOX_S, M2M_msgHTTPTask);

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
  azx_sleep_ms(2000);

}

