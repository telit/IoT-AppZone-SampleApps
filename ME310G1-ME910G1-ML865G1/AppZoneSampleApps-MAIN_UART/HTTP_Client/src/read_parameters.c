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

char gAPN[MAX_PARAMS_STR_LEN] = {0};
char gAPN_UserName[MAX_PARAMS_STR_LEN] = {0};
char gAPN_Password[MAX_PARAMS_STR_LEN] = {0};
UINT16 gCID = 0;

char gCACERTFILE[MAX_PARAMS_STR_LEN] = {0};
char gCLIENTCERTFILE[MAX_PARAMS_STR_LEN] = {0};
char gCLIENTKEYFILE[MAX_PARAMS_STR_LEN] = {0};
UINT16 gREQUEST_TYPE = 0;
char gSERVER[MAX_PARAMS_STR_LEN] = {0};

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

  char *_APN = NULL;
  char *_APN_USERNAME = NULL;
  char *_APN_PASSWORD = NULL;
  char *_CID = NULL;
  char *_CACERTFILE = NULL;
  char *_CLIENTCERTFILE = NULL;
  char *_CLIENTKEYFILE = NULL;
  char *_REQUEST_TYPE = NULL;
  char *_SERVER = NULL;

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

    //APN
    _APN = mystrtok(&p, recv,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //APN_USERNAME
    _APN_USERNAME = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //APN_PASSWORD
    _APN_PASSWORD = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //_CID
    _CID = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CACERTFILE
    _CACERTFILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENTCERTFILE
    _CLIENTCERTFILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENTKEYFILE
    _CLIENTKEYFILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //REQUEST_TYPE
    _REQUEST_TYPE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //SERVER
    _SERVER = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description


    if( !_APN || !_APN_USERNAME || ! _APN_PASSWORD || !_CID ||
            !_CACERTFILE || !_CLIENTCERTFILE ||  !_CLIENTKEYFILE
            || !_REQUEST_TYPE || !_SERVER)
    {
      AZX_LOG_CRITICAL("Cannot extract parameters from file!!\r\n");
      return -1;
    }
    else
    {

      strcpy(gAPN, _APN);
      strcpy(gAPN_UserName, _APN_USERNAME);
      strcpy(gAPN_Password, _APN_PASSWORD);

      gCID = atoi(_CID);

      strcpy(gCACERTFILE, _CACERTFILE);
      strcpy(gCLIENTCERTFILE, _CLIENTCERTFILE);
      strcpy(gCLIENTKEYFILE, _CLIENTKEYFILE);

      gREQUEST_TYPE = atoi (_REQUEST_TYPE);
      strcpy(gSERVER, _SERVER);

      AZX_LOG_INFO("Set APN to: <<%s>>\r\n", gAPN);
      AZX_LOG_INFO("Set APN USER to: <<%s>>\r\n", gAPN_UserName);
      AZX_LOG_INFO("Set APN PASS to: <<%s>>\r\n", gAPN_Password);
      AZX_LOG_INFO("Set CID to: %u\r\n", gCID);
      AZX_LOG_INFO("Set CACERTFILE to: <<%s>>\r\n", gCACERTFILE);
      AZX_LOG_INFO("Set CLIENTCERTFILE to: <<%s>>\r\n", gCLIENTCERTFILE);
      AZX_LOG_INFO("Set CLIENTKEYFILE to: <<%s>>\r\n", gCLIENTKEYFILE);
      AZX_LOG_INFO("Set REQUEST_TYPE to: %u\r\n", gREQUEST_TYPE);
      AZX_LOG_INFO("Set SERVER to: <<%s>>\r\n", gSERVER);
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
  strcpy(gAPN, APN);
  strcpy(gAPN_UserName, APN_USER);
  strcpy(gAPN_Password, APN_PASS);
  gCID = CID;

  strcpy(gCACERTFILE, CACERTFILE);
  strcpy(gCLIENTCERTFILE, CLIENTCERTFILE);
  strcpy(gCLIENTKEYFILE, CLIENTKEYFILE);

  gREQUEST_TYPE = REQUEST_TYPE;
  strcpy(gSERVER, SERVER);
}
