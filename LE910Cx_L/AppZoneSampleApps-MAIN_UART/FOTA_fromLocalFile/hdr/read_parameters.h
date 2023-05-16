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

#define FOTA_WRITING_FILE_NAME LOCALPATH "/delta.bin"
#define FOTA_UP_INFO_PATH LOCALPATH "/fota_up_info.txt"

#define CONFIG_FILE  LOCALPATH "/FOTA_fromLocalFile_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gFOTA_WRITING_FILE_NAME[MAX_PARAMS_STR_LEN];
extern char gFOTA_UP_INFO_PATH[MAX_PARAMS_STR_LEN];

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
