/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    read_parameters.c

  @brief
    The file contains the parameters reading utilities

  @details

  @version 
    1.0.0

  @note

  @author


  @date
    21/10/2022
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "m2mb_os_api.h"

#include "m2mb_fs_posix.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h" /*FOR LOCALPATH define*/

#include "read_parameters.h"

char gFOTA_WRITING_FILE_NAME[MAX_PARAMS_STR_LEN];
char gFOTA_UP_INFO_PATH[MAX_PARAMS_STR_LEN];

/* Macro =============================================================================*/

/* Local defines ================================================================================*/

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
char *mystrtok(char **m, char *s, char c)
{
  char *p = s?s:*m;
  if( !p )
  {
    return 0;
  }

  if( !*p )
  {
    return 0;
  }
  *m = strchr(p,c);
  if( *m )
    *(*m)++=0;
  else
    *m = p+strlen(p);
  return p;
}

int readConfigFromFile(void)
{

  INT32 fd = -1;
  INT32 fs_res;
  CHAR recv[512];

  char *p = NULL;

  char *_FOTA_WRITING_FILE_NAME = NULL;
  char *_FOTA_UP_INFO_PATH = NULL;

  AZX_LOG_TRACE("Reading parameters from file\r\n");

  AZX_LOG_TRACE("Opening %s in read mode..\r\n", CONFIG_FILE);

  fd = m2mb_fs_open(CONFIG_FILE,M2MB_O_RDONLY);

  if(fd != -1)
  {
    memset(recv, 0, sizeof(recv));
    fs_res = m2mb_fs_read(fd, recv, sizeof(recv));
    if(-1 == fs_res)
    {
      AZX_LOG_ERROR("cannot open config file!\r\n");
    }
    AZX_LOG_TRACE("Received %d bytes from file: \r\n<%.*s> \r\n", fs_res, fs_res, recv);

    AZX_LOG_TRACE("Closing file.\r\n");
    m2mb_fs_close(fd);

    //Using mystrtok function to separate #tag from the setting file

    //FOTA_WRITING_FILE_NAME
    _FOTA_WRITING_FILE_NAME = mystrtok(&p, recv,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FOTA_UP_INFO_PATH
    _FOTA_UP_INFO_PATH = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    if( !_FOTA_WRITING_FILE_NAME || !_FOTA_UP_INFO_PATH)
    {
      AZX_LOG_CRITICAL("Cannot extract parameters from file!!\r\n");
      return -1;
    }
    else
    {

      char tmp[100];
      strcpy(tmp, _FOTA_WRITING_FILE_NAME);
      sprintf(gFOTA_WRITING_FILE_NAME, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);

      strcpy(tmp, _FOTA_UP_INFO_PATH);
      sprintf(gFOTA_UP_INFO_PATH, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);

      AZX_LOG_INFO("Set FOTA_WRITING_FILE_NAME to: <<%s>>\r\n", gFOTA_WRITING_FILE_NAME);
      AZX_LOG_INFO("Set gFOTA_UP_INFO_PATH to: <<%s>>\r\n", gFOTA_UP_INFO_PATH);
      return 1;
    }
  }
  else
  {
    return -1;
  }
}

void configureParameters(void)
{
  strcpy(gFOTA_WRITING_FILE_NAME, FOTA_WRITING_FILE_NAME);
  strcpy(gFOTA_UP_INFO_PATH, FOTA_UP_INFO_PATH);
}
