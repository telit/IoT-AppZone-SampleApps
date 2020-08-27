/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    The sample application shows how to use HW Timers M2MB API. Debug prints on AUX UART
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    24/10/2019
*/
/* Include files ================================================================================*/
#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_hwTmr.h"

#include <stdio.h>
#include <string.h>

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

/* Local function prototypes ====================================================================*/

/*-----------------------------------------------------------------------------------------------*/
/* Static functions =============================================================================*/
static void TimerCb( M2MB_HWTMR_HANDLE hwTmrHandle, void *argCb )
{
  (void)hwTmrHandle;
  (void)argCb;
  static unsigned int Count = 0;

  AZX_LOG_DEBUG("Callback Count: [%u]\r\n", Count++);
}


/* Global functions =============================================================================*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
**************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;
  
  
  
  M2MB_HWTMR_RESULT_E        hwRes;
  M2MB_HWTMR_ATTR_HANDLE     tmrHwAttrHandle;
  M2MB_HWTMR_HANDLE          hwTmrHandle;
  
  MEM_W                      argCb = 0;


  azx_sleep_ms(2000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting HW Timers demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  //Create timer attributes
  if((hwRes = m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle, 1, M2MB_HWTMR_SEL_CMD_CREATE_ATTR, NULL )) != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Create the timer attributes, error %d\r\n", hwRes);
    return;
  }

  //set timer attributes structure
  hwRes = m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle,
                 CMDS_ARGS(
                   M2MB_HWTMR_SEL_CMD_CB_FUNC, &TimerCb,
                   M2MB_HWTMR_SEL_CMD_ARG_CB, argCb,
                   M2MB_HWTMR_SEL_CMD_TIME_DURATION, 100000, //set timer to fire every 100 mSecs (time unit is usecs)
                   M2MB_HWTMR_SEL_CMD_PERIODIC, M2MB_HWTMR_PERIODIC_TMR, //timer is periodic, will auto fire each time it expires
                   M2MB_HWTMR_SEL_CMD_AUTOSTART, M2MB_HWTMR_AUTOSTART
                    )
                 );

  if( hwRes != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Create the timer attributes, error %d\r\n", hwRes);
    return;
  }
  else
  {
    AZX_LOG_DEBUG( "Set the timer attributes structure: success.\r\n" );
  }

  //Define the timer and pass the parameters structure to it.
  hwRes = m2mb_hwTmr_init( &hwTmrHandle, &tmrHwAttrHandle );

  if( hwRes != M2MB_HW_SUCCESS )
  {
    //delete the parameters structure
    m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle, 1, M2MB_HWTMR_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_ERROR("Delete timer attributes, error %d\r\n", hwRes);
  }
  else
  {
    AZX_LOG_INFO("Timer successfully created\r\n" );
  }

  //Start the timer
  hwRes = m2mb_hwTmr_start( hwTmrHandle );

  if( hwRes != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Start the timer, error %d\r\n", hwRes);
  }
  else
  {
    AZX_LOG_INFO("Start the timer, success.\r\n" );
  }

  azx_sleep_ms(10000);

  //Stop a running the timer
  hwRes = m2mb_hwTmr_stop( hwTmrHandle );
  if( hwRes != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Cannot stop running timer! Res: %d\r\n", hwRes);
  }
  else
  {
    AZX_LOG_INFO("Stop a running timer: success\r\n" );
  }
  hwRes = m2mb_hwTmr_deinit( hwTmrHandle);
  AZX_LOG_INFO("Application end\r\n");
  
}

