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

#define SENDER_NUMBER "+391234567890"   //the phone number in international format
//#define SENDER_NUMBER "1234567890"    //the phone number in national format
#define MESSAGE "How are you?"

#define CONFIG_FILE  LOCALPATH "/SMS_PDU_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gSENDER_NUMBER[MAX_PARAMS_STR_LEN];
extern char gMESSAGE[MAX_PARAMS_STR_LEN];

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
