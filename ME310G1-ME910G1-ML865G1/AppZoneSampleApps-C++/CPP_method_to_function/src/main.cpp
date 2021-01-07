/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    main.cpp

  @brief
    The file contains the main user entry point of Appzone C++ code

  @details
    This file contains the C++ application logic

  @note
    User code entry is in function cpp_main, called by default M2MB_main()

  @author
    
  @date
    09/07/2020
*/
/* Include files ================================================================================*/
#include "m2mb_types.h"
#include "m2mb_hwTmr.h"

#include "azx_log.h"
#include "azx_utils.h"


#include "hwtimer_class.h"

using namespace HwTimerClass;


/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   Entry Point of Appzone

   \param [in] Module Id

   \details
**************************************************************************************************/


/*-----------------------------------------------------------------------------------------------*/
/**
  \brief Main entrance function of the C++ code.
  \details This is the function that will execute the C++ code of the application.

  \param [in] argc: number of application input parameters (passed by M2MB_Main())
  \param [in] argv: array of application input parameters (passed by M2MB_Main())
  \return user defined.

 */
/*-----------------------------------------------------------------------------------------------*/
int cpp_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;
  AZX_LOG_CFG_T cfg =
  {
    /*.log_level*/   AZX_LOG_LEVEL_DEBUG,
    /*.log_channel*/ AZX_LOG_TO_MAIN_UART,
    /*.log_colours*/ 0
  };
  
  azx_sleep_ms(5000);


  /*SET output channel */
  
  
  azx_log_init(&cfg);\
  
  AZX_LOG_INFO("Starting C++ method as function pointer example. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);

  HwTimer timer1 = HwTimer(1000, "first");
  HwTimer timer2 = HwTimer(1500, "second");

  timer1.start();
  timer2.start();


  azx_sleep_ms(10000);

  return 0;
}

