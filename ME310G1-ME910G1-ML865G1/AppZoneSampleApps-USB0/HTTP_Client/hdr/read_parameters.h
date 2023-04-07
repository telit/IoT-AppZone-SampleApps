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
#define CID (UINT8)3

/* SSL Configuration */
#define CACERTFILE        ""   /* Root CA file path in module filesystem (if needed) */
#define CLIENTCERTFILE    ""   /* Client certificate file path in module filesystem (if needed) */
#define CLIENTKEYFILE     ""   /* Client private key file path in module filesystem (if needed) */

/* Request configuration*/
#define REQUEST_TYPE (UINT8)2 /* HTTP_GET: Perform HTTP GET without SSL */
#define SERVER "Server"

#define CONFIG_FILE  LOCALPATH "/HTTP_Client_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gCID;

extern char gCACERTFILE[MAX_PARAMS_STR_LEN];
extern char gCLIENTCERTFILE[MAX_PARAMS_STR_LEN];
extern char gCLIENTKEYFILE[MAX_PARAMS_STR_LEN];
extern UINT16 gREQUEST_TYPE;
extern char gSERVER[MAX_PARAMS_STR_LEN];

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
