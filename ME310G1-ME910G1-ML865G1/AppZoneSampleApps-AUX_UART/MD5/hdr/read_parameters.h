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

#define FILE LOCALPATH "/file.txt"

#define DATA_STRING "the quick brown fox jumped over the lazy dog."
#define RIGHT_MD5_HASH "bb0fa6eff92c305f166803b6938dd33a"

#define CONFIG_FILE  LOCALPATH "/MD5_config.txt"

#define MAX_PARAMS_STR_LEN 100

extern char gFILE[MAX_PARAMS_STR_LEN];
extern char gDATA_STRING[MAX_PARAMS_STR_LEN];
extern char gRIGHT_MD5_HASH[MAX_PARAMS_STR_LEN];

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
