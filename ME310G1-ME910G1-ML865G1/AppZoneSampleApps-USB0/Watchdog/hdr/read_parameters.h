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
#define WAKE_UP_TICKS 20       //to be used in m2mb_wDog_enable
#define CTRL_TICKS_TO_REBOOT 6 //to be used in m2mb_wDog_enable
#define WD_TOUT_COUNT 3        //to be used in m2mb_wDog_addTask

#define TIMER_TOUT 5000

#define CONFIG_FILE  LOCALPATH "/Watchdog_config.txt"

extern UINT16 gWAKE_UP_TICKS;
extern UINT16 gCTRL_TICKS_TO_REBOOT;
extern UINT16 gWD_TOUT_COUNT;
extern UINT16 gTIMER_TOUT;

/* Global typedefs ==============================================================================*/


/* Global functions =============================================================================*/
int readConfigFromFile(void);
char *mystrtok(char **m, char *s, char c);
void configureParameters(void);

#endif /* HDR_READ_PARAMETERS_H_ */
