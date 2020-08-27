/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showing how to use GNSS functionality. Debug prints on MAIN UART
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    23/10/2019
 */
/* Include files ================================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_os.h"
#include "m2mb_os_sem.h"
#include "m2mb_gnss.h"


#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/



void sleep_ms(UINT32 ms)
{
  m2mb_os_taskSleep( M2MB_OS_MS2TICKS(ms) );
}


void printGnssInfo_test( M2MB_GNSS_POSITION_REPORT_INFO_T *locData )
{
  locData->speed.speed = sqrt( pow( locData->speed.speed_horizontal,2 ) + pow( locData->speed.speed_vertical,2 ) );

  AZX_LOG_INFO("latitude_valid: %d - ", locData->latitude_valid );
  AZX_LOG_INFO("latitude: %f\r\n", locData->latitude );

  AZX_LOG_INFO("longitude_valid: %d - ", locData->longitude_valid );
  AZX_LOG_INFO("longitude: %f\r\n", locData->longitude );

  AZX_LOG_INFO("altitude_valid: %d - ", locData->altitude_valid );
  AZX_LOG_INFO("altitude: %f\r\n", locData->altitude );

  AZX_LOG_INFO("uncertainty_valid: %d - ", locData->uncertainty_valid );
  AZX_LOG_INFO("uncertainty: %f\r\n", locData->uncertainty );

  AZX_LOG_INFO("velocity_valid: %d - ", locData->velocity_valid );
  AZX_LOG_INFO("codingType: %d\r\n", locData->velocity.codingType );
  AZX_LOG_INFO("speed_horizontal: %f\r\n", locData->velocity.speed_horizontal );
  AZX_LOG_INFO("bearing: %f\r\n", locData->velocity.bearing );

  AZX_LOG_INFO("timestamp_valid: %d -", locData->timestamp_valid );
  AZX_LOG_INFO("timestamp: %llu\r\n", locData->timestamp );  // milliseconds since Jan. 1, 1970

  AZX_LOG_INFO("speed_valid: %d - ", locData->speed_valid );
  AZX_LOG_INFO("speed: %f\r\n\r\n", locData->speed.speed );

  return;
}

void gnssCallbackFN( M2MB_GNSS_HANDLE handle, M2MB_GNSS_IND_E event, UINT16 resp_size, void *resp, void *userdata )
{
  (void)handle;
  (void)resp_size;
  (void)userdata;
  
  if(event == M2MB_GNSS_INDICATION_POSITION_REPORT)
  {
    printGnssInfo_test( (M2MB_GNSS_POSITION_REPORT_INFO_T *)resp );
  }
  else
  {
    AZX_LOG_INFO("gnssCallback_test => event FAIL\r\n");
  }
}




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
  
  void *userdata = NULL;

  M2MB_GNSS_HANDLE handle1;

  azx_sleep_ms(2000);

  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting GNSS demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);


  if( M2MB_RESULT_SUCCESS != m2mb_gnss_init( &handle1, gnssCallbackFN, userdata ) )
  {
    return;
  }


  if( M2MB_RESULT_SUCCESS != m2mb_gnss_enable( handle1, M2MB_GNSS_SERVICE_POSITION_REPORT ) )
  {
    return;
  }

  AZX_LOG_INFO("m2mb_gnss_enable OK\r\n");



  if( M2MB_RESULT_SUCCESS != m2mb_gnss_start( handle1 ) )
  {
    return;
  }

  AZX_LOG_INFO("m2mb_gnss_start OK\r\n");


  /*
   * System will start to execute the callback function.
   */

  azx_sleep_ms(120000);


  if( M2MB_RESULT_SUCCESS != m2mb_gnss_stop( handle1 ) )
  {
    return;
  }
  AZX_LOG_INFO("m2mb_gnss_stop OK\r\n");

  if( M2MB_RESULT_SUCCESS != m2mb_gnss_disable( handle1, M2MB_GNSS_SERVICE_POSITION_REPORT ) )
  {
    return;
  }
  AZX_LOG_INFO("m2mb_gnss_disable OK\r\n");

  if( M2MB_RESULT_SUCCESS != m2mb_gnss_deinit( handle1 ) )
  {
    return;
  }
  AZX_LOG_INFO("m2mb_gnss_deinit OK\r\n");


}

