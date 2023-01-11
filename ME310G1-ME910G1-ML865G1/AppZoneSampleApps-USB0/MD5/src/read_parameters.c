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

char gFILE[MAX_PARAMS_STR_LEN];
char gDATA_STRING[MAX_PARAMS_STR_LEN];
char gRIGHT_MD5_HASH[MAX_PARAMS_STR_LEN];

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

  char *_FILE = NULL;
  char *_DATA_STRING = NULL;
  char *_RIGHT_MD5_HASH = NULL;

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

    //FILE
    _FILE = mystrtok(&p, recv,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //DATA_STRING
    _DATA_STRING = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //RIGHT_MD5_HASH
    _RIGHT_MD5_HASH = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    if( !_FILE || !_DATA_STRING || ! _RIGHT_MD5_HASH )
    {
      AZX_LOG_CRITICAL("Cannot extract parameters from file!!\r\n");
      return -1;
    }
    else
    {
      char tmp[100];
      strcpy(tmp, _FILE);
      sprintf(gFILE, "%s%s",LOCALPATH,tmp);

      strcpy(gDATA_STRING, _DATA_STRING);
      strcpy(gRIGHT_MD5_HASH, _RIGHT_MD5_HASH);

      AZX_LOG_INFO("Set FILE to: <<%s>>\r\n", gFILE);
      AZX_LOG_INFO("Set DATA_STRING to: <<%s>>\r\n", gDATA_STRING);
      AZX_LOG_INFO("Set RIGHT_MD5_HASH PASS to: <<%s>>\r\n", gRIGHT_MD5_HASH);
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
  strcpy(gFILE, FILE);
  strcpy(gDATA_STRING, DATA_STRING);
  strcpy(gRIGHT_MD5_HASH, RIGHT_MD5_HASH);
}
