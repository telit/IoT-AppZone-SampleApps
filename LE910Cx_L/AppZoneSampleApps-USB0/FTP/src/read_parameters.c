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
UINT16 gPDP_CTX = 0;

char gFTP_ADDR[MAX_PARAMS_STR_LEN] = {0};
UINT16 gFTP_PORT = {0};
char gFTP_USER[MAX_PARAMS_STR_LEN] = {0};
char gFTP_PASS[MAX_PARAMS_STR_LEN] = {0};
UINT16 gENABLE_TLS = 0;
UINT16 gAUTH_TYPE = 0;
char gCA_CERT_PATH[MAX_PARAMS_STR_LEN] = {0};
char gCLIENT_CERT_PATH[MAX_PARAMS_STR_LEN] = {0};
char gCLIENT_KEY_PATH[MAX_PARAMS_STR_LEN] = {0};
char gREMOTE_FOLDER[MAX_PARAMS_STR_LEN] = {0};
char gDLFILE[MAX_PARAMS_STR_LEN] = {0};
char gDLTOBUF_FILE[MAX_PARAMS_STR_LEN] = {0};

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
  char *_PDP_CDX = NULL;
  char *_FTP_ADDR = NULL;
  char *_FTP_PORT = NULL;
  char *_FTP_USER = NULL;
  char *_FTP_PASS = NULL;
  char * _ENABLETLS = NULL;
  char * _AUTHTYPE = NULL;
  char *_CA_CERT_PATH = NULL;
  char *_CLIENT_CERT_PATH = NULL;
  char *_CLIENT_KEY_PATH = NULL;
  char *_REMOTE_FOLDER = NULL;
  char *_DLFILE = NULL;
  char *_DLTOBUF_FILE = NULL;

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

    //PDP_CDX
    _PDP_CDX = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_ADDR
    _FTP_ADDR = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_PORT
    _FTP_PORT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_USER
    _FTP_USER = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //FTP_PASS
    _FTP_PASS = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //ENABLE_TLS
   _ENABLETLS = mystrtok(&p, NULL,',');
   mystrtok(&p, NULL,'\n'); //strip description

   //AUTH_TYPE
    _AUTHTYPE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CA_CERT_PATH
    _CA_CERT_PATH = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_CERT_PATH
    _CLIENT_CERT_PATH = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_KEY_PATH
    _CLIENT_KEY_PATH = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description
    //REMOTE_FOLDER
    _REMOTE_FOLDER = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //_DLFILE
    _DLFILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //_DLTOBUF_FILE
    _DLTOBUF_FILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    if( !_APN || !_APN_USERNAME || ! _APN_PASSWORD || !_PDP_CDX ||
            !_FTP_ADDR || !_FTP_PORT || ! _FTP_USER || !_FTP_PASS ||!_ENABLETLS || !_AUTHTYPE ||
            !_CA_CERT_PATH ||!_CLIENT_CERT_PATH ||!_CLIENT_KEY_PATH ||
            !_REMOTE_FOLDER || !_DLFILE || !_DLTOBUF_FILE)
    {
      AZX_LOG_CRITICAL("Cannot extract parameters from file!!\r\n");
      return -1;
    }
    else
    {
      strcpy(gAPN, _APN);
      strcpy(gAPN_UserName, _APN_USERNAME);
      strcpy(gAPN_Password, _APN_PASSWORD);

      gPDP_CTX = atoi(_PDP_CDX);

      strcpy(gFTP_ADDR, _FTP_ADDR);
      gFTP_PORT = atoi(_FTP_PORT);
      strcpy(gFTP_USER, _FTP_USER);
      strcpy(gFTP_PASS, _FTP_PASS);
      gENABLE_TLS = atoi(_ENABLETLS);

      gAUTH_TYPE = atoi(_AUTHTYPE);

      char tmp[100];
      strcpy(tmp, _CA_CERT_PATH);
      sprintf(gCA_CERT_PATH, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);
      strcpy(gCLIENT_CERT_PATH, _CLIENT_CERT_PATH);
      strcpy(gCLIENT_KEY_PATH, _CLIENT_KEY_PATH);

      strcpy(tmp, _CLIENT_CERT_PATH);
      sprintf(gCLIENT_CERT_PATH, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);

      strcpy(tmp, _CLIENT_KEY_PATH);
      sprintf(gCLIENT_KEY_PATH, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);
      strcpy(gREMOTE_FOLDER, _REMOTE_FOLDER);
      strcpy(gDLFILE,_DLFILE);
      strcpy(gDLTOBUF_FILE, _DLTOBUF_FILE);

      AZX_LOG_INFO("Set APN to: <<%s>>\r\n", gAPN);
      AZX_LOG_INFO("Set APN USER to: <<%s>>\r\n", gAPN_UserName);
      AZX_LOG_INFO("Set APN PASS to: <<%s>>\r\n", gAPN_Password);
      AZX_LOG_INFO("Set PDP_CDX to: %u\r\n", gPDP_CTX);
      AZX_LOG_INFO("Set FTP_ADDR to: <<%s>>\r\n", gFTP_ADDR);
      AZX_LOG_INFO("Set FTP_PORT to: %u\r\n", gFTP_PORT);
      AZX_LOG_INFO("Set FTP_USER to: <<%s>>\r\n", gFTP_USER);
      AZX_LOG_INFO("Set FTP_PASS to: <<%s>>\r\n", gFTP_PASS);
      AZX_LOG_INFO("Set ENABLE_TLS to: <<%d>>\r\n", gENABLE_TLS);
      AZX_LOG_INFO("Set AUTH_TYPE to: <<%d>>\r\n", gAUTH_TYPE);
      AZX_LOG_INFO("Set CA_CERT_PATH to: <<%s>>\r\n", gCA_CERT_PATH);
      AZX_LOG_INFO("Set CLIENT_CERT_PATH to: <<%s>>\r\n", gCLIENT_CERT_PATH);
      AZX_LOG_INFO("Set CLIENT_KEY_PATH to: <<%s>>\r\n", gCLIENT_KEY_PATH);
      AZX_LOG_INFO("Set REMOTE_FOLDER to: <<%s>>\r\n", gREMOTE_FOLDER);
      AZX_LOG_INFO("Set DLFILE to: <<%s>>\r\n", gDLFILE);
      AZX_LOG_INFO("Set DLTOBUF_FILE to: <<%s>>\r\n", gDLTOBUF_FILE);
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
  gPDP_CTX = PDP_CTX;

  strcpy(gFTP_ADDR, FTP_ADDR);
  gFTP_PORT = FTP_PORT;
  strcpy(gFTP_USER, FTP_USER);
  strcpy(gFTP_PASS, FTP_PASS);
  gENABLE_TLS = ENABLE_TLS;
  gAUTH_TYPE = AUTH_TYPE;

  strcpy(gCA_CERT_PATH, CA_CERT_PATH);
  strcpy(gCLIENT_CERT_PATH, CLIENT_CERT_PATH);
  strcpy(gCLIENT_KEY_PATH, CLIENT_KEY_PATH);
  strcpy(gREMOTE_FOLDER, REMOTE_FOLDER);
  strcpy(gDLFILE, DLFILE);
  strcpy(gDLTOBUF_FILE, DLTOBUF_FILE);
}
