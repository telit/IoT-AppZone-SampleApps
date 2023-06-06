/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing how use lfs2 porting with RAM disk and SPI data flash. Debug prints on USB0
  @version
    1.0.3
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    21/10/2020
*/

/* Include files ================================================================================*/
#include "m2mb_types.h"

#include "azx_utils.h"
#include "azx_log.h"
#include "app_cfg.h"

#include "lfs_usage.h"


/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

/* Local function prototypes ====================================================================*/
//	ramDiskDemo();
//	flashDiskDemo();

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


  azx_sleep_ms(4000);


  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting lfs2 demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);


  ramDiskDemo();
  
  azx_sleep_ms(4000);

  flashDiskDemo();

}

