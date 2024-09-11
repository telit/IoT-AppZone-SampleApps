/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to protect an AT command with a pwd using EasyAT functionality. Debug prints on AUX UART
  @version 
    1.0.0
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Roberta Galeazzo

  @date
    13/08/2024
*/

/* Include files ================================================================================*/
#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_atp.h"
#include "azx_log.h"
#include "azx_utils.h"
#include "app_cfg.h"
#include "at_common.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

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
  azx_sleep_ms( 4000 );

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting Easy AT admin demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  if( my_cmds_at_init() != 0 )
  {
    AZX_LOG_CRITICAL( "Failed mycmd init\r\n" );
  }
  azx_sleep_ms( 2000 );
}

