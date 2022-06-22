/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application that shows how perform FOTA upgrade using a delta file stored into file system. Debug prints on USB0
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Alessio Quieti/Roberta Galeazzo

  @date
    23/05/2022
*/

/* Include files ================================================================================*/
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include "m2mb_types.h"
#include "m2mb_fs_posix.h"
#include "m2mb_fota.h"
#include "m2mb_os_api.h"
#include "m2mb_info.h"
#include "m2mb_power.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
#define fotaWritingFileName (char*) "mod/delta.bin"
#define FOTA_UP_INFO_PATH (char*) "mod/fota_up_info.txt"
/* Local typedefs ===============================================================================*/
typedef enum {

  FOTA_ERROR = -1,
  FOTA_OK = 1,
  FOTA_DELTA_OK = 2,
  FOTA_DELTA_ERROR = 3


} FOTA_RESULT_E;

typedef struct {
	UINT8 fotaFlag;
	CHAR* fwVer;
} FOTA_UP_INFO;

/* Local statics ================================================================================*/
static M2MB_FOTA_HANDLE g_FotaHandle = NULL;
static UINT32 gBlockSize;

static UINT32 fwVersionSize = 100;
static INT32 fd;
static FOTA_UP_INFO fotaUPInfo;
CHAR currFWver[150];

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
static FOTA_RESULT_E fota_deinit(void);
static FOTA_RESULT_E copyFromFileSystemToDelta(char *fileName);
static FOTA_RESULT_E getFotaInfo(void);
static FOTA_RESULT_E check_fota_delta(void);
static FOTA_RESULT_E startUpdate(void);
/* Global functions =============================================================================*/

M2MB_RESULT_E getModuleFwVersion(void)
{
  M2MB_INFO_HANDLE hInfo;
  M2MB_RESULT_E ret;
  CHAR *tempFWver;

  if(m2mb_info_init(&hInfo) != M2MB_RESULT_SUCCESS)
  {
	AZX_LOG_ERROR("m2mb_info_init FAIL\r\n");
	return M2MB_RESULT_FAIL;
  }

  ret = m2mb_info_get(hInfo, M2MB_INFO_GET_SW_VERSION, &tempFWver);
  if(ret != M2MB_RESULT_SUCCESS)
  {
	AZX_LOG_ERROR("m2mb_info_get FAIL\r\n");
	return M2MB_RESULT_FAIL;
  }
  else
  {
	AZX_LOG_INFO("Module current fw version is: \r\n%s\r\n", tempFWver);
	strcpy(currFWver, tempFWver);
	azx_sleep_ms(500);
	m2mb_info_deinit(hInfo);
  }
  return M2MB_RESULT_SUCCESS;
}


M2MB_RESULT_E readFOTAUpgradeStatusFile(const CHAR *path)
{
CHAR tmpBuff[150];
CHAR *start;
CHAR* end;
SSIZE_T nBytes;
UINT8 nVer;
INT32 fileDesc;

  AZX_LOG_TRACE("Read info file\r\n");
  
  memset(tmpBuff,0,sizeof(tmpBuff));
  fileDesc = m2mb_fs_open(path, M2MB_O_RDWR);
  if(fileDesc >= 0)
  {
	nBytes = m2mb_fs_read(fileDesc, tmpBuff, sizeof(tmpBuff));
	if (nBytes < 0)
	{
		AZX_LOG_ERROR("reading failure\r\n");
		return M2MB_RESULT_FAIL;
	}
	if ( tmpBuff[0] >= '0' && tmpBuff[0] <='9')
	{
	  fotaUPInfo.fotaFlag = tmpBuff[0] - '0';
	    //AZX_LOG_INFO("******* fotaUPInfo.fotaFlag: %d    ******\r\n", fotaUPInfo.fotaFlag);
	}
	else
	{
	  AZX_LOG_ERROR("No valid Flag value\r\n");
	}
	start = strstr(tmpBuff, "*");
	if (start == NULL)
	{
	  AZX_LOG_ERROR("no fw version found\r\n");
    }
	else
	{
	  end = strstr(start + 1, "*");
	  if (end != NULL)
	  {
		nVer= end - (start + 1);
	    strncpy(fotaUPInfo.fwVer, start + 1, nVer); 
        AZX_LOG_INFO("\r\nFOTA Flag: %d Module previous fw version: \r\n%s\r\n", fotaUPInfo.fotaFlag, fotaUPInfo.fwVer);		
	  }
	  else
	  {
		AZX_LOG_ERROR("fw version not correctly stored\r\n");  
	  }
	}
	m2mb_fs_close(fileDesc);
	return M2MB_RESULT_SUCCESS;
  }
  else
  {
	return M2MB_RESULT_FAIL;
  }


}


