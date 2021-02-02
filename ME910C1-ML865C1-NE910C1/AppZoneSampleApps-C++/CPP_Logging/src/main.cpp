/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    main.cpp

  @brief
    The file contains the main user entry point of Appzone C++ code

  @details
    This file contains the C++ application usage

  @note
    User code entry is in function cpp_main, called by default M2MB_main()

  @author
    FabioPi

  @date
    11/09/2019
*/
/* Include files ================================================================================*/
#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "Log.h"

using namespace M2MLog;


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
  
  Logger log{LOG_LEVEL_TRACE, LOG_TO_MAIN_UART, false};

  log.trace(   "C++ Trace print example\r\n");
  log.debug(   "C++ Debug print example\r\n");
  log.info(    "C++ Info print example\r\n");
  log.warn(    "C++ Warning print example\r\n");
  log.error(   "C++ Error print example\r\n");
  log.critical("C++ Critical print example\r\n");
  return 0;
}

