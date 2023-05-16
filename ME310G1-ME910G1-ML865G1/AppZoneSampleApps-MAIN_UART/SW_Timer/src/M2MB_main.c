/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    The sample application shows how to use SW Timers M2MB API. Debug prints on MAIN UART
  @version 
    1.0.2
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

#include "m2mb_os_tmr.h"

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
void timerCb( M2MB_OS_TMR_HANDLE tmrHandle, void *ctx)
{
  (void)tmrHandle;
  
  M2MB_OS_TMR_HANDLE handle;

  AZX_LOG_INFO("timer expired!\r\n");
  
  /* here in this case ctx is address of  M2MB_OS_TMR_HANDLE tmrHandle, so it is a (M2MB_OS_TMR_HANDLE *) */
  handle = *((M2MB_OS_TMR_HANDLE *)ctx);

  AZX_LOG_DEBUG("timer handle: %p\r\n", handle);

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

  M2MB_OS_TMR_ATTR_HANDLE tmrAttrHandle;
  M2MB_OS_TMR_HANDLE      tmrHandle;
  M2MB_OS_RESULT_E        osRes;


  azx_sleep_ms(2000);

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting SW Timers demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  /* Create the attribute structure which will hold the timer attributes */
  if ( m2mb_os_tmr_setAttrItem( &tmrAttrHandle, 1, M2MB_OS_TMR_SEL_CMD_CREATE_ATTR, NULL ) != M2MB_OS_SUCCESS )
  {
    AZX_LOG_ERROR("error_creating attribute timer\r\n");
    return;
  }
  
  /* set the attributes in parameters structure */
  osRes = m2mb_os_tmr_setAttrItem( &tmrAttrHandle,
       CMDS_ARGS(
           M2MB_OS_TMR_SEL_CMD_NAME, "mytmr",  /*optional timer name */
           M2MB_OS_TMR_SEL_CMD_CB_FUNC, &timerCb, /*the callback function to run */
           M2MB_OS_TMR_SEL_CMD_ARG_CB, &tmrHandle, /*arg for callback function, here handle of timer for example*/
           M2MB_OS_TMR_SEL_CMD_TICKS_PERIOD, M2MB_OS_MS2TICKS( 4000 ),  /*wait 4 seconds */
           M2MB_OS_TMR_SEL_CMD_PERIODIC, M2MB_OS_TMR_PERIODIC_TMR  /*set the timer as periodic (auto restarts after expiration)*/
         )
      );

  if ( osRes != M2MB_OS_SUCCESS )
  {
    /* delete the attributes structure in case of failure*/
    m2mb_os_tmr_setAttrItem( &tmrAttrHandle, 1, M2MB_OS_TMR_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_ERROR("error setting or creating tmrAttrHandle\r\n");
    return;
  }


  /* Now, init timer passing the attributes structure */
  if ( m2mb_os_tmr_init( &tmrHandle, &tmrAttrHandle ) != M2MB_OS_SUCCESS )
  {
    /* in case of error, manually remove attributes structure and release resources (in case of success, it will be managed by the task deinit function*/
    m2mb_os_tmr_setAttrItem( &tmrAttrHandle, 1, M2MB_OS_TMR_SEL_CMD_DEL_ATTR, NULL );
  }
  
   /*Now timer can be started. Once expired, the callback function will be executed*/
  m2mb_os_tmr_start(tmrHandle);

  azx_sleep_ms(10000);

  
  AZX_LOG_INFO("Stopping the timer\r\n");
  osRes = m2mb_os_tmr_stop(tmrHandle);
  if( osRes != M2MB_OS_SUCCESS )
  {
    AZX_LOG_ERROR("Cannot stop running timer! Res: %d\r\n", osRes);
  }
  else
  {
    AZX_LOG_INFO("Stop a running timer: success\r\n" );
  }
  
  osRes = m2mb_os_tmr_deinit( tmrHandle);

  AZX_LOG_INFO("Application end\r\n");
  
}