M2MB_RESULT_E writeFOTAUpgradeStatusFile(const CHAR *path, BOOLEAN fotaFlag, CHAR *fwVer)
{
CHAR tmpBuff[100];
SSIZE_T nBytes;
INT32 fileDesc;

fileDesc = m2mb_fs_open(path, M2MB_O_RDWR);
  if(fileDesc >= 0)
  {
	memset(tmpBuff,0,sizeof(tmpBuff));
	sprintf(tmpBuff, "%d*%s*", fotaFlag, fwVer);
	nBytes = m2mb_fs_write(fileDesc, tmpBuff, strlen(tmpBuff));
	if (nBytes < 0)
	{
		AZX_LOG_ERROR("Writeing failure\r\n");
		m2mb_fs_close(fileDesc);
		return M2MB_RESULT_FAIL;
	}
	m2mb_fs_close(fileDesc);
	return M2MB_RESULT_SUCCESS;
  }
  else
  {
	return M2MB_RESULT_FAIL;
  }
}

M2MB_RESULT_E FOTAUpgradeStatusInit(const CHAR *path)
{
  AZX_LOG_INFO("\r\nCheck FOTA upgrade status in file system\r\n\r\n");
  //Get current module fw version
  memset(currFWver,0,sizeof(currFWver));
  getModuleFwVersion(); //get current module fw version

  fotaUPInfo.fwVer = (CHAR*) m2mb_os_malloc(fwVersionSize + 1);
  memset(fotaUPInfo.fwVer,0,fwVersionSize + 1);

  fd = m2mb_fs_open(path, M2MB_O_RDWR); /*Open in read only mode*/
  if(fd == -1)
  {
	AZX_LOG_WARN("File doesn't exist create it, first app execution\r\n");
	//create file
	fd = m2mb_fs_open(path, M2MB_O_CREAT | M2MB_O_RDWR);
	if(fd == -1)
	{
	  AZX_LOG_ERROR("File creation failed!\r\n");
	  return M2MB_RESULT_FAIL;
	}
	else
	{
	  AZX_LOG_INFO("File created, store current fw version and fota upgrade flag=0\r\n");
	  //set default parameters
	  fotaUPInfo.fotaFlag=0;
	  strcpy(fotaUPInfo.fwVer, currFWver);
	  //close file
	  m2mb_fs_close(fd);
	  //write content
	  azx_sleep_ms(500);
	  writeFOTAUpgradeStatusFile(path, fotaUPInfo.fotaFlag, fotaUPInfo.fwVer);

	}

  }
  else
  {
	AZX_LOG_INFO("File exists\r\n");
	m2mb_fs_close(fd);
  }

  return M2MB_RESULT_SUCCESS;
}


static void FOTAIndCallBack(M2MB_FOTA_HANDLE h, M2MB_FOTA_IND_E fota_event, UINT16 resp_size, void *resp_struct, void *userdata)
{
  (void) h;
  (void) resp_size;
  (void) resp_struct;
  (void) userdata;

  AZX_LOG_DEBUG(">>>>>>>>>>>>>>fota_event: %d<<<<<<<<<<<<<<<\r\n", fota_event);

}

