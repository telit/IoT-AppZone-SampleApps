/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    read_parameters.c

  @brief
    The file contains the parameters reading utilities

  @details

  @version 
    1.0.1

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

char gSSL_CERT_CA_NAME[MAX_PARAMS_STR_LEN] = {0};
char gSSL_CLIENT_NAME[MAX_PARAMS_STR_LEN] = {0};
UINT8 gHOSTMISMATCH_ENABLE = 0;
UINT8 gSNI_ENABLE = 0;
UINT16 gCLIENT_TIMEOUT_SEC = 0;
UINT16 gCLIENT_KEEPALIVE_SEC = 0;
UINT16 gUSER_SSL_AUTH = 0;
UINT16 gMQTT_BROKER_PORT_SSL = 0;
char gCACERTPREFILE[MAX_PARAMS_STR_LEN] = {0};
char gCACERTFILE[MAX_PARAMS_STR_LEN] = {0};
char gCLIENTCERTFILE[MAX_PARAMS_STR_LEN] = {0};
char gCLIENTKEYFILE[MAX_PARAMS_STR_LEN] = {0};
char gAWS_BROKER_ADDRESS[MAX_PARAMS_STR_LEN] = {0};
char gCLIENT_ID[MAX_PARAMS_STR_LEN] = {0};
char gCLIENT_USERNAME[MAX_PARAMS_STR_LEN] = {0};
char gCLIENT_PASSWORD[MAX_PARAMS_STR_LEN] = {0};
char gPUB_TOPIC_TEMPLATE[MAX_PARAMS_STR_LEN] = {0};
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

  char *_SSL_CERT_CA_NAME = NULL;
  char *_SSL_CLIENT_NAME = NULL;
  char *_HOSTMISMATCH_ENABLE = NULL;
  char *_SNI_ENABLE = NULL;
  char *_CLIENT_TIMEOUT_SEC = NULL;
  char *_CLIENT_KEEPALIVE_SEC = NULL;
  char *_USER_SSL_AUTH = NULL;
  char *_MQTT_BROKER_PORT_SSL = NULL;
  char *_CACERTPREFILE = NULL;
  char *_CACERTFILE = NULL;
  char *_CLIENTCERTFILE = NULL;
  char *_CLIENTKEYFILE = NULL;
  char *_AWS_BROKER_ADDRESS = NULL;
  char *_CLIENT_ID = NULL;
  char *_CLIENT_USERNAME = NULL;
  char *_CLIENT_PASSWORD = NULL;
  char *_PUB_TOPIC_TEMPLATE = NULL;

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

    //SSL_CERT_CA_NAME
    _SSL_CERT_CA_NAME = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //SSL_CLIENT_NAME
    _SSL_CLIENT_NAME = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //HOSTMISMATCH_ENABLE
    _HOSTMISMATCH_ENABLE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //SNI_ENABLE
    _SNI_ENABLE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_TIMEOUT_SEC
    _CLIENT_TIMEOUT_SEC = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_KEEPALIVE_SEC
    _CLIENT_KEEPALIVE_SEC = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //USER_SSL_AUTH
    _USER_SSL_AUTH = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MQTT_BROKER_PORT_SSL
    _MQTT_BROKER_PORT_SSL = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CACERTPREFILE
    _CACERTPREFILE = mystrtok(&p, NULL,',');
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

    //AWS_BROKER_ADDRESS
    _AWS_BROKER_ADDRESS = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_ID
    _CLIENT_ID = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_USERNAME
    _CLIENT_USERNAME = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_PASSWORD
    _CLIENT_PASSWORD = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //PUB_TOPIC_TEMPLATE
    _PUB_TOPIC_TEMPLATE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    if( !_APN || !_APN_USERNAME || ! _APN_PASSWORD || !_PDP_CDX ||
            !_SSL_CERT_CA_NAME || !_SSL_CLIENT_NAME ||
            !_HOSTMISMATCH_ENABLE||! _SNI_ENABLE||
            ! _CLIENT_TIMEOUT_SEC || !_CLIENT_KEEPALIVE_SEC ||
            !_USER_SSL_AUTH || ! _MQTT_BROKER_PORT_SSL ||
            !_CACERTPREFILE || !_CACERTFILE ||
            !_CLIENTCERTFILE || !_CLIENTKEYFILE || !_AWS_BROKER_ADDRESS||
            ! _CLIENT_ID || !_CLIENT_USERNAME || !_CLIENT_PASSWORD || !_PUB_TOPIC_TEMPLATE)
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

      strcpy(gSSL_CERT_CA_NAME, _SSL_CERT_CA_NAME);
      strcpy(gSSL_CLIENT_NAME, _SSL_CLIENT_NAME);

      gHOSTMISMATCH_ENABLE = atoi(_HOSTMISMATCH_ENABLE);
      gSNI_ENABLE = atoi(_SNI_ENABLE);
      gCLIENT_TIMEOUT_SEC = atoi(_CLIENT_TIMEOUT_SEC);
      gCLIENT_KEEPALIVE_SEC = atoi(_CLIENT_KEEPALIVE_SEC);
      gUSER_SSL_AUTH = atoi(_USER_SSL_AUTH);
      gMQTT_BROKER_PORT_SSL = atoi(_MQTT_BROKER_PORT_SSL);

      char tmp[100];
      strcpy(tmp, _CACERTPREFILE);
      sprintf(gCACERTPREFILE, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);

      strcpy(tmp, _CACERTFILE);
      sprintf(gCACERTFILE, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);

      strcpy(tmp, _CLIENTCERTFILE);
      sprintf(gCLIENTCERTFILE, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);

      strcpy(tmp, _CLIENTKEYFILE);
      sprintf(gCLIENTKEYFILE, "%s%s",LOCALPATH,tmp);
      memset(tmp, 0, 100);

      strcpy(gAWS_BROKER_ADDRESS, _AWS_BROKER_ADDRESS);
      strcpy(gCLIENT_ID, _CLIENT_ID);
      strcpy(gCLIENT_USERNAME, _CLIENT_USERNAME);
      strcpy(gCLIENT_PASSWORD, _CLIENT_PASSWORD);
      strcpy(gPUB_TOPIC_TEMPLATE, _PUB_TOPIC_TEMPLATE);

      AZX_LOG_INFO("Set APN to: <<%s>>\r\n", gAPN);
      AZX_LOG_INFO("Set APN USER to: <<%s>>\r\n", gAPN_UserName);
      AZX_LOG_INFO("Set APN PASS to: <<%s>>\r\n", gAPN_Password);
      AZX_LOG_INFO("Set PDP_CDX to: %u\r\n", gPDP_CTX);
      AZX_LOG_INFO("Set SSL_CERT_CA_NAME to: <<%s>>\r\n", gSSL_CERT_CA_NAME);
      AZX_LOG_INFO("Set SSL_CLIENT_NAME to: <<%s>>\r\n", gSSL_CLIENT_NAME);
      AZX_LOG_INFO("Set HOSTMISMATCH_ENABLE to: %u\r\n", gHOSTMISMATCH_ENABLE);
      AZX_LOG_INFO("Set SNI_ENABLE to: %u\r\n", gSNI_ENABLE);
      AZX_LOG_INFO("Set CLIENT_TIMEOUT_SEC to: %u\r\n", gCLIENT_TIMEOUT_SEC);
      AZX_LOG_INFO("Set CLIENT_KEEPALIVE_SEC to: %u\r\n", gCLIENT_KEEPALIVE_SEC);
      AZX_LOG_INFO("Set USER_SSL_AUTH to: %u\r\n", gUSER_SSL_AUTH);
      AZX_LOG_INFO("Set MQTT_BROKER_PORT_SSL to: %u\r\n", gMQTT_BROKER_PORT_SSL);
      AZX_LOG_INFO("Set CACERTPREFILE to: <<%s>>\r\n", gCACERTPREFILE);
      AZX_LOG_INFO("Set CACERTFILE to: <<%s>>\r\n", gCACERTFILE);
      AZX_LOG_INFO("Set CLIENTCERTFILE to: <<%s>>\r\n", gCLIENTCERTFILE);
      AZX_LOG_INFO("Set CLIENTKEYFILE to: <<%s>>\r\n", gCLIENTKEYFILE);
      AZX_LOG_INFO("Set AWS_BROKER_ADDRESS to: <<%s>>\r\n", gAWS_BROKER_ADDRESS);
      AZX_LOG_INFO("Set CLIENT_ID to: <<%s>>\r\n", gCLIENT_ID);
      AZX_LOG_INFO("Set CLIENT_USERNAME to: <<%s>>\r\n", gCLIENT_USERNAME);
      AZX_LOG_INFO("Set CLIENT_PASSWORD to: <<%s>>\r\n", gCLIENT_PASSWORD);
      AZX_LOG_INFO("Set PUB_TOPIC_TEMPLATE to: <<%s>>\r\n", gPUB_TOPIC_TEMPLATE);
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

  strcpy(gSSL_CERT_CA_NAME, SSL_CERT_CA_NAME);
  strcpy(gSSL_CLIENT_NAME, SSL_CLIENT_NAME);

  gHOSTMISMATCH_ENABLE = HOSTMISMATCH_ENABLE;
  gSNI_ENABLE = SNI_ENABLE;
  gCLIENT_TIMEOUT_SEC = CLIENT_TIMEOUT_SEC;
  gCLIENT_KEEPALIVE_SEC = CLIENT_KEEPALIVE_SEC;
  gUSER_SSL_AUTH = USER_SSL_AUTH;
  gMQTT_BROKER_PORT_SSL = MQTT_BROKER_PORT_SSL;

  strcpy(gCACERTPREFILE, CACERTPREFILE);
  strcpy(gCACERTFILE, CACERTFILE);
  strcpy(gCLIENTCERTFILE, CLIENTCERTFILE);
  strcpy(gCLIENTKEYFILE, CLIENTKEYFILE);
  strcpy(gAWS_BROKER_ADDRESS, AWS_BROKER_ADDRESS);
  strcpy(gCLIENT_ID, CLIENT_ID);
  strcpy(gCLIENT_USERNAME, CLIENT_USERNAME);
  strcpy(gCLIENT_PASSWORD, CLIENT_PASSWORD);
  strcpy(gPUB_TOPIC_TEMPLATE, PUB_TOPIC_TEMPLATE);
}
