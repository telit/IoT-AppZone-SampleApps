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

#define FTP_ADDR "x.x.x.x"
#define FTP_PORT 21

#define FTP_USER "user"
#define FTP_PASS "password"
#define ENABLE_TLS 0
#define AUTH_TYPE 0  //NO AUTH
#define CA_CERT_PATH ""
#define CLIENT_CERT_PATH ""
#define CLIENT_KEY_PATH ""
#define REMOTE_FOLDER "/samples"

//#define DLFILE "pattern_big.txt"

/*Local path where the file will be downloaded is defined in app_cfg.h LOCALPATH variable*/

/*Remote path of file on FTP server to be downloaded in a buffer*/
#define DLTOBUF_FILE "pattern.txt"
/*Local file storing the status of FOTA process*/
#define FOTA_STATUS_FILE "/mod/status.txt"
#define CONFIG_FILE  LOCALPATH "/FOTA_FTPclient_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gPDP_CTX;

extern char gFTP_ADDR[MAX_PARAMS_STR_LEN];
extern UINT16 gFTP_PORT;
extern char gFTP_USER[MAX_PARAMS_STR_LEN];
extern char gFTP_PASS[MAX_PARAMS_STR_LEN];
extern UINT16 gENABLE_TLS;
extern UINT16 gAUTH_TYPE;
extern char gCA_CERT_PATH[MAX_PARAMS_STR_LEN];
extern char gCLIENT_CERT_PATH[MAX_PARAMS_STR_LEN];
extern char gCLIENT_KEY_PATH[MAX_PARAMS_STR_LEN];
extern char gREMOTE_FOLDER[MAX_PARAMS_STR_LEN];
extern char gDLFILE[MAX_PARAMS_STR_LEN];
extern char gDLTOBUF_FILE[MAX_PARAMS_STR_LEN];
extern char gFOTA_STATUS_FILE[MAX_PARAMS_STR_LEN];
/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
