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

#define NTP_SERVER "0.pool.ntp.org"
#define NTP_PORT  123
#define TIMEOUT   5

#define CONFIG_FILE  LOCALPATH "/NTP_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gPDP_CTX;

extern char gNTP_SERVER[MAX_PARAMS_STR_LEN];
extern UINT16 gNTP_PORT;
extern UINT16 gTIMEOUT;

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
