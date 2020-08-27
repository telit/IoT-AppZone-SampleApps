/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showcasing how to setup and use multiple events to create a barrier. Debug prints on USB0
    
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    12/11/2019
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"
#include "m2mb_hwTmr.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"


/* Local defines ================================================================================*/
/* Each event handler can manage up to 32 events (1 bit each in the bitmask) */
#define MY_EVENT1_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define MY_EVENT2_BIT         (UINT32)0x2    /*0x0000000000000010*/

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

static M2MB_OS_EV_HANDLE my_evHandle   = NULL;

/* Local function prototypes ====================================================================*/
static void hwTimerCb1( M2MB_HWTMR_HANDLE hwTmrHandle, void *argCb );
static void hwTimerCb2( M2MB_HWTMR_HANDLE hwTmrHandle, void *argCb );

/* Static functions =============================================================================*/

static void hwTimerCb1( M2MB_HWTMR_HANDLE hwTmrHandle, void *argCb )
{
  (void)hwTmrHandle;
  (void)argCb;

  
  AZX_LOG_DEBUG("Timer Callback, generate event %d!\r\n", MY_EVENT1_BIT);
  m2mb_os_ev_set(my_evHandle, MY_EVENT1_BIT, M2MB_OS_EV_SET);
}

static void hwTimerCb2( M2MB_HWTMR_HANDLE hwTmrHandle, void *argCb )
{
  (void)hwTmrHandle;
  (void)argCb;
  
  AZX_LOG_DEBUG("Timer Callback, generate event %d!\r\n", MY_EVENT2_BIT);
  m2mb_os_ev_set(my_evHandle, MY_EVENT2_BIT, M2MB_OS_EV_SET);
}

/*Create an hw timer given the handler to be initialized, the timer timeout and the
 * callback to execute at expiration*/
int setup_timer( M2MB_HWTMR_HANDLE *tmrHwHandle, UINT32 timeout, USR_HWTMR_CB *cb)
{
  M2MB_HWTMR_RESULT_E        hwRes;
  M2MB_HWTMR_ATTR_HANDLE     tmrHwAttrHandle;
  UINT32                     timeDuration = timeout * 1000; //time unit is usecs
  MEM_W                      argCb = 0;

  /* Create timer attributes */
  if( (hwRes = m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle, 1, M2MB_HWTMR_SEL_CMD_CREATE_ATTR, NULL )) != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Create the timer attributes, error %d\r\n", hwRes);
    return -1;

  }

  /* Set timer attributes structure */

  hwRes = m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle,
      CMDS_ARGS(
          M2MB_HWTMR_SEL_CMD_CB_FUNC, cb,
          M2MB_HWTMR_SEL_CMD_ARG_CB, argCb,
          M2MB_HWTMR_SEL_CMD_TIME_DURATION, timeDuration, //set timer timeout
          M2MB_HWTMR_SEL_CMD_PERIODIC, M2MB_HWTMR_ONESHOT_TMR, //timer is oneshot, will run only once
          M2MB_HWTMR_SEL_CMD_AUTOSTART, M2MB_HWTMR_NOT_START //do not start automatically
      )
  );

  if( hwRes != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Create the timer attributes, error %d\r\n", hwRes);
    return -1;
  }
  else
  {
    AZX_LOG_INFO( "Set the timer attributes structure success.\r\n" );
  }

  /* Define the timer and pass the parameters structure to it. */
  hwRes = m2mb_hwTmr_init( tmrHwHandle, &tmrHwAttrHandle );
  if( hwRes != M2MB_HW_SUCCESS )
  {
    /* delete the parameters structure */
    m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle, 1, M2MB_HWTMR_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_ERROR("Delete timer attributes, error %d\r\n", hwRes);
    return hwRes;
  }
  else
  {
    AZX_LOG_INFO("Timer successfully created with %u timeout (ms) \r\n", timeout );
  }
  return 0;
}

/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;

  M2MB_HWTMR_HANDLE tmrHwHandle1, tmrHwHandle2;

  azx_sleep_ms(5000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting Barrier demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  /* Init events handler */
  osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, 
              CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, 
                        M2MB_OS_EV_SEL_CMD_NAME, "my_ev"
                        )
          );
  osRes = m2mb_os_ev_init( &my_evHandle, &evAttrHandle );

  if ( osRes != M2MB_OS_SUCCESS )
  {
    m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
    return;
  }
  else
  {
    AZX_LOG_DEBUG("m2mb_os_ev_init success\r\n");
  }

  /*First, create the two timers*/
  if(0 != setup_timer(&tmrHwHandle1, 3000, (USR_HWTMR_CB *) &hwTimerCb1))
  {
    return;
  }

  if(0 != setup_timer(&tmrHwHandle2, 6000, (USR_HWTMR_CB *) &hwTimerCb2))
  {
    return;
  }

  /* Then, start the timers */
  if( m2mb_hwTmr_start( tmrHwHandle1 ) != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Cannot start timer 1!\r\n");
    return;
  }

  if( m2mb_hwTmr_start( tmrHwHandle2 ) != M2MB_HW_SUCCESS )
  {
    AZX_LOG_ERROR("Cannot start timer 1!\r\n");
    return;
  }

  
  /*Lastly, wait for both events generated by timers to procede further*/
  m2mb_os_ev_get(my_evHandle, MY_EVENT1_BIT | MY_EVENT2_BIT, M2MB_OS_EV_GET_ALL_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
  /* Waiting for the event to be generated in the hw timer callback function, with EVENT_MYEV1_BIT + EVENT_MYEV2_BIT bitmask.
   * - To react only after ALL events in a mask occur, use M2MB_OS_EV_GET_ALL_AND_CLEAR option.
   * - To react to any of the set bits, use M2MB_OS_EV_GET_ANY_AND CLEAR
   * */
  
  AZX_LOG_DEBUG("BOTH events occurred!\r\n");
}