static FOTA_RESULT_E getFotaInfo(void)
{
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  M2MB_FOTA_RESULT_CODE_E res_code;
  UINT32 partitionSize;

  retVal = m2mb_fota_update_package_info_get(g_FotaHandle, &gBlockSize, &partitionSize);
  if(retVal != M2MB_RESULT_SUCCESS)
  {
    m2mb_fota_result_code_get(g_FotaHandle, &res_code);
    AZX_LOG_ERROR("Info get failed with code %d\r\n", res_code);
    return FOTA_ERROR;
  }


  AZX_LOG_INFO("OTA blockSize: %d\r\n", gBlockSize);
  AZX_LOG_INFO("OTA partitionSize: %d\r\n", partitionSize);
  return FOTA_OK;
}

static FOTA_RESULT_E copyFromFileSystemToDelta(char *fileName)
{
  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  M2MB_FOTA_RESULT_CODE_E res_code;
  int res = -1;
  UINT8 *pBuffer;
  INT32 fd = -1;

  struct M2MB_STAT stat;

  fd = m2mb_fs_open(fileName, M2MB_O_RDONLY); /*Open in read only mode*/
  if(fd == -1)
  {
    AZX_LOG_ERROR("Cannot open file\r\n");
    return FOTA_ERROR;
  }

  res = m2mb_fs_fstat(fd, &stat);
  if(res == -1)
  {
    AZX_LOG_ERROR("Cannot read file stats\r\n");
    return FOTA_ERROR;
  }
  AZX_LOG_DEBUG("File size: %u\r\n", stat.st_size);

  /* Seek to the beginning of the file */
  res = m2mb_fs_lseek(fd, 0, M2MB_SEEK_SET);
  if(res == -1)
  {
    AZX_LOG_ERROR("Cannot move file offset\r\n");
    return FOTA_ERROR;
  }

  AZX_LOG_TRACE("BLOCK_SIZE: %d\r\n", gBlockSize);

  pBuffer = (UINT8 *) m2mb_os_malloc(gBlockSize);

  UINT32 blockNumber;
  UINT32 remainingBytes;
  blockNumber = stat.st_size / gBlockSize;
  remainingBytes = stat.st_size - (gBlockSize * blockNumber);

  retVal = m2mb_fota_reset(g_FotaHandle);
  if(retVal != M2MB_RESULT_SUCCESS)
  {
    AZX_LOG_ERROR("m2mb_fota_reset FAIL \r\n");
    return FOTA_ERROR;
  }

  UINT8 n = 0;
  for (n = 0; n <= blockNumber; n++)
  {
    memset(pBuffer, 0, gBlockSize);
    res = m2mb_fs_read(fd, pBuffer, gBlockSize);
    if(res < 0)
    {
      AZX_LOG_ERROR("Cannot read file\r\n");
      return FOTA_ERROR;
    }
    retVal = m2mb_fota_update_package_write(g_FotaHandle, n * gBlockSize, pBuffer);
    if(retVal != M2MB_RESULT_SUCCESS)
    {
	  m2mb_fota_result_code_get(g_FotaHandle, &res_code);
      AZX_LOG_ERROR("Write partition failed with code %d at block %d\r\n", res_code, n);
      return FOTA_ERROR;
    }

  }
  /* Read the remaining byte */
  memset(pBuffer, 0, gBlockSize);
  res = m2mb_fs_read(fd, pBuffer, remainingBytes);
  if(res < 0)
  {
    AZX_LOG_ERROR("Cannot read file\r\n");
    return FOTA_ERROR;
  }
  retVal = m2mb_fota_update_package_write(g_FotaHandle, n * gBlockSize, pBuffer);
  if(retVal != M2MB_RESULT_SUCCESS)
  {
	m2mb_fota_result_code_get(g_FotaHandle, &res_code);
    AZX_LOG_ERROR("Write partition failed with code %d at block %d\r\n", res_code, n);
    return FOTA_ERROR;
  }

  m2mb_fs_close(fd);
  m2mb_os_free(pBuffer);


  AZX_LOG_TRACE("End copyFromFileSystemToDelta\r\n");
  return FOTA_OK;
}


