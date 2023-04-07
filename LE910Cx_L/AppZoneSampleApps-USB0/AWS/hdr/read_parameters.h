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

#define SSL_CERT_CA_NAME "ca-cert-pool"
#define SSL_CLIENT_NAME "SSL-Client"

#define HOSTMISMATCH_ENABLE 0
#define SNI_ENABLE 1

#define CLIENT_TIMEOUT_SEC 60 /*operations timeout*/
#define CLIENT_KEEPALIVE_SEC 60 /*KeepAlive timeout*/


#define USER_SSL_AUTH      2

/* SSL */
#define MQTT_BROKER_PORT_SSL    (UINT32)8883

/* Configure your certificates names, server URL and credentials */

/*Note: LOCALPATH define in app_cfg.h file depends on the family*/
#define CACERTPREFILE     LOCALPATH "/ssl_certs/preload_CACert_01.crt"   /* Root CA file path in module filesystem */
#define CACERTFILE        LOCALPATH "/ssl_certs/Amazon-IoT.crt"   /* Cross signed CA cert, it could be different from the provided one */
#define CLIENTCERTFILE    LOCALPATH "/ssl_certs/xxxxx.crt"   /* Client certificate file path in module filesystem  */
#define CLIENTKEYFILE     LOCALPATH "/ssl_certs/xxxxx.key"   /* Client private key file path in module filesystem  */

/* Server configuration */
#define AWS_BROKER_ADDRESS "xxxxxxxx.amazonaws.com"

/* Client Configuration */
#define CLIENT_ID "my_client_id"
#define CLIENT_USERNAME ""
#define CLIENT_PASSWORD ""


#define PUB_TOPIC_TEMPLATE "device/%s/updates" /*will be filled with module IMEI number*/

/* PDP configuration */
#define APN      "NTX17.NET"
#define APN_USER ""
#define APN_PASS ""

#define PDP_CTX  (UINT8)3

#define CONFIG_FILE  LOCALPATH "/AWS_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gPDP_CTX;

extern char gSSL_CERT_CA_NAME[MAX_PARAMS_STR_LEN];
extern char gSSL_CLIENT_NAME[MAX_PARAMS_STR_LEN];
extern UINT8 gHOSTMISMATCH_ENABLE;
extern UINT8 gSNI_ENABLE;
extern UINT16 gCLIENT_TIMEOUT_SEC;
extern UINT16 gCLIENT_KEEPALIVE_SEC;
extern UINT16 gUSER_SSL_AUTH;
extern UINT16 gMQTT_BROKER_PORT_SSL;
extern char gCACERTPREFILE[MAX_PARAMS_STR_LEN];
extern char gCACERTFILE[MAX_PARAMS_STR_LEN];
extern char gCLIENTCERTFILE[MAX_PARAMS_STR_LEN];
extern char gCLIENTKEYFILE[MAX_PARAMS_STR_LEN];
extern char gAWS_BROKER_ADDRESS[MAX_PARAMS_STR_LEN];
extern char gCLIENT_ID[MAX_PARAMS_STR_LEN];
extern char gCLIENT_USERNAME[MAX_PARAMS_STR_LEN];
extern char gCLIENT_PASSWORD[MAX_PARAMS_STR_LEN];
extern char gPUB_TOPIC_TEMPLATE[MAX_PARAMS_STR_LEN];

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
