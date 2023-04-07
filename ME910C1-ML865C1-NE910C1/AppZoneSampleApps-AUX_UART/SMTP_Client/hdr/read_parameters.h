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
    Cristina Desogus

  @date
    21/12/2022
 */


#ifndef HDR_READ_PARAMETERS_H_
#define HDR_READ_PARAMETERS_H_

/* Global declarations ==========================================================================*/

#define APN      		    "internet"
#define APN_USER 		    ""
#define APN_PASS 		    ""
#define PDP_CTX   		  (UINT8)1
#define MAIL_SERVER		  "example.server.com"
#define MAIL_PORT		    "25"
#define MAIL_USER		    "exampleUser1@exampledomain.com"
#define MAIL_PASS       "exampleUserPwd1"
#define MAIL_FROM       "exampleUser1@exampledomain.com"
#define MAIL_FROM_NAME  "ExUser1"
#define MAIL_SUBJECT    "Hello from AppZone"
#define MAIL_BODY       "AppZone Test"
#define MAIL_TO         "exampleUser2@exampledomain.com"
#define MAIL_TO_NAME    "ExUser2"

#define CONFIG_FILE  LOCALPATH "/SMTP_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gAPN[MAX_PARAMS_STR_LEN];
extern char gAPN_UserName[MAX_PARAMS_STR_LEN];
extern char gAPN_Password[MAX_PARAMS_STR_LEN];
extern UINT16 gPDP_CTX;

extern char gMAIL_SERVER[MAX_PARAMS_STR_LEN];
extern char gMAIL_PORT[MAX_PARAMS_STR_LEN];
extern char gMAIL_USER[MAX_PARAMS_STR_LEN];
extern char gMAIL_PASS[MAX_PARAMS_STR_LEN];
extern char gMAIL_FROM[MAX_PARAMS_STR_LEN];
extern char gMAIL_FROM_NAME[MAX_PARAMS_STR_LEN];
extern char gMAIL_SUBJECT[MAX_PARAMS_STR_LEN];
extern char gMAIL_BODY[MAX_PARAMS_STR_LEN];
extern char gMAIL_TO[MAX_PARAMS_STR_LEN];
extern char gMAIL_TO_NAME[MAX_PARAMS_STR_LEN];
/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
