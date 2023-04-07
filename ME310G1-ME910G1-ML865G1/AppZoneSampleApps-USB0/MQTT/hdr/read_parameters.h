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

#define APN  "web.omnitel.it"
#define APN_USER ""
#define APN_PASS ""
#define PDP_CTX (UINT8)3

#define SSL_CERT_CA_NAME "ca-cert-pool"
#define SSL_CLIENT_NAME "SSL-Client"
/* SSL */
/*Note: A possible root path for certificates could be LOCALPATH define in app_cfg.h file */
#define CACERTFILE        ""   /* Root CA file path in module filesystem (if needed) */
#define CLIENTCERTFILE    ""   /* Client certificate file path in module filesystem (if needed) */
#define CLIENTKEYFILE     ""   /* Client private key file path in module filesystem (if needed) */
#define USER_SSL_AUTH      0
/* Server configuration */
#define MQTT_BROKER_ADDRESS "api-dev.devicewise.com"
#define MQTT_BROKER_PORT        (UINT32)1883
#define MQTT_BROKER_PORT_SSL    (UINT32)8883

/* Client Configuration */
#define CLIENT_ID "test_m2mb_mqtt_id"
#define CLIENT_USERNAME "test_m2mb_mqtt"
#define CLIENT_PASSWORD "q3XYKetChZRdGuKF"

#define CLIENT_TIMEOUT_SEC 60 /*operations timeout*/
#define CLIENT_KEEPALIVE_SEC 60 /*KeepAlive timeout*/

#define SUB_TOPIC "test_topic"
#define SUB_TOPIC2 "test_topic2"

#define PUB_MESSAGE "Hello from M2MB MQTT!"

#define CONFIG_FILE  LOCALPATH "/MQTT_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gPDP_CTX;

extern char gSSL_CERT_CA_NAME[MAX_PARAMS_STR_LEN];
extern char gSSL_CLIENT_NAME[MAX_PARAMS_STR_LEN];
extern char gCACERTFILE[MAX_PARAMS_STR_LEN];
extern char gCLIENTCERTFILE[MAX_PARAMS_STR_LEN];
extern char gCLIENTKEYFILE[MAX_PARAMS_STR_LEN];
extern UINT16 gUSER_SSL_AUTH;
extern char gMQTT_BROKER_ADDRESS[MAX_PARAMS_STR_LEN];
extern UINT16 gMQTT_BROKER_PORT;
extern UINT16 gMQTT_BROKER_PORT_SSL;
extern char gCLIENT_ID[MAX_PARAMS_STR_LEN];
extern char gCLIENT_USERNAME[MAX_PARAMS_STR_LEN];
extern char gCLIENT_PASSWORD[MAX_PARAMS_STR_LEN];
extern UINT16 gCLIENT_TIMEOUT_SEC;
extern UINT16 gCLIENT_KEEPALIVE_SEC;
extern char gSUB_TOPIC[MAX_PARAMS_STR_LEN];
extern char gSUB_TOPIC2[MAX_PARAMS_STR_LEN];
extern char gPUB_MESSAGE[MAX_PARAMS_STR_LEN];

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