static FOTA_RESULT_E check_fota_delta(void)
{
  AZX_LOG_DEBUG("-- check_fota_delta...\r\n");
  //M2MB_RESULT_E retVal = M2MB_RESULT_FAIL;

  if(g_FotaHandle == NULL)
  {
    AZX_LOG_ERROR("Fota handle not valid!\r\n");
    fota_deinit();
    return FOTA_ERROR;
  }

  else
  {
    if(m2mb_fota_update_package_check_setup( g_FotaHandle, M2MB_FOTA_CHECK_SETUP_SOURCE ) != M2MB_RESULT_SUCCESS)
    {
      AZX_LOG_ERROR("Fota check integrity FAIL\r\n");
      return FOTA_DELTA_ERROR;
    }
    else
    {
      AZX_LOG_DEBUG("Fota check integrity PASS\r\n");
      return FOTA_DELTA_OK;
    }
  }
 
}

static FOTA_RESULT_E startUpdate()
{

  M2MB_RESULT_E retVal = M2MB_RESULT_FAIL;

  if(g_FotaHandle == NULL)
  {
    AZX_LOG_ERROR("Fota handle not valid!\r\n");
    fota_deinit();
    return FOTA_ERROR;
  }

  retVal = m2mb_fota_start(g_FotaHandle);
  if(retVal != M2MB_RESULT_SUCCESS)
  {
    AZX_LOG_ERROR("m2mb_fota_start FAIL\r\n");
    fota_deinit();
    return FOTA_ERROR;
  }

  M2MB_POWER_HANDLE h = NULL;

  AZX_LOG_TRACE("m2mb_fota_start PASS\r\n");

  azx_sleep_ms(2000);

  retVal = m2mb_power_init(&h, NULL, NULL);
  if(retVal == M2MB_RESULT_SUCCESS)
  {
	AZX_LOG_DEBUG("\r\nReboot module to start delta deployment\r\n");
	fotaUPInfo.fotaFlag = 1;
	writeFOTAUpgradeStatusFile(FOTA_UP_INFO_PATH, fotaUPInfo.fotaFlag, currFWver);
	m2mb_fs_close(fd);
	m2mb_os_free(fotaUPInfo.fwVer);
	fotaUPInfo.fwVer = NULL;
	azx_sleep_ms(3000);
    m2mb_power_reboot(h);
  }
  else
  {
    AZX_LOG_ERROR("m2mb_power_init FAIL\r\n");
    fota_deinit();
    return FOTA_ERROR;
  }

  return FOTA_OK;
}

static FOTA_RESULT_E fota_deinit(void)
{
  AZX_LOG_DEBUG("fota_deinit Called\r\n");
  if(g_FotaHandle != NULL)
  {
    m2mb_fota_deinit(g_FotaHandle);
    g_FotaHandle = NULL;
  }

  return FOTA_OK;

}
/*-----------------------------------------------------------------------------------------------*/

