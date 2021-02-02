/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
  	  test_main.c.h

  @brief
  	  Project: SPI data flash

  @details

  @version
 	 1.0.0

  @note

  @author
  	  Norman Argiolas

  @note
  	  File created on: Sep 4, 2020
*/

/* Include files ================================================================================*/
#include <stdio.h>

#include "azx_utils.h"
#include "azx_tasks.h"
#include "azx_log.h"

#include "test_main.h"

//--------select the running tests
#define NAND_TESTGROUP						1

int error = 0;

//------List of all test groups

#if NAND_TESTGROUP == 1
TEST_RESULT_E 	runNand_TestGroup	(void);
INT32 runNandTask(INT32 type, INT32 param1, INT32 param2);
#endif


//---------------------Task functions-------------------------------------

#if NAND_TESTGROUP == 1
INT32 runNandTask(INT32 type, INT32 param1, INT32 param2)
{
	(void)type;
	(void)param1;
	(void)param2;

	M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
	MEM_W  name = 0;

	m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &name, NULL );
	AZX_LOG_DEBUG("Inside %s user callback function\r\n", (char*) name);

	AZX_LOG_INFO("<><><><>Nand Test Group STARTING!!<><><><>\r\n");
	if (runNand_TestGroup() == TEST_OK)
	{
		AZX_LOG_INFO("<><><><>Nand Test Group PASSED!!<><><><>\r\n");
	}
	else
	{
		AZX_LOG_ERROR("<><><><>Nand Test Group FAILED!!<><><><>\r\n");
		error ++;
	}
	AZX_LOG_DEBUG("Exit from  %s user callback function\r\n", (char*) name);
	return 0;
}
#endif
//---------------------END Task functions-------------------------------------

TEST_RESULT_E runMainTest(void)
{
	azx_sleep_ms(2000);
	if (M2MB_OS_SUCCESS != azx_tasks_init())
	{
		AZX_LOG_DEBUG("Failed to initialize task structures\r\n");
		return TEST_ERROR;
	}

#if NAND_TESTGROUP == 1
	AZX_LOG_INFO("<>\r\n<>\r\n<>\r\n<>\r\n");

	INT8 nandTaskID;
	nandTaskID = azx_tasks_createTask((char*) "runNand_Task",
											AZX_TASKS_STACK_XL, 2,
											AZX_TASKS_MBOX_L,
											runNandTask);
	if (nandTaskID > 0)
	{
		azx_tasks_sendMessageToTask( nandTaskID, 3, 4, 5 );
	}
	else
	{
		AZX_LOG_DEBUG("Task error\r\n");
		return TEST_ERROR;
	}
#endif

	if (error)
	{
		return TEST_ERROR;
	}
	return TEST_OK;
}


