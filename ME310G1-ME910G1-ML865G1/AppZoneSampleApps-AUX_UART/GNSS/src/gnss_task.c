/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showing how to use GNSS functionality. Debug prints on $OUTPUT
  @version
    1.0.3
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
    Roberta Galeazzo

  @date
    12/10/2021
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
#include "m2mb_info.h"


#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "gnss_task.h"
#include "app_cfg.h"

/* Local defines ================================================================================*/
#define GPS_BIT         (UINT32)0x1    /*0x0000000000000001*/

/*defined via CPPFLAGS by Makefile.in depending on module family (from prod_tool.in file)*/
#ifndef M2MB_WWAN_GNSS_SWITCH_AVAILABLE
  #define GNSS_MEX10G1 0
#else
  #define GNSS_MEX10G1 1
#endif
/* Local typedefs ===============================================================================*/

typedef enum{
	MEX10C1,
	MEX10G1,
	LE910CX,
	LE910CX_X,
	NONE
} MODULE_TYPE_E;

/* Local statics ================================================================================*/

void *userdata = NULL;

M2MB_GNSS_HANDLE handle1;
UINT8 priority;
UINT32 TBF;
UINT8 constellation;

const CHAR *gnssServ[] = {"POSITION", "NMEA sentences"};

M2MB_GNSS_SERVICE_E gnss_service;
MODULE_TYPE_E moduleType;
static M2MB_OS_EV_HANDLE gps_evHandle = NULL;
//static UINT32 sentenceNum = 0;

extern INT32 gpsTask;

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

MODULE_TYPE_E getModuleType(void)
{
M2MB_RESULT_E res;
M2MB_INFO_HANDLE hInfo;
CHAR *info;
MODULE_TYPE_E moduleType = NONE;

	res = m2mb_info_init(&hInfo);
	if (res != M2MB_RESULT_SUCCESS)
	{
		 AZX_LOG_ERROR("Impossible init info\r\n");
	}
    else
	{
		res = m2mb_info_get(hInfo, M2MB_INFO_GET_MODEL, &info);

		if (res != M2MB_RESULT_SUCCESS)
		{
			AZX_LOG_ERROR("Impossible to get model\r\n");
		}
		else
		{
			AZX_LOG_INFO("Model: %s\r\n", info);
			if (strstr(info, "G1") != NULL)
			{
				AZX_LOG_TRACE("Type: %d\r\n", MEX10G1);
				moduleType = MEX10G1;
			}
			else if (info[0] == 'M' && strstr(info, "C1"))
			{
				AZX_LOG_TRACE("type: %d\r\n", MEX10C1);
				moduleType = MEX10C1;
			}
			else if ( strstr(info, "LE910C"))
			{
				if (info[strlen(info) -1 ] == 'X')
				{
					AZX_LOG_TRACE("type: %d\r\n", LE910CX_X);
					moduleType = LE910CX_X;
				}
				else
				{
					AZX_LOG_TRACE("type: %d\r\n", LE910CX);
					moduleType = LE910CX;
				}
			}
		}
		m2mb_info_deinit(hInfo);
	}
	return moduleType;
}


/*-----------------------------------------------------------------------------------------------*/

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


/*-----------------------------------------------------------------------------------------------*/

void gnssCallbackFN( M2MB_GNSS_HANDLE handle, M2MB_GNSS_IND_E event, UINT16 resp_size, void *resp, void *userdata )
{
  (void)handle;
  (void)resp_size;
  (void)userdata;
  //INT32 retVal;

  //AZX_LOG_DEBUG("gnssCallback[%d]\r\n", sentenceNum);
  switch (event){

	  case M2MB_GNSS_INDICATION_POSITION_REPORT:
	  {
		  printGnssInfo_test( (M2MB_GNSS_POSITION_REPORT_INFO_T *)resp );
		  m2mb_os_ev_set(gps_evHandle,GPS_BIT, M2MB_OS_EV_SET);
	  }
	  break;

#if GNSS_MEX10G1
	  case M2MB_GNSS_INDICATION_NMEA_REPORT:
	  {
		  AZX_LOG_INFO("NMEA: %s\r\n", (CHAR*)resp);
		  m2mb_os_ev_set(gps_evHandle,GPS_BIT, M2MB_OS_EV_SET);
	  }
	  break;
#endif
	  default:
		  AZX_LOG_WARN("unexpected event\r\n");
		  break;
  }
  //sentenceNum++;

}

/*-----------------------------------------------------------------------------------------------*/
/***************************************************************************************************
   GPS_task handles GPS init, configuration, start and stop
 **************************************************************************************************/
