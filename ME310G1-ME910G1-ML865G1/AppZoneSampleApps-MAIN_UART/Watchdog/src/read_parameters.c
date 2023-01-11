/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    read_parameters.c

  @brief
    The file contains the parameters reading utilities

  @details

  @version 
    1.0.0

  @note

  @author


  @date
    21/10/2022
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "m2mb_os_api.h"

#include "m2mb_fs_posix.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h" /*FOR LOCALPATH define*/

#include "read_parameters.h"

UINT16 gWAKE_UP_TICKS;
UINT16 gCTRL_TICKS_TO_REBOOT;
UINT16 gWD_TOUT_COUNT;
UINT16 gTIMER_TOUT;

/* Macro =============================================================================*/

/* Local defines ================================================================================*/

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/


/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/
char *mystrtok(char **m, char *s, char c)
{
  char *p = s?s:*m;
  if( !p )
  {
    return 0;
  }

  if( !*p )
  {
    return 0;
  }
  *m = strchr(p,c);
  if( *m )
    *(*m)++=0;
  else
    *m = p+strlen(p);
  return p;
}

int readConfigFromFile(void)
{
  INT32 fd = -1;
  INT32 fs_res;
  CHAR recv[512];

  char *p = NULL;

  char *_WAKE_UP_TICKS = NULL;
  char *_CTRL_TICKS_TO_REBOOT = NULL;
  char *_WD_TOUT_COUNT = NULL;
  char *_TIMER_TOUT = NULL;

  AZX_LOG_TRACE("Reading parameters from file\r\n");

  AZX_LOG_TRACE("Opening %s in read mode..\r\n", CONFIG_FILE);

  fd = m2mb_fs_open(CONFIG_FILE,M2MB_O_RDONLY);

  if(fd != -1)
  {
    memset(recv, 0, sizeof(recv));
    fs_res = m2mb_fs_read(fd, recv, sizeof(recv));
    if(-1 == fs_res)
    {
      AZX_LOG_ERROR("cannot open config file!\r\n");
    }
    AZX_LOG_TRACE("Received %d bytes from file: \r\n<%.*s> \r\n", fs_res, fs_res, recv);

    AZX_LOG_TRACE("Closing file.\r\n");
    m2mb_fs_close(fd);

    //Using mystrtok function to separate #tag from the setting file

    //WAKE_UP_TICKS
    _WAKE_UP_TICKS = mystrtok(&p, recv,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //CTRL_TICKS_TO_REBOOT
    _CTRL_TICKS_TO_REBOOT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //WD_TOUT_COUNT
    _WD_TOUT_COUNT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    //TIMER_TOUT
    _TIMER_TOUT = mystrtok(&p, NULL,',');
    mystrtok(&p, NULL,'\n'); //strip description

    if( !_WAKE_UP_TICKS || !_CTRL_TICKS_TO_REBOOT || ! _WD_TOUT_COUNT || !_TIMER_TOUT )
    {
      AZX_LOG_CRITICAL("Cannot extract parameters from file!!\r\n");
      return -1;
    }
    else
    {
      gWAKE_UP_TICKS = atoi(_WAKE_UP_TICKS);
      gCTRL_TICKS_TO_REBOOT = atoi(_CTRL_TICKS_TO_REBOOT);
      gWD_TOUT_COUNT = atoi(_WD_TOUT_COUNT);
      gTIMER_TOUT = atoi(_TIMER_TOUT);

      AZX_LOG_INFO("Set WAKE_UP_TICKS to: %u\r\n", gWAKE_UP_TICKS);
      AZX_LOG_INFO("Set CTRL_TICKS_TO_REBOOT to: %u\r\n", gCTRL_TICKS_TO_REBOOT);
      AZX_LOG_INFO("Set WD_TOUT_COUNT to: %u\r\n", gWD_TOUT_COUNT);
      AZX_LOG_INFO("Set TIMER_TOUT to: %u\r\n", gTIMER_TOUT);
      return 1;
    }
  }
  else
  {
    return -1;
  }
}

void configureParameters(void)
{
  gWAKE_UP_TICKS = WAKE_UP_TICKS;
  gCTRL_TICKS_TO_REBOOT = CTRL_TICKS_TO_REBOOT;
  gWD_TOUT_COUNT = WD_TOUT_COUNT;
  gTIMER_TOUT = TIMER_TOUT;
}
