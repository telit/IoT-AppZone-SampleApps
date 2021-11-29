/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
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

  @author


  @date
    22/11/2021
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

#define EV_LWM2M_EXIST_RES_BIT          (UINT32)0x00000400

#define EV_LWM2M_OBJGET_RES_BIT         (UINT32)0x00100000
#define EV_LWM2M_OBJSET_RES_BIT         (UINT32)0x00200000
#define EV_LWM2M_FAIL_RES_BIT           (UINT32)0x80000000
#define EV_MON_URC_RECEIVED  2
#define EV_URC_TO_BE_ENABLED 3


/*OBJECTs and RESOURCEs IDs*/
#define DEVICE_OBJ_ID 3
#define   DEVICE_BATTERY_RES_ID 9 /*integer, percentage*/


#define LOCATION_OBJ_ID 6
#define   LOCATION_LATITUDE_RES_ID 0 /*float, degrees*/
#define   LOCATION_LONGITUDE_RES_ID 1 /*float, degrees*/

#define CONN_STATS_OBJ_ID 7
#define   CONN_STATS_STOP_EXEC_RES_ID 7
#define   CONN_STATS_COLLECTION_PERIOD_RES_ID 8 /* integer, in seconds */

#define APN_CONN_PROFILE_OBJ_ID 11
#define   APN_PROFILE_APN_1_RES_ID 1 /*first CID apn string*/

#define M2MB_LWM2M_DEMO_OBJ_ID          32011
#define   DEMO_STRING_RW_RES_ID         11       /* single string resource, Read + Write*/
#define   DEMO_INT_RW_RES_ID            12       /* single integer resource, Read + Write*/
#define   DEMO_FLOAT_RW_RES_ID          13       /* single floating point resource, Read + Write*/
#define   DEMO_BOOL_RW_RES_ID           14       /* single boolean resource, Read + Write*/
#define   DEMO_OPAQUE_RW_RES_ID         15       /* single opaque (raw data) resource, Read + Write*/
#define   DEMO_TIME_RW_RES_ID           16       /* single time resource, Read + Write*/
#define   DEMO_OBJLINK_RW_RES_ID        17       /* single object link resource, Read + Write*/

#define   DEMO_MULTI_STRING_RW_RES_ID   31       /* multiple string resource, Read + Write*/
#define   DEMO_MULTI_BOOL_RW_RES_ID     34       /* multiple boolean resource, Read + Write*/

#define OBJECT_XML_NAME "object_32011.xml"


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