INT32 GPS_task(INT32 type, INT32 param1, INT32 param2){

M2MB_OS_RESULT_E        osRes;
M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
UINT32                  curEvBits;
//INT32 retVal;
#if GNSS_MEX10G1
M2MB_RESULT_E res;
#endif
	(void)param1;
	(void)param2;


	switch(type){

		case START_GPS:
		{
			osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "gps_ev"));
			osRes = m2mb_os_ev_init( &gps_evHandle, &evAttrHandle );

			if ( osRes != M2MB_OS_SUCCESS ){
			  m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
			  AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
			  return -1;
			} else {

			  AZX_LOG_TRACE("m2mb_os_ev_init success\r\n");

			}
			if( M2MB_RESULT_SUCCESS != m2mb_gnss_init( &handle1, gnssCallbackFN, userdata ) )
			{
				AZX_LOG_ERROR("m2mb_gnss_enable NMEA REPORT, failed!\r\n");
				return -1;
			}

			moduleType = getModuleType();

			/*this part is available ONLY for MEX10G1 products*/
#if GNSS_MEX10G1
			if(moduleType == MEX10G1)
			{
				m2mb_gnss_getcfg(handle1, M2MB_GNSS_PRIORITY, &priority);

				AZX_LOG_INFO("Priority: %d\r\n", priority);

				m2mb_gnss_getcfg(handle1, M2MB_GNSS_TBF, &TBF);
				AZX_LOG_INFO("TBF: %d\r\n", TBF);
				m2mb_gnss_getcfg(handle1, M2MB_GNSS_CONSTELLATION, &constellation);
				AZX_LOG_INFO("constellation: %d\r\n", constellation);

				/*To start getting GPS position priority MUST be set to GNSS */
				res = m2mb_gnss_set_prio_runtime(handle1, GNSS_PRIORITY);
				if(res != M2MB_RESULT_SUCCESS){
					AZX_LOG_ERROR("Can't change the priority to GNSS_PRIORITY!\r\n");
				} else {
					AZX_LOG_INFO("Priority changed to GNSS_PRIORITY, start GPS\r\n");
				}

			}
#endif
			/*
			 * on MEX10G1 both M2MB_GNSS_SERVICE_NMEA_REPORT and M2MB_GNSS_SERVICE_POSITION_REPORT services are available, while
			 * on ME910C1 product family only M2MB_GNSS_SERVICE_POSITION_REPORT is available
			 */

			if(moduleType == MEX10G1){
				//gnss_service = M2MB_GNSS_SERVICE_NMEA_REPORT;   /*On MEx10G1 family NMEA report sentences can be enabled as well*/
				gnss_service = M2MB_GNSS_SERVICE_POSITION_REPORT;
			} else {
				gnss_service = M2MB_GNSS_SERVICE_POSITION_REPORT;
			}


			if( M2MB_RESULT_SUCCESS != m2mb_gnss_enable( handle1, gnss_service) )
			{
				AZX_LOG_ERROR("m2mb_gnss_enable %s REPORT, failed!\r\n", gnssServ[gnss_service]);
				return -1;
			}

			AZX_LOG_INFO("\r\nm2mb_gnss_enable, %s OK\r\n",gnssServ[gnss_service]);


			/*
			* System will start to execute the callback function.
			*/


			if( M2MB_RESULT_SUCCESS != m2mb_gnss_start( handle1 ) )
			{
				AZX_LOG_ERROR("Failed to start GPS\r\n");
				return -1;
			}
			AZX_LOG_INFO("\r\nm2mb_gnss_start OK, waiting for position/nmea sentences...\r\n");
			/*Wait for GPS fix/NMEA sentences event to occur (released in gnssCallbackFN function) */
			m2mb_os_ev_get(gps_evHandle, GPS_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);

			AZX_LOG_INFO("\r\n***** Wait 120 seconds and then stop GPS *****\r\n\r\n");

			azx_sleep_ms(120000);

			azx_tasks_sendMessageToTask(gpsTask, STOP_GPS, 0, 0 );

		}
		break;

		case STOP_GPS:
		{
			AZX_LOG_INFO("***** 120 seconds expired *****\r\n\r\n");

			if( M2MB_RESULT_SUCCESS != m2mb_gnss_stop( handle1 ) )
			{
				AZX_LOG_ERROR("m2mb_gnss_stop fail\r\n");
				return -1;
			}
			AZX_LOG_INFO("m2mb_gnss_stop OK\r\n");

			if( M2MB_RESULT_SUCCESS != m2mb_gnss_disable( handle1, gnss_service ) )
			{
				AZX_LOG_ERROR("m2mb_gnss_disable fail\r\n");
				return -1;
			}
			AZX_LOG_INFO("m2mb_gnss_disable OK\r\n");

#if GNSS_MEX10G1
			/*Restore priority to WWAN */
			if(moduleType == MEX10G1){
				res = m2mb_gnss_set_prio_runtime(handle1, WWAN_PRIORITY);
				if(res != M2MB_RESULT_SUCCESS){
					AZX_LOG_ERROR("Can't change the priority to WWAN_PRIORITY!\r\n");
				} else {
					AZX_LOG_INFO("Priority changed to WWAN_PRIORITY, stop GPS\r\n");
				}
			}
#endif
			if( M2MB_RESULT_SUCCESS != m2mb_gnss_deinit( handle1 ) )
			{
				AZX_LOG_ERROR("m2mb_gnss_deinit fail\r\n");
				return -1;
			}
			AZX_LOG_INFO("m2mb_gnss_deinit OK\r\n");
		}
		break;

		default:
		break;

	}

	return 0;


}
