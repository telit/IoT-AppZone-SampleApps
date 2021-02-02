/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*
 * app_utils.c
 *
 */

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_ati.h"
#include "m2mb_fs_posix.h" //for file stats
#include "m2mb_fs_stdio.h"


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h" /*FOR LOCALPATH define*/

/* Macro =============================================================================*/
#define AT_INSTANCE 2

#define APP_RESTART_CMD "AT+M2M=4,10\r"
/* Local defines ======================================================================*/

/* Local statics ======================================================================*/
static char AT_BUF[160];
static char AT_RSP[160];
static char filePath[160];
/* Local function prototypes ===========================================================*/

/* Static functions ====================================================================*/


/* Global functions =============================================================================*/
INT32 update_app(const CHAR* new_app, const CHAR* old_app)
{
  M2MB_RESULT_E retVal;
  M2MB_ATI_HANDLE hATI;
  SSIZE_T res;

  INT32 ret = 0;
  int timeout = 10000; /*10 seconds*/

  retVal = m2mb_ati_init( &hATI, AT_INSTANCE, NULL, NULL );
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    do
    {
      memset(AT_BUF,0, sizeof(AT_BUF));
      memset(AT_RSP,0, sizeof(AT_RSP));
      sprintf(AT_BUF, "AT#M2MRUN=2,\"%s\"\r", new_app);
      retVal = m2mb_ati_send_cmd(hATI, AT_BUF, strlen(AT_BUF));
      if ( retVal != M2MB_RESULT_SUCCESS )
      {
        ret = -1;
        break;
      }
      while(1)
      {
        res = m2mb_ati_rcv_resp(hATI, AT_RSP + strlen(AT_RSP), sizeof(AT_RSP) - strlen(AT_RSP) );
        if ( res < 0 )
        {
          AZX_LOG_ERROR("failed receiving from AT\r\n");
          ret = -1;
          break;
        }
        if (strstr(AT_RSP, "OK"))
        {
          AZX_LOG_DEBUG("Application successfully configured.\r\n");

          if ((old_app != NULL) && (strcmp (old_app, "") != 0) )
          {
            memset(filePath,0, sizeof(filePath));
            sprintf(filePath, LOCALPATH "%s", old_app);
            AZX_LOG_DEBUG("Deleting old application %s\r\n", filePath);
            m2mb_fs_unlink(filePath);
          }

          m2mb_ati_send_cmd(hATI, (char*) APP_RESTART_CMD, strlen(APP_RESTART_CMD));
          azx_sleep_ms(2000);
          break;

        }
        if (strstr(AT_RSP, "ERROR"))
        {
          AZX_LOG_ERROR("ERROR received from AT response\r\n");
          ret = -1;
          break;
        }

        azx_sleep_ms(200);
        timeout -=200;

        if (timeout <= 0)
        {
          ret = -1;
          break;
        }
      }
    } while(0);

    m2mb_ati_deinit(hATI);

  }
  else
  {
    ret = -1;
  }

  return ret;
}
