/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */

/**
  @file
    read_parameters.h

  @brief
    read parameters definitions

  @details


  @note
    Dependencies:
    m2mb_types.h

  @author


  @date
    21/10/2022
 */


#ifndef HDR_READ_PARAMETERS_H_
#define HDR_READ_PARAMETERS_H_

/* Global declarations ==========================================================================*/
#define APN      "internet"
#define APN_USER ""
#define APN_PASS ""
#define PDP_CTX   (UINT8)3

#define HTTP_443 1
#if HTTP_443 //https with server authentication only
#define SERVER_PORT 443 //https
#define SERVER "modules.telit.com"
#define CA_CERT_PATH LOCALPATH "/ssl_certs/modulesCA.crt"
#define CLIENT_CERT_PATH ""
#define CLIENT_KEY_PATH ""

#else //https with client cert
#define SERVER "modules.telit.com"
#define CA_CERT_PATH LOCALPATH "/ssl_certs/modulesCA.crt"

#define CLIENT_CERT_PATH LOCALPATH "/ssl_certs/modulesClient.crt"
#define CLIENT_KEY_PATH LOCALPATH "/ssl_certs/modulesClient_pkcs1.key"  //Only RSA Private keys are supported

#define SERVER_PORT 20443 //echo client+server
M2MB_SSL_AUTH_TYPE_E SSL_AUTH_MODE  = M2MB_SSL_SERVER_CLIENT_AUTH;
CHAR  queryBuf[] = "GET / HTTP/1.1\r\nHost: modules.telit.com\r\n\r\n";
#endif

#define CONFIG_FILE  LOCALPATH "/TLS_SSL_CLIENT_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gPDP_CTX;

extern char gSERVER[MAX_PARAMS_STR_LEN];
extern UINT16 gSERVER_PORT;
extern char gCA_CERT_PATH[MAX_PARAMS_STR_LEN];
extern char gCLIENT_CERT_PATH[MAX_PARAMS_STR_LEN];
extern char gCLIENT_KEY_PATH[MAX_PARAMS_STR_LEN];;

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
