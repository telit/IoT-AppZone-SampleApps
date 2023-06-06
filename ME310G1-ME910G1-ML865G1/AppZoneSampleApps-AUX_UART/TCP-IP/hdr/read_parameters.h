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

#define SERVER "server"
#define SERVER_PORT 10510

#define APN      "internet"
#define APN_USER ""
#define APN_PASS ""
#define PDP_CTX   (UINT8)3

#define CONFIG_FILE  LOCALPATH "/TCP_IP_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gPDP_CTX;
extern char gServer[MAX_PARAMS_STR_LEN];
extern UINT16 gServer_Port;


/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
