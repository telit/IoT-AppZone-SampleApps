/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing multi tasking functionalities with M2MB API. Debug prints on AUX UART
  @version 
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    12/03/2019
*/
/* Include files ================================================================================*/
#include <stdio.h>

#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"


#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/


/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
static M2MB_OS_SEM_HANDLE taskSemHandle = NULL;

static INT32 taskID1, taskID2, taskID3;

/* Local function prototypes ====================================================================*/
INT32 azx_msgTask1(INT32 type, INT32 param1, INT32 param2)
{

  
  M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
  MEM_W  name = 0;

  m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &name, NULL );
  AZX_LOG_INFO("\r\nInside \"%s\" user callback function. Received parameters from MAIN: %d %d %d\r\n", (char*) name, type, param1, param2);
  
  azx_sleep_ms(1000);
  AZX_LOG_INFO("Task1 - Sending a message to task 2 with modified parameters...\r\n");
  azx_tasks_sendMessageToTask( taskID2, type + 2, param1 +3, param2 * 2 );
  
  
  AZX_LOG_INFO("Task1 - Waiting for semaphore to be released by task 3 now...\r\n");
  m2mb_os_sem_get(taskSemHandle, M2MB_OS_WAIT_FOREVER );
  
  AZX_LOG_INFO("\r\nTask1 - After semaphore! return...\r\n\r\n");
  return 0;
}

INT32 azx_msgTask2(INT32 type, INT32 param1, INT32 param2)
{

  
  M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
  MEM_W  name = 0;

  m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &name, NULL );
  AZX_LOG_INFO("\r\nInside \"%s\" user callback function. Received parameters: %d %d %d\r\n", (char*) name, type, param1, param2);
  
  azx_sleep_ms(2000);
  
  if(type >= 5) /*initial Main value 3 + task1 addition of 2 -> 5*/
  {
    AZX_LOG_INFO("Task2 - Sending a message to task 3 with modified parameters...\r\n");
    azx_tasks_sendMessageToTask( taskID3, type * 3 , param1 + 7, param2 - 1 );
  }
  else
  {
    /*type is less than 5, can be arriving from task 3 with value 0*/
    taskHandle = (M2MB_OS_TASK_HANDLE) param1;
    m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &name, NULL );
    AZX_LOG_INFO("Task2 - Received type %d from task \"%s\"\r\n", type, name);
  }

  AZX_LOG_INFO("Task2 - Done.\r\n");
  return 0;
}

INT32 azx_msgTask3(INT32 type, INT32 param1, INT32 param2)
{
  (void)type;
  (void)param1;
  (void)param2;
  
  M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
  MEM_W  name = 0;

  m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &name, NULL );
  AZX_LOG_INFO("\r\nInside \"%s\" user callback function. Received parameters from Task 2: %d %d %d\r\n", (char*) name, type, param1, param2);
  
  azx_sleep_ms(3000);
  
  AZX_LOG_INFO("Task3 - Releasing IPC semaphore...\r\n");
  m2mb_os_sem_put(taskSemHandle);
  AZX_LOG_INFO("Task3 - IPC semaphore released.\r\n");
  
  AZX_LOG_INFO("Task3 - Sending a message to task 2 with specific 'type' parameter value of 0 and task 3 handle as param1...\r\n");
  azx_tasks_sendMessageToTask( taskID2, 0 , (INT32)taskHandle, param2 );
  AZX_LOG_INFO("Task3 - Done.\r\n");
  return 0;
}



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
  
  M2MB_OS_SEM_ATTR_HANDLE semAttrHandle;
  

  

  azx_tasks_init();

  azx_sleep_ms(5000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting MultiTask demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);


  /*Creating an InterProcess Communication (IPC) semaphore*/
  if (taskSemHandle == NULL)
  {
    m2mb_os_sem_setAttrItem( &semAttrHandle, CMDS_ARGS( 
    M2MB_OS_SEM_SEL_CMD_CREATE_ATTR,  NULL,
    M2MB_OS_SEM_SEL_CMD_COUNT, 0 /*IPC*/, 
    M2MB_OS_SEM_SEL_CMD_TYPE, M2MB_OS_SEM_GEN,M2MB_OS_SEM_SEL_CMD_NAME, "taskSem"));
    m2mb_os_sem_init( &taskSemHandle, &semAttrHandle );
  }

  taskID1 = azx_tasks_createTask((char*) "myTask1", AZX_TASKS_STACK_M, 1, AZX_TASKS_MBOX_M, azx_msgTask1);
  taskID2 = azx_tasks_createTask((char*) "myTask2", AZX_TASKS_STACK_M, 2, AZX_TASKS_MBOX_M, azx_msgTask2);
  taskID3 = azx_tasks_createTask((char*) "myTask3", AZX_TASKS_STACK_M, 3, AZX_TASKS_MBOX_M, azx_msgTask3);


  AZX_LOG_TRACE("Task1 ID : %d.\r\n", taskID1);
  AZX_LOG_TRACE("Task2 ID : %d.\r\n", taskID2);
  AZX_LOG_TRACE("Task3 ID : %d.\r\n", taskID3);
  
  azx_sleep_ms(1000);
  
  if (taskID1 > 0 && taskID2 > 0 && taskID2 > 0 )
  {
    azx_tasks_sendMessageToTask( taskID1, 3, 4, 5 );
  }
  else
  {
    AZX_LOG_ERROR("Cannot create tasks!\r\n");
    return;
  }

  azx_sleep_ms(2000);

}

