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
    Cristina Desogus

  @date
    21/12/2022
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


char gAPN[MAX_PARAMS_STR_LEN];
char gAPN_UserName[MAX_PARAMS_STR_LEN];
char gAPN_Password[MAX_PARAMS_STR_LEN];
UINT16 gPDP_CTX;

char gMAIL_SERVER[MAX_PARAMS_STR_LEN];
char gMAIL_PORT[MAX_PARAMS_STR_LEN];
char gMAIL_USER[MAX_PARAMS_STR_LEN];
char gMAIL_PASS[MAX_PARAMS_STR_LEN];
char gMAIL_FROM[MAX_PARAMS_STR_LEN];
char gMAIL_FROM_NAME[MAX_PARAMS_STR_LEN];
char gMAIL_SUBJECT[MAX_PARAMS_STR_LEN];
char gMAIL_BODY[MAX_PARAMS_STR_LEN];
char gMAIL_TO[MAX_PARAMS_STR_LEN];
char gMAIL_TO_NAME[MAX_PARAMS_STR_LEN];
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

  char *_APN;
  char *_APN_USERNAME;
  char *_APN_PASSWORD;
  char *_PDP_CTX;

  char *_MAIL_SERVER;
  char *_MAIL_PORT;
  char *_MAIL_USER;
  char *_MAIL_PASS;
  char *_MAIL_FROM;
  char *_MAIL_FROM_NAME;
  char *_MAIL_SUBJECT;
  char *_MAIL_BODY;
  char *_MAIL_TO;
  char *_MAIL_TO_NAME;

  AZX_LOG_DEBUG("Reading parameters from file\r\n");

  AZX_LOG_DEBUG("Opening %s in read mode..\r\n", CONFIG_FILE);

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

    //PDP_CTX
    _PDP_CTX = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_SERVER
    _MAIL_SERVER = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_PORT
    _MAIL_PORT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_USER
    _MAIL_USER = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_PASS
    _MAIL_PASS = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_FROM
    _MAIL_FROM = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_FROM_NAME
    _MAIL_FROM_NAME = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_SUBJECT
    _MAIL_SUBJECT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_BODY
    _MAIL_BODY = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_TO
    _MAIL_TO = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MAIL_TO_NAME
    _MAIL_TO_NAME  = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    if( !_APN || !_APN_USERNAME || !_APN_PASSWORD || !_PDP_CTX || !_MAIL_SERVER || !_MAIL_PORT
    		|| !_MAIL_USER || !_MAIL_PASS || !_MAIL_FROM || !_MAIL_FROM_NAME || !_MAIL_SUBJECT
			|| !_MAIL_BODY || !_MAIL_TO ||!_MAIL_TO_NAME)
    {
      AZX_LOG_CRITICAL("Cannot extract parameters from file!!\r\n");
      return -1;
    }
    else
    {
      AZX_LOG_INFO("Reading parameters from file...\r\n");

      strcpy(gAPN, _APN);
      strcpy(gAPN_UserName, _APN_USERNAME);
      strcpy(gAPN_Password, _APN_PASSWORD);

      gPDP_CTX = atoi(_PDP_CTX);

      strcpy(gMAIL_SERVER, _MAIL_SERVER);
      strcpy(gMAIL_PORT, _MAIL_PORT);
      strcpy(gMAIL_USER, _MAIL_USER);
      strcpy(gMAIL_PASS, _MAIL_PASS);
      strcpy(gMAIL_FROM, _MAIL_FROM);
      strcpy(gMAIL_FROM_NAME, _MAIL_FROM_NAME);
      strcpy(gMAIL_SUBJECT, _MAIL_SUBJECT);
      strcpy(gMAIL_BODY, _MAIL_BODY);
      strcpy(gMAIL_TO, _MAIL_TO);
      strcpy(gMAIL_TO_NAME, _MAIL_TO_NAME);



      AZX_LOG_INFO("Set APN to: <<%s>>\r\n", gAPN);
      AZX_LOG_INFO("Set APN USER to: <<%s>>\r\n", gAPN_UserName);
      AZX_LOG_INFO("Set APN PASS to: <<%s>>\r\n", gAPN_Password);
      AZX_LOG_INFO("Set PDP_CTX to: %u\r\n", gPDP_CTX);
      AZX_LOG_INFO("Set MAIL SERVER to: <<%s>>\r\n", gMAIL_SERVER);
      AZX_LOG_INFO("Set MAIL PORT to: <<%s>>\r\n", gMAIL_PORT);
      AZX_LOG_INFO("Set MAIL USER to: <<%s>>\r\n", gMAIL_USER);
      AZX_LOG_INFO("Set MAIL PASS to: <<%s>>\r\n", gMAIL_PASS);
      AZX_LOG_INFO("Set MAIL FROM to: <<%s>>\r\n", gMAIL_FROM);
      AZX_LOG_INFO("Set MAIL FROM NAME to: <<%s>>\r\n", gMAIL_FROM_NAME);
      AZX_LOG_INFO("Set MAIL SUBJECT to: <<%s>>\r\n", gMAIL_SUBJECT);
      AZX_LOG_INFO("Set MAIL BODY to: <<%s>>\r\n", gMAIL_BODY);
      AZX_LOG_INFO("Set MAIL TO to: <<%s>>\r\n", gMAIL_TO);
      AZX_LOG_INFO("Set MAIL TO NAME to: <<%s>>\r\n", gMAIL_TO_NAME);
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

  strcpy(gMAIL_SERVER, MAIL_SERVER);
  strcpy(gMAIL_PORT, MAIL_PORT);
  strcpy(gMAIL_USER, MAIL_USER);
  strcpy(gMAIL_PASS, MAIL_PASS);
  strcpy(gMAIL_FROM, MAIL_FROM);
  strcpy(gMAIL_FROM_NAME, MAIL_FROM_NAME);
  strcpy(gMAIL_SUBJECT, MAIL_SUBJECT);
  strcpy(gMAIL_BODY, MAIL_BODY);
  strcpy(gMAIL_TO, MAIL_TO);
  strcpy(gMAIL_TO_NAME, MAIL_TO_NAME);
}