INT32 smartFotaTask(INT32 type, INT32 map, INT32 param2)
{

  M2MB_RESULT_E retVal = M2MB_RESULT_SUCCESS;
  (void)type;
  (void)map;
  (void)param2;
  azx_sleep_ms(500);

  AZX_LOG_DEBUG("INIT\r\n");

  retVal = m2mb_fota_init(&g_FotaHandle, FOTAIndCallBack, NULL);
  if(retVal != M2MB_RESULT_SUCCESS)
  {
    AZX_LOG_ERROR("m2mb_fota_init FAIL\r\n");
    fota_deinit();
    return FOTA_ERROR;
  }
  else
  {
	AZX_LOG_DEBUG("m2mb_fota_init success\r\n");
  }

  AZX_LOG_DEBUG("\r\nGet block and FOTA partition size\r\n");
  if(getFotaInfo() != FOTA_OK)
  {
    AZX_LOG_ERROR("Failed fota info!\r\n");
    fota_deinit();
    return FOTA_ERROR;
  }

  AZX_LOG_DEBUG("\r\n Copy delta file from File system to FOTA partition\r\n");
  if(copyFromFileSystemToDelta(fotaWritingFileName) != FOTA_OK)
  {
    AZX_LOG_ERROR("Failed to write delta file.\r\n");
    return FOTA_ERROR;
  }
  else
  {
	AZX_LOG_DEBUG("Delta file writing completed\r\n");
  }

  //Check Delta file before calling FOTA Start process
  AZX_LOG_DEBUG("\r\nDelta file check...\r\n");
  if(check_fota_delta() != FOTA_DELTA_OK)
  {
    AZX_LOG_ERROR("Failed check_fota_delta.\r\n");
    return FOTA_ERROR;
  }
  else
  {
  	AZX_LOG_DEBUG("...delta file OK\r\n");
  }

  //Delta deployment: this operation can take some minutes and module will perform several reboots
  AZX_LOG_DEBUG("\r\n--> Start update...\r\n");

  if(startUpdate() != FOTA_OK)
  {
    AZX_LOG_ERROR("Failed starting the fota update.\r\n");
    return FOTA_ERROR;
  }

  return FOTA_OK;

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
  
  UINT8 i = 0;
  INT32 fatTaskID;

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting FOTA from Local File demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);
		
  azx_tasks_init();
  
  if (FOTAUpgradeStatusInit(FOTA_UP_INFO_PATH) == M2MB_RESULT_FAIL)
  {
	AZX_LOG_ERROR("Impossible to init FOTA upgrade status file\r\n");
  }
  else
  {
	azx_sleep_ms(500);
	AZX_LOG_INFO("\r\nRead stored data\r\n");
	readFOTAUpgradeStatusFile(FOTA_UP_INFO_PATH);


	if(fotaUPInfo.fotaFlag == 0)
	{
	  for(i=0; i < strlen(currFWver); i++)
	  {
		//AZX_LOG_INFO("fotaUPInfo.fwVer[%d] = %c, currFWver[%d] = %c\r\n", i, fotaUPInfo.fwVer[i], i, currFWver[i]);
		if(fotaUPInfo.fwVer[i] != currFWver[i])
		{
		  AZX_LOG_INFO("Already upgraded\r\n");
		  break;
		}
	  }

	  if (i == strlen(currFWver))
	  {
		AZX_LOG_INFO("Fw to be upgraded...\r\n");
	  }

	}
	else if(fotaUPInfo.fotaFlag == 1)
	{
	  if(strcmp(fotaUPInfo.fwVer, currFWver) == 0)
	  {
		AZX_LOG_INFO("FOTA not successful, still old fw verison...\r\n");
	  }
	  else
	  {
		AZX_LOG_INFO("FOTA process successful!\r\n");

		fotaUPInfo.fotaFlag = 0;
		writeFOTAUpgradeStatusFile(FOTA_UP_INFO_PATH, fotaUPInfo.fotaFlag, currFWver);
		m2mb_os_free(fotaUPInfo.fwVer);
		fotaUPInfo.fwVer = NULL;
		return;    //just do nothing
	  }
	}
  }


  AZX_LOG_INFO("\r\nStart FOTA process\r\n");

  fatTaskID = azx_tasks_createTask((char*) "FOTA_TASK", AZX_TASKS_STACK_XL, 1, 5, smartFotaTask);

  if(fatTaskID > 0)
  {
    azx_tasks_sendMessageToTask(fatTaskID, 0, 0, 0);
  }
  else
  {
    AZX_LOG_ERROR("Cannot create task!\r\n");
    return;
  }
}

