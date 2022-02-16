/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    lwm2m_demo.h

  @brief
    demo related definitions

  @details
    

  @note
    Dependencies:
    m2mb_types.h
    m2mb_lwm2m.h

  @author
    

  @date
    21/01/2022
*/


#ifndef HDR_LWM2M_DEMO_H_
#define HDR_LWM2M_DEMO_H_


/* Global declarations ==========================================================================*/


/*LWM2M event bits*/
#define EV_LWM2M_ENABLE_RES_BIT         (UINT32)0x00000001
#define EV_LWM2M_SET_RES_BIT            (UINT32)0x00000002
#define EV_LWM2M_GET_RES_BIT            (UINT32)0x00000004
#define EV_LWM2M_MON_RES_BIT            (UINT32)0x00000008
#define EV_LWM2M_WRITE_RES_BIT          (UINT32)0x00000010
#define EV_LWM2M_READ_RES_BIT           (UINT32)0x00000020
#define EV_LWM2M_SRV_REG_BIT            (UINT32)0x00000040
#define EV_LWM2M_GET_STAT_RES_BIT       (UINT32)0x00000080

#define EV_LWM2M_GET_LIST_RES_BIT       (UINT32)0x00000100
#define EV_LWM2M_SRV_DISABLED_BIT       (UINT32)0x00000200
#define EV_LWM2M_EXIST_RES_BIT          (UINT32)0x00000400

#define EV_LWM2M_NFYADD_RES_BIT         (UINT32)0x00001000
#define EV_LWM2M_NFYACK_BIT             (UINT32)0x00002000
#define EV_LWM2M_NFYACK_LIST_RES_BIT    (UINT32)0x00004000
#define EV_LWM2M_NFYSTAT_RES_BIT        (UINT32)0x00008000

#define EV_LWM2M_FOTA_CFG_RES_BIT       (UINT32)0x00100000
#define EV_LWM2M_FOTA_STATUS_RES_BIT    (UINT32)0x00200000
#define EV_LWM2M_FOTA_ACK_RES_BIT       (UINT32)0x00400000

#define EV_LWM2M_FAIL_RES_BIT           (UINT32)0x80000000
#define EV_MON_URC_RECEIVED  2
#define EV_URC_TO_BE_ENABLED 3


/*OBJECTs and RESOURCEs IDs*/
#define DEVICE_OBJ_ID 3
#define   DEVICE_BATTERY_RES_ID 9 /*integer, percentage*/

#define FIRMWARE_UPDATE_OBJ_ID 5


#define LOCATION_OBJ_ID 6
#define   LOCATION_LATITUDE_RES_ID 0 /*float, degrees*/
#define   LOCATION_LONGITUDE_RES_ID 1 /*float, degrees*/

#define CONN_STATS_OBJ_ID 7
#define   CONN_STATS_STOP_EXEC_RES_ID 7
#define   CONN_STATS_COLLECTION_PERIOD_RES_ID 8 /* integer, in seconds */

#define APN_CONN_PROFILE_OBJ_ID 11
#define   APN_PROFILE_APN_1_RES_ID 1 /*first CID apn string*/


/* Global typedefs ==============================================================================*/
typedef enum {
	INIT=0,

	APPLICATION_EXIT
} APP_STATES;



/* Global functions =============================================================================*/

/**
  @brief        Initialize OneEdge connection
  @param[in]    obj_id      Object ID to be used for initialization
  @return       result of initialization

*/
UINT8 oneedge_init( void );

INT32 msgLWM2MTask(INT32 type, INT32 param1, INT32 param2);
#endif /* HDR_LWM2M_DEMO_H_ */
