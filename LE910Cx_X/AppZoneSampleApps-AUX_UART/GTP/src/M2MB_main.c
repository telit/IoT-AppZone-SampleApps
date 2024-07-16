/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application that shows hot to get the position using GTP feature. Debug prints on AUX UART
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
     Roberta Galeazzo

  @date
    2/93/2023
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
#include "m2mb_power.h"
#include "m2mb_net.h"
#include "m2mb_pdp.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "app_cfg.h"

/* Local defines ================================================================================*/
#define GPS_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define NET_BIT         (UINT32)0x2    /*0x0000000000000010*/

#define APN      "apn"
#define PDP_CTX   (UINT8)1

/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
static M2MB_OS_EV_HANDLE net_gps_evHandle = NULL;
M2MB_GNSS_HANDLE gtp_handle;
M2MB_PDP_HANDLE pdpHandle;
void *userdata = NULL;

UINT8 n;


static void checkNetStat(  M2MB_NET_REG_STATUS_T *stat_info)
{
  if  (stat_info->stat == 1 || stat_info->stat == 5)
  {
    AZX_LOG_DEBUG("Module is registered to cell 0x%X!\r\n", (unsigned int)stat_info->cellID);
    m2mb_os_ev_set(net_gps_evHandle, NET_BIT, M2MB_OS_EV_SET);
  }
  else
  {
    m2mb_os_ev_set(net_gps_evHandle, NET_BIT, M2MB_OS_EV_CLEAR);
  }
}

static void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
  UNUSED_3( h, resp_size, myUserdata);

  M2MB_NET_REG_STATUS_T *stat_info;

  switch (net_event)
  {
  case M2MB_NET_GET_REG_STATUS_INFO_RESP:
    stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
    checkNetStat(stat_info);
    break;

  case M2MB_NET_REG_STATUS_IND:
    stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
    AZX_LOG_DEBUG("Net Stat IND is %d, %d, %d, %d, %ld\r\n",
        stat_info->stat, stat_info->rat, stat_info->srvDomain,
        stat_info->areaCode, stat_info->cellID);
    checkNetStat(stat_info);
    break;

  default:
    AZX_LOG_TRACE("Unexpected net_event: %d\r\n", net_event);
    break;

  }
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

void printGTPInfo_test( M2MB_GNSS_GTP_INFO_T *locData )
{

  AZX_LOG_INFO("latitude: %Lf\r\n", locData->latitude );

  AZX_LOG_INFO("longitude: %Lf\r\n", locData->longitude );

  AZX_LOG_INFO("altitude: %Lf\r\n", locData->altitude );
  AZX_LOG_INFO("altitudeMeanSeaLevel: %Lf\r\n", locData->altitudeMeanSeaLevel );

  AZX_LOG_INFO("accuracy: %f\r\n", locData->accuracy );

  AZX_LOG_INFO("speed: %f\r\n", locData->speed );
  AZX_LOG_INFO("bearing: %f\r\n", locData->bearing );

  AZX_LOG_INFO("timestamp: %llu\r\n", locData->timestamp );  // milliseconds since Jan. 1, 1970

  AZX_LOG_INFO("verticalAccuracy %f\r\n", locData->verticalAccuracy );
  AZX_LOG_INFO("speedAccuracy %f\r\n", locData->speedAccuracy );
  AZX_LOG_INFO("bearingAccuracy: %f\r\n\r\n", locData->bearingAccuracy );

  return;
}

