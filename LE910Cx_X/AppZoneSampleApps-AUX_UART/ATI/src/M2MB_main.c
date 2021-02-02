/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing how to use AT Instance functionality (sending AT commands from code). The example supports both sync and async (using a callback) modes. Debug prints on AUX UART
  @version 
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author

  @date
    18/07/2019
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_ati.h"

#include "at_utils.h"

#include "azx_log.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

INT16 instanceID = 0; /*AT0, bound to UART by default config*/

/*Set sync to 1 to use sync functions, to 0 to use async functions (with callback)*/
int sync = 0;


/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
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
  
  CHAR rsp[100];

  M2MB_RESULT_E retVal;

  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(2000) );


  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting AT demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);
  

  if(sync)
  {
    retVal = at_cmd_sync_init(instanceID);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "at_cmd_sync_init() returned success value\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "at_cmd_sync_init() returned failure value\r\n" );
      return;
    }


    //Sending command
    AZX_LOG_INFO("Sending command AT+CGMR in sync mode\r\n");
    retVal = send_sync_at_command(instanceID, "AT+CGMR\r", rsp, sizeof(rsp));
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "Error sending command AT+CGMR\r\n" );
    }
    else
    {
      AZX_LOG_INFO("Command response: <%s>\r\n\r\n", rsp);
    }


    retVal = at_cmd_sync_deinit(instanceID);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "at_cmd_sync_deinit() returned success value\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "at_cmd_sync_deinit() returned failure value\r\n" );
      return;
    }
  }
  else /*async*/
  {
    retVal = at_cmd_async_init(instanceID);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "at_cmd_async_init() returned success value\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "at_cmd_async_init() returned failure value\r\n" );
      return;
    }

    //Sending command +CGMR
    AZX_LOG_INFO("Sending command AT+CGMR in async mode\r\n");
    retVal = send_async_at_command(instanceID, "AT+CGMR\r", rsp, sizeof(rsp));
    if ( retVal != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR( "Error sending command AT+CGMR\r\n" );
    }
    else
    {
      AZX_LOG_INFO("Command response: <%s>\r\n\r\n", rsp);
    }


    retVal = at_cmd_async_deinit(instanceID);
    if ( retVal == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_TRACE( "at_cmd_async_deinit() returned success value\r\n" );
    }
    else
    {
      AZX_LOG_ERROR( "at_cmd_async_deinit() returned failure value\r\n" );
      return;
    }
  }

  AZX_LOG_INFO("Application end\r\n");
}

