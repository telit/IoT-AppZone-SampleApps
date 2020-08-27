/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    The application prints "Hello World!" over selected output every two seconds. Debug prints on AUX UART, <ins>using AZX log example functions</ins>
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
	
	AZX_LOG_INIT();
	AZX_LOG_INFO("Starting. This is v%s built on %s %s. LEVEL: %d\r\n",
			VERSION, __DATE__, __TIME__, azx_log_getLevel());
	UINT32 counter = 0;

	azx_sleep_ms(5000);


	AZX_LOG_INFO( "\r\n Start Hello world Application  [ version: %f ] \r\n", 2.0 );

	while (1)
	{
		counter++;
		if ( counter > 999999 )
		{
			AZX_LOG_INFO( "\r\n Counter zeroed " );
		  counter = 0;
		}

		AZX_LOG_INFO( "\r\n Hello world 2.0 [ %06d ] ", counter );
		azx_sleep_ms(2000);

	}



}

