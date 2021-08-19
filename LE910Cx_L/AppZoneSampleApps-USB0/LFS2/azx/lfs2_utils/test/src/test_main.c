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
#define RAW_LFS_USAGE_TESTGROUP 			0
#define LFS_RAM_UTILS_USAGE_TESTGROUP 		0
#define LFS_FLASH_UTILS_USAGE_TESTGROUP 	1

int error = 0;

//------List of all test groups

#if RAW_LFS_USAGE_TESTGROUP == 1
TEST_RESULT_E 	runRawLfsUsage_TestGroup			(void);	//without task
#endif

#if LFS_RAM_UTILS_USAGE_TESTGROUP == 1
TEST_RESULT_E 	runRamLfsUtilsUsage_TestGroup			(void);
INT32 runRamUtilsUsageTask (INT32 type, INT32 param1, INT32 param2);
#endif

#if LFS_FLASH_UTILS_USAGE_TESTGROUP == 1
TEST_RESULT_E 	runLfsFlashUtilsUsage_TestGroup			(void);
INT32 runLfsFlashUtilsUsageTask(INT32 type, INT32 param1, INT32 param2);
#endif




//---------------------Task functions-------------------------------------

#if LFS_FLASH_UTILS_USAGE_TESTGROUP == 1
INT32 runLfsFlashUtilsUsageTask(INT32 type, INT32 param1, INT32 param2)
{
	(void)type;
	(void)param1;
	(void)param2;

	M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
	MEM_W  name = 0;

	m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &name, NULL );
	AZX_LOG_DEBUG("Inside %s user callback function\r\n", (char*) name);

	AZX_LOG_INFO("<><><><>Test Utils Usage Test Group STARTING!!<><><><>\r\n");
	if (runLfsFlashUtilsUsage_TestGroup() == TEST_OK)
	{
		AZX_LOG_INFO("<><><><>Test Utils Usage Test Group PASSED!!<><><><>\r\n");
	}
	else
	{
		AZX_LOG_ERROR("<><><><>Test Utils Usage Test Group FAILED!!<><><><>\r\n");
		error ++;
	}
	AZX_LOG_DEBUG("Exit from  %s user callback function\r\n", (char*) name);
	return 0;
}
#endif

#if LFS_RAM_UTILS_USAGE_TESTGROUP == 1
INT32 runRamUtilsUsageTask (INT32 type, INT32 param1, INT32 param2)
{
	(void)type;
	(void)param1;
	(void)param2;

	M2MB_OS_TASK_HANDLE taskHandle = m2mb_os_taskGetId();
	MEM_W  name = 0;

	m2mb_os_taskGetItem( taskHandle, M2MB_OS_TASK_SEL_CMD_NAME, &name, NULL );
	AZX_LOG_DEBUG("Inside %s user callback function\r\n", (char*) name);

	AZX_LOG_INFO("<><><><>RAM Utils Usage Test Group STARTING!!<><><><>\r\n");
	if (runRamLfsUtilsUsage_TestGroup() == TEST_OK)
	{
		AZX_LOG_INFO("<><><><>Test Utils Usage Test Group PASSED!!<><><><>\r\n");
	}
	else
	{
		AZX_LOG_ERROR("<><><><>RAM Utils Usage Test Group FAILED!!<><><><>\r\n");
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

#if RAW_LFS_USAGE_TESTGROUP == 1
	AZX_LOG_INFO("<>\r\n<>\r\n<>\r\n<>\r\n");

	AZX_LOG_INFO("<><><><>Row Lfs Usage Test Group STARTING!!<><><><>\r\n");
	if (runRawLfsUsage_TestGroup() == TEST_OK)
	{
		AZX_LOG_INFO("<><><><>Row Lfs Usage Test Group PASSED!!<><><><>\r\n");
	}
	else
	{
		AZX_LOG_ERROR("<><><><>Row Lfs Usage Test Group FAILED!!<><><><>\r\n");
		error ++;
	}
#endif

#if LFS_FLASH_UTILS_USAGE_TESTGROUP == 1
	AZX_LOG_INFO("<>\r\n<>\r\n<>\r\n<>\r\n");

	INT8 testUtilsUsageTaskID;
	testUtilsUsageTaskID = azx_tasks_createTask((char*) "runLfsFlashUtilsUsageTask",
											AZX_TASKS_STACK_XL, 2,
											AZX_TASKS_MBOX_L,
											runLfsFlashUtilsUsageTask);
	if (testUtilsUsageTaskID > 0)
	{
		azx_tasks_sendMessageToTask( testUtilsUsageTaskID, 3, 4, 5 );
	}
	else
	{
		AZX_LOG_DEBUG("Task error\r\n");
		return TEST_ERROR;
	}
#endif


#if LFS_RAM_UTILS_USAGE_TESTGROUP == 1
	AZX_LOG_INFO("<>\r\n<>\r\n<>\r\n<>\r\n");

	INT8 ramUtilsUasgeTaskID;
	ramUtilsUasgeTaskID = azx_tasks_createTask((char*) "runRamUtilsUasgeTask",
											AZX_TASKS_STACK_XL, 2,
											AZX_TASKS_MBOX_L,
											runRamUtilsUsageTask);
	if (ramUtilsUasgeTaskID > 0)
	{
		azx_tasks_sendMessageToTask( ramUtilsUasgeTaskID, 3, 4, 5 );
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


