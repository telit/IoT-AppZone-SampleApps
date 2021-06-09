/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application prints some Module/SIM information as IMEI, fw version, IMSI and so on; it prints also some information about registration. Debug prints on USB0
  @version 
    1.0.0 
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
	Roberta Galeazzo

  @date
    31/03/2021
*/

/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>

#include "m2mb_types.h"
#include "m2mb_os_types.h"
#include "m2mb_os_ev.h"
#include "m2mb_os.h"
#include "m2mb_info.h"
#include "m2mb_net.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/
static M2MB_OS_EV_HANDLE net_evHandle = NULL;
M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
UINT32                  curEvBits;

M2MB_NET_STAT_E modStat;

#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/

const CHAR *status[]={"NOT REGISTERED","HOME","SEARCHING","REGISTRATION DENIED", "UNDEFINED", "ROAMING"};


/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

void NetCallback(M2MB_NET_HANDLE h, M2MB_NET_IND_E net_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
(void)resp_size;
(void)myUserdata;

M2MB_NET_REG_STATUS_T *stat_info;



	switch (net_event)
	{

		#if 0
		case M2MB_NET_REG_STATUS_IND:
		AZX_LOG_DEBUG("Network change event!\r\n");
		m2mb_net_get_reg_status_info(h);
		break;
		#endif

		case M2MB_NET_GET_REG_STATUS_INFO_RESP:
		{
			stat_info = (M2MB_NET_REG_STATUS_T*)resp_struct;
			modStat = stat_info->stat;
			if  (stat_info->stat == 1 || stat_info->stat == 5)
			{
				AZX_LOG_INFO("Module is registered to %s network cellID 0x%X\r\n", status[stat_info->stat],stat_info->cellID);
				m2mb_os_ev_set(net_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
			}
			else
			{
				//AZX_LOG_DEBUG("Module status is: %s\r\n", status[stat_info->stat]);
				m2mb_net_get_reg_status_info(h); //call it again
			}
		}
		break;

		case M2MB_NET_GET_CURRENT_OPERATOR_INFO_RESP:
		{
			M2MB_NET_GET_CURRENT_OPERATOR_INFO_RESP_T *resp = (M2MB_NET_GET_CURRENT_OPERATOR_INFO_RESP_T*)resp_struct;
			AZX_LOG_INFO("NETWORK OPERATOR (mcc mnc): %d %02d\r\n", resp->mcc, resp->mnc);
			m2mb_os_ev_set(net_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
		}
		break;

		case M2MB_NET_GET_SIGNAL_INFO_RESP:
		{
			M2MB_NET_GET_SIGNAL_INFO_RESP_T *resp = (M2MB_NET_GET_SIGNAL_INFO_RESP_T*)resp_struct;
			CHAR act[10];

			memset(act,0,sizeof(act));

			switch(resp->rat)
			{
			case M2MB_NET_RAT_GSM:
				strcpy(act,"2G");
				break;

			case M2MB_NET_RAT_EC_GSM_IoT:
				strcpy(act,"CatM");
				break;

			case M2MB_NET_RAT_EUTRAN_NB_S1:
				strcpy(act,"NBIoT");
				break;

			default:
				break;
			}

			AZX_LOG_INFO("Network Technology %s (AcT: %d) RSSI: %d\r\n", act, resp->rat, resp->rssi);

			if((resp->sigInfo != NULL) && ((resp->rat == M2MB_NET_RAT_EC_GSM_IoT) ||(resp->rat == M2MB_NET_RAT_EUTRAN_NB_S1)) )
			{
				M2MB_NET_SIGNAL_INFO_EUTRAN_T *tmpSigInfo = (M2MB_NET_SIGNAL_INFO_EUTRAN_T*)(resp->sigInfo);
				AZX_LOG_INFO("RSRQ: %d RSRP: %d SNR: %d\r\n", tmpSigInfo->rsrq, tmpSigInfo->rsrp, tmpSigInfo->snr);
			}

			m2mb_os_ev_set(net_evHandle, EV_NET_BIT, M2MB_OS_EV_SET);
		}
		break;

		default:
			AZX_LOG_DEBUG("unexpected net_event: %d\r\n", net_event);
		break;

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
	M2MB_RESULT_E retval = M2MB_RESULT_SUCCESS;
	M2MB_OS_RESULT_E osRes = M2MB_OS_SUCCESS;
	M2MB_INFO_HANDLE hInfo;
	CHAR *info;
	M2MB_NET_HANDLE hnet;

	void *myUserdata = NULL;
	

	AZX_LOG_INIT();
	AZX_LOG_INFO("\r\nStarting. This is v%s built on %s %s. LEVEL: %d\r\n",
			VERSION, __DATE__, __TIME__, azx_log_getLevel());


	azx_sleep_ms(5000);


	AZX_LOG_INFO( "\r\n\r\n Start General INFO application [ version: %f ] \r\n\r\n", 1.0 );

	osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "net_ev"));

	osRes = m2mb_os_ev_init( &net_evHandle, &evAttrHandle );
	if ( osRes != M2MB_OS_SUCCESS )
	{
		m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
		AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
		return;
	}
	else
	{
		AZX_LOG_TRACE("m2mb_os_ev_init success\r\n");
	}

	retval = m2mb_net_init(&hnet, NetCallback, myUserdata);
	if ( retval == M2MB_RESULT_SUCCESS )
	{
		AZX_LOG_TRACE( "m2mb_net_init returned M2MB_RESULT_SUCCESS\r\n");
	}
	else
	{
		AZX_LOG_ERROR( "m2mb_net_init did not return M2MB_RESULT_SUCCESS\r\n" );
	}




	retval = m2mb_info_init(&hInfo);
	if(retval != M2MB_RESULT_SUCCESS){

		AZX_LOG_ERROR( "can't init info apis\r\n");
		
	} else {

		AZX_LOG_TRACE( "m2mb_info_init successful\r\n");

		azx_sleep_ms(1000);
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_MODEL, &info);
		AZX_LOG_INFO( "\r\n======================\r\n");
		AZX_LOG_INFO( "\r\nMODULE %s INFO\r\n", info);
		AZX_LOG_INFO( "\r\n======================\r\n\r\n");
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_MANUFACTURER, &info);
		AZX_LOG_INFO( "MANUFACTURER: %s \r\n", info);
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_SERIAL_NUM, &info);
		AZX_LOG_INFO( "\r\nIMEI: %s \r\n", info);
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_FW_VERSION, &info);
		AZX_LOG_INFO( "\r\nMODEM FIRMWARE VERSION: %s\r\n", info );
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_SW_VERSION, &info);
		AZX_LOG_INFO( "\r\nPACKAGE VERSION: \r\n%s\r\n\r\n", info);
		AZX_LOG_INFO( "\r\n========\r\n");
		AZX_LOG_INFO( "\r\nSIM INFO\r\n");
		AZX_LOG_INFO( "\r\n========\r\n\r\n");
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_IMSI, &info);
		AZX_LOG_INFO( "IMSI: %s\r\n", info);
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_ICCID, &info);
		AZX_LOG_INFO( "\r\nICCID: %s \r\n", info );
		retval = m2mb_info_get(hInfo, M2MB_INFO_GET_MSISDN, &info);
		if(strcmp(info,"") != 0 ){
			AZX_LOG_INFO( "\r\nMSISDN: %s \r\n", info );
		}



	}
	AZX_LOG_INFO("\r\n\r\n===========================\r\n");
	AZX_LOG_INFO("\r\nWaiting for registration...\r\n");
	AZX_LOG_INFO("\r\n===========================\r\n\r\n");

	retval = m2mb_net_get_reg_status_info(hnet);
	if ( retval != M2MB_RESULT_SUCCESS )
	{
		AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
	}

	/*Wait for network registration event to occur (released in NetCallback function) */
	m2mb_os_ev_get(net_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_WAIT_FOREVER);



	if (modStat == 1 || modStat ==5)
	{
		retval = m2mb_net_get_current_operator_info(hnet);
		if ( retval != M2MB_RESULT_SUCCESS )
		{
			AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
		}

		/*Wait for network registration event to occur (released in NetCallback function) */
		m2mb_os_ev_get(net_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS( 5000 ));

		retval = m2mb_net_get_signal_info(hnet);
		if ( retval != M2MB_RESULT_SUCCESS )
		{
			AZX_LOG_ERROR( "m2mb_net_get_reg_status_info did not return M2MB_RESULT_SUCCESS\r\n" );
		}

		/*Wait for network registration event to occur (released in NetCallback function) */
		m2mb_os_ev_get(net_evHandle, EV_NET_BIT, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS( 5000 ));

	}

	azx_sleep_ms(2000);

	m2mb_net_deinit(hnet);

	m2mb_info_deinit(hInfo);

}

