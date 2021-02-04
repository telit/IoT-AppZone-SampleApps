/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to print on one of the available output interfaces. Debug prints on USB0
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    02/03/2017
*/
/* Include files ================================================================================*/
#include "m2mb_types.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

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
  
	AZX_LOG_CFG_T log_cfg;

	azx_sleep_ms(5000);

	/*Set log configuration */
	log_cfg.log_channel = LOG_CHANNEL; /* Defined in app_cfg.h */
	log_cfg.log_level = AZX_LOG_LEVEL_TRACE; /*Enable all logs*/
  log_cfg.log_colours = 0; /*Set to 1 to use coloured logs (not all terminals are compatible)*/

	azx_log_init(&log_cfg);

	//Print all levels with default macros
  AZX_LOG_INFO("Starting Logging demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

	AZX_LOG_WARN("This is a WARNING MESSAGE\r\n");

	AZX_LOG_ERROR("THIS IS AN ERROR MESSAGE\r\n");
  
  AZX_LOG_CRITICAL("THIS IS AN CRITICAL MESSAGE\r\n");

	AZX_LOG_DEBUG("This is a DEBUG message\r\n");

	AZX_LOG_TRACE("This is a TRACE message\r\n");

	AZX_LOG_INFO("END.\r\n");

	azx_log_deinit();

}

