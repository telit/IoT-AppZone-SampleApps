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

char gSSL_CERT_CA_NAME[MAX_PARAMS_STR_LEN] = {0};
char gSSL_CLIENT_NAME[MAX_PARAMS_STR_LEN] = {0};
char gCACERTFILE[MAX_PARAMS_STR_LEN] = {0};
char gCLIENTCERTFILE[MAX_PARAMS_STR_LEN] = {0};
char gCLIENTKEYFILE[MAX_PARAMS_STR_LEN] = {0};
UINT16 gUSER_SSL_AUTH = 0;
char gMQTT_BROKER_ADDRESS[MAX_PARAMS_STR_LEN] = {0};
UINT16 gMQTT_BROKER_PORT = 0;
UINT16 gMQTT_BROKER_PORT_SSL = 0;
char gCLIENT_ID[MAX_PARAMS_STR_LEN] = {0};
char gCLIENT_USERNAME[MAX_PARAMS_STR_LEN] = {0};
char gCLIENT_PASSWORD[MAX_PARAMS_STR_LEN] = {0};
UINT16 gCLIENT_TIMEOUT_SEC = 0;
UINT16 gCLIENT_KEEPALIVE_SEC = 0;
char gSUB_TOPIC[MAX_PARAMS_STR_LEN] = {0};
char gSUB_TOPIC2[MAX_PARAMS_STR_LEN] = {0};
char gPUB_MESSAGE[MAX_PARAMS_STR_LEN] = {0};

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
  char *_CACERTFILE= NULL;
  char *_CLIENTCERTFILE = NULL;
  char *_CLIENTKEYFILE = NULL;
  char *_USER_SSL_AUTH = NULL;
  char *_MQTT_BROKER_ADDRESS = NULL;
  char *_MQTT_BROKER_PORT = NULL;
  char *_MQTT_BROKER_PORT_SSL = NULL;
  char *_CLIENT_ID = NULL;
  char *_CLIENT_USERNAME = NULL;
  char *_CLIENT_PASSWORD = NULL;
  char *_CLIENT_TIMEOUT_SEC = NULL;
  char *_CLIENT_KEEPALIVE_SEC = NULL;
  char *_SUB_TOPIC = NULL;
  char *_SUB_TOPIC2 = NULL;
  char *_PUB_MESSAGE = NULL;

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

    //CACERTFILE
    _CACERTFILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENTCERTFILE
    _CLIENTCERTFILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENTKEYFILE
    _CLIENTKEYFILE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //USER_SSL_AUTH
    _USER_SSL_AUTH = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MQTT_BROKER_ADDRESS
    _MQTT_BROKER_ADDRESS = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MQTT_BROKER_PORT
    _MQTT_BROKER_PORT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //MQTT_BROKER_PORT_SSL
    _MQTT_BROKER_PORT_SSL = mystrtok(&p, NULL,',');
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

    //CLIENT_TIMEOUT_SEC
    _CLIENT_TIMEOUT_SEC = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CLIENT_KEEPALIVE_SEC
    _CLIENT_KEEPALIVE_SEC = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //SUB_TOPIC
    _SUB_TOPIC = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //SUB_TOPIC2
    _SUB_TOPIC2 = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //PUB_MESSAGE
    _PUB_MESSAGE = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    if( !_APN || !_APN_USERNAME || ! _APN_PASSWORD || !_PDP_CDX ||
            !_SSL_CERT_CA_NAME || !_SSL_CLIENT_NAME ||
            !_CACERTFILE || !_CLIENTCERTFILE ||!_CLIENTKEYFILE ||
            !_USER_SSL_AUTH ||
            !_MQTT_BROKER_ADDRESS || !_MQTT_BROKER_PORT ||!_MQTT_BROKER_PORT_SSL ||
            ! _CLIENT_ID || !_CLIENT_USERNAME || !_CLIENT_PASSWORD ||
            ! _CLIENT_TIMEOUT_SEC || !_CLIENT_KEEPALIVE_SEC ||
            !_SUB_TOPIC || !_SUB_TOPIC2 || !_PUB_MESSAGE)
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
      strcpy(gCACERTFILE, _CACERTFILE);
      strcpy(gCLIENTCERTFILE, _CLIENTCERTFILE);
      strcpy(gCLIENTKEYFILE, _CLIENTKEYFILE);

      gUSER_SSL_AUTH = atoi(_USER_SSL_AUTH);

      strcpy(gMQTT_BROKER_ADDRESS, _MQTT_BROKER_ADDRESS);

      gMQTT_BROKER_PORT = atoi(_MQTT_BROKER_PORT);
      gMQTT_BROKER_PORT_SSL = atoi(_MQTT_BROKER_PORT_SSL);

      strcpy(gCLIENT_ID, _CLIENT_ID);
      strcpy(gCLIENT_USERNAME, _CLIENT_USERNAME);
      strcpy(gCLIENT_PASSWORD, _CLIENT_PASSWORD);

      gCLIENT_TIMEOUT_SEC = atoi(_CLIENT_TIMEOUT_SEC);
      gCLIENT_KEEPALIVE_SEC = atoi(_CLIENT_KEEPALIVE_SEC);

      strcpy(gSUB_TOPIC, _SUB_TOPIC);
      strcpy(gSUB_TOPIC2, _SUB_TOPIC2);
      strcpy(gPUB_MESSAGE, _PUB_MESSAGE);

      AZX_LOG_INFO("Set APN to: <<%s>>\r\n", gAPN);
      AZX_LOG_INFO("Set APN USER to: <<%s>>\r\n", gAPN_UserName);
      AZX_LOG_INFO("Set APN PASS to: <<%s>>\r\n", gAPN_Password);
      AZX_LOG_INFO("Set PDP_CDX to: %u\r\n", gPDP_CTX);

      AZX_LOG_INFO("Set SSL_CERT_CA_NAME to: <<%s>>\r\n", gSSL_CERT_CA_NAME);
      AZX_LOG_INFO("Set SSL_CLIENT_NAME to: <<%s>>\r\n", gSSL_CLIENT_NAME);
      AZX_LOG_INFO("Set CACERTFILE to: <<%s>>\r\n", gCACERTFILE);
      AZX_LOG_INFO("Set CLIENTCERTFILE to: <<%s>>\r\n", gCLIENTCERTFILE);
      AZX_LOG_INFO("Set CLIENTKEYFILE to: <<%s>>\r\n", gCLIENTKEYFILE);
      AZX_LOG_INFO("Set USER_SSL_AUTH to: %u\r\n", gUSER_SSL_AUTH);
      AZX_LOG_INFO("Set MQTT_BROKER_ADDRESS to: <<%s>>\r\n", gMQTT_BROKER_ADDRESS);
      AZX_LOG_INFO("Set MQTT_BROKER_PORT to: %u\r\n", gMQTT_BROKER_PORT);
      AZX_LOG_INFO("Set MQTT_BROKER_PORT_SSL to: %u\r\n", gMQTT_BROKER_PORT_SSL);
      AZX_LOG_INFO("Set CLIENT_ID to: <<%s>>\r\n", gCLIENT_ID);
      AZX_LOG_INFO("Set CLIENT_USERNAME to: <<%s>>\r\n", gCLIENT_USERNAME);
      AZX_LOG_INFO("Set CLIENT_PASSWORD to: <<%s>>\r\n", gCLIENT_PASSWORD);
      AZX_LOG_INFO("Set CLIENT_TIMEOUT_SEC to: %u\r\n", gCLIENT_TIMEOUT_SEC);
      AZX_LOG_INFO("Set CLIENT_KEEPALIVE_SEC to: %u\r\n", gCLIENT_KEEPALIVE_SEC);
      AZX_LOG_INFO("Set SUB_TOPIC to: <<%s>>\r\n", gSUB_TOPIC);
      AZX_LOG_INFO("Set SUB_TOPIC2 to: <<%s>>\r\n", gSUB_TOPIC2);
      AZX_LOG_INFO("Set PUB_MESSAGE to: <<%s>>\r\n", gPUB_MESSAGE);
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
  strcpy(gCACERTFILE, CACERTFILE);
  strcpy(gCLIENTCERTFILE, CLIENTCERTFILE);
  strcpy(gCLIENTKEYFILE, CLIENTKEYFILE);
  gUSER_SSL_AUTH = USER_SSL_AUTH;
  strcpy(gMQTT_BROKER_ADDRESS, MQTT_BROKER_ADDRESS);
  gMQTT_BROKER_PORT = MQTT_BROKER_PORT;
  gMQTT_BROKER_PORT_SSL = MQTT_BROKER_PORT_SSL;
  strcpy(gCLIENT_ID, CLIENT_ID);
  strcpy(gCLIENT_USERNAME, CLIENT_USERNAME);
  strcpy(gCLIENT_PASSWORD, CLIENT_PASSWORD);
  gCLIENT_TIMEOUT_SEC = CLIENT_TIMEOUT_SEC;
  gCLIENT_KEEPALIVE_SEC = CLIENT_KEEPALIVE_SEC;
  strcpy(gSUB_TOPIC, SUB_TOPIC);
  strcpy(gSUB_TOPIC2, SUB_TOPIC2);
  strcpy(gPUB_MESSAGE, PUB_MESSAGE);
}