void gnssCallbackFN( M2MB_GNSS_HANDLE handle, M2MB_GNSS_IND_E event, UINT16 resp_size, void *resp, void *userdata )
{
  (void)handle;
  (void)resp_size;

  AZX_LOG_DEBUG("gnssCallback event: %d\r\n", event);
  switch (event)
  {

  case M2MB_GNSS_INDICATION_POSITION_REPORT:
  {

    memcpy(userdata,(M2MB_GNSS_GTP_INFO_T *)resp,sizeof(M2MB_GNSS_GTP_INFO_T));
    m2mb_os_ev_set(net_gps_evHandle,GPS_BIT, M2MB_OS_EV_SET);
  }
  break;

  case M2MB_GNSS_INDICATION_NMEA_REPORT:
  {
    //old fw versions
    memcpy(userdata,(M2MB_GNSS_GTP_INFO_T *)resp,sizeof(M2MB_GNSS_GTP_INFO_T));
    m2mb_os_ev_set(net_gps_evHandle,GPS_BIT, M2MB_OS_EV_SET);
  }
  break;

  default:
    AZX_LOG_WARN("unexpected event\r\n");
    break;
  }

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
  UINT8 status;
  M2MB_RESULT_E res;
  M2MB_POWER_HANDLE pwHandle;
  M2MB_NET_HANDLE hNet;

  M2MB_OS_RESULT_E        osRes;
  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  UINT32                  curEvBits;
  M2MB_GNSS_GTP_INFO_T gtpData;

  /*SET output channel */
  azx_sleep_ms(2000);
  AZX_LOG_INIT();

  AZX_LOG_INFO("\r\nStarting GTP demo app. This is v%s built on %s %s.\r\n\r\n",
      VERSION, __DATE__, __TIME__);
  osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "gps_ev"));
  osRes = m2mb_os_ev_init( &net_gps_evHandle, &evAttrHandle );

  if ( osRes != M2MB_OS_SUCCESS ){
    m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
    return;
  } else {

    AZX_LOG_TRACE("m2mb_os_ev_init success\r\n");

  }

  /*Check network registration*/
  res = m2mb_net_init(&hNet, NetCallback, userdata);
  if ( res == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_DEBUG( "m2mb_net_init returned M2MB_RESULT_SUCCESS\r\n");
  }
  else
  {
    AZX_LOG_ERROR( "m2mb_net_init did not return M2MB_RESULT_SUCCESS\r\n" );
  }

  res = m2mb_net_enable_ind(hNet, M2MB_NET_REG_STATUS_IND, 1);
  if ( res != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_net_enable_ind failed\r\n" );
    return;
  }

  AZX_LOG_DEBUG("Waiting for registration...\r\n");

  res = m2mb_net_get_reg_status_info(hNet);
  if ( res != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
  }


  /*Wait for network registration event to occur (released in NetCallback function) */
  m2mb_os_ev_get(net_gps_evHandle, NET_BIT, M2MB_OS_EV_GET_ANY, &curEvBits, M2MB_OS_WAIT_FOREVER);

  AZX_LOG_DEBUG("Pdp context setting\r\n");
  res = m2mb_pdp_init(&pdpHandle, NULL, NULL);
  if ( res == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_DEBUG( "m2mb_pdp_init returned M2MB_RESULT_SUCCESS\r\n");
  }
  else
  {
    AZX_LOG_DEBUG( "m2mb_pdp_init did not return M2MB_RESULT_SUCCESS\r\n" );
  }

  res = m2mb_pdp_APN_set( pdpHandle, PDP_CTX, (CHAR*)APN );
  res = m2mb_pdp_deinit(pdpHandle);
  if ( res == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_DEBUG( "m2mb_pdp_deinit returned M2MB_RESULT_SUCCESS\r\n");
  }
  else
  {
    AZX_LOG_DEBUG( "m2mb_pdp_deinit did not return M2MB_RESULT_SUCCESS\r\n" );
  }


  /*Create GPS/GTP handle*/
  if( M2MB_RESULT_SUCCESS != m2mb_gnss_init( &gtp_handle, gnssCallbackFN, &gtpData ) )
  {
    AZX_LOG_ERROR("m2mb_gnss_init, failed!\r\n");
    return;
  }

  azx_sleep_ms(1000);

  /*Check GTP status, if not enabled enable it and reboot*/
  AZX_LOG_INFO("\r\nCheck if GTP has been already enabled\r\n");
  m2mb_gnss_GetGTPstatus(gtp_handle,&status);
  if (status == 0)
  {
    AZX_LOG_DEBUG("GTP status: %d => not enabled. Enable it and reboot module\r\n", status);
    if (M2MB_RESULT_SUCCESS != m2mb_power_init (&pwHandle, NULL, NULL))
    {
      AZX_LOG_ERROR("m2mb_power_init, failed!\r\n");
      return;
    }

    res = m2mb_gnss_EnableGTP( gtp_handle, 1 );
    if ( res == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_INFO( "m2mb_gnss_EnableGTP succeeded\r\n");
      m2mb_power_reboot(pwHandle);
      m2mb_power_deinit(pwHandle);
    }
    else
    {
      AZX_LOG_ERROR( "m2mb_gnss_EnableGTP fail, res: %d\r\n", res );
      return;
    }
  }
  else
  {
    AZX_LOG_DEBUG("GTP status: %d => Enabled\r\n", status);
  }

  /*Get GTP position*/

  AZX_LOG_DEBUG("Get the position...\r\n");
  if( M2MB_RESULT_SUCCESS != m2mb_gnss_GTP( gtp_handle ) )
  {
    AZX_LOG_ERROR("Failed to get GTP position\r\n");
    return;
  }
  AZX_LOG_INFO("\r\nm2mb_gnss_GTP OK, waiting for position...\r\n");
  /*Wait for GPS fix/NMEA sentences event to occur (released in gnssCallbackFN function) */
  m2mb_os_ev_get(net_gps_evHandle, GPS_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);
  AZX_LOG_INFO("GTP position got\r\n");
  printGTPInfo_test( &gtpData );

  azx_sleep_ms(2000);
}

