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
    21/05/2020
*/


#ifndef HDR_LWM2M_DEMO_H_
#define HDR_LWM2M_DEMO_H_


/* Global declarations ==========================================================================*/


/*LWM2M event bits*/
#define EV_LWM2M_ENABLE_RES_BIT         (UINT32)0x00000001
#define EV_LWM2M_DISABLE_RES_BIT        (UINT32)0x00000002
#define EV_LWM2M_SET_RES_BIT            (UINT32)0x00000004
#define EV_LWM2M_GET_RES_BIT            (UINT32)0x00000008
#define EV_LWM2M_WRITE_RES_BIT          (UINT32)0x00000010
#define EV_LWM2M_READ_RES_BIT           (UINT32)0x00000020
#define EV_LWM2M_MON_RES_BIT            (UINT32)0x00000040
#define EV_LWM2M_SRV_REG_BIT            (UINT32)0x00000080
#define EV_LWM2M_GET_STAT_RES_BIT       (UINT32)0x00000100

#define EV_LWM2M_GET_LIST_RES_BIT       (UINT32)0x00000200
#define EV_LWM2M_SRV_DISABLED_BIT       (UINT32)0x00000400
#define EV_LWM2M_EXIST_RES_BIT          (UINT32)0x00000800

#define EV_LWM2M_NFYADD_RES_BIT         (UINT32)0x00001000
#define EV_LWM2M_NFYACK_BIT             (UINT32)0x00002000
#define EV_LWM2M_NFYACK_LIST_RES_BIT    (UINT32)0x00004000
#define EV_LWM2M_NFYSTAT_RES_BIT        (UINT32)0x00008000

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

#define M2MB_LWM2M_DEMO_OBJ_ID          32010
#define   DEMO_STRING_R_RES_ID          1        /* single string resource, Read only*/
#define   DEMO_INT_R_RES_ID             2        /* single integer resource, Read only*/
#define   DEMO_FLOAT_R_RES_ID           3        /* single floating point resource, Read only*/
#define   DEMO_BOOL_R_RES_ID            4        /* single boolean resource, Read only*/
#define   DEMO_OPAQUE_R_RES_ID          5        /* single opaque (raw data) resource, Read only*/
#define   DEMO_TIME_R_RES_ID            6        /* single time resource, Read only*/
#define   DEMO_OBJLINK_R_RES_ID         7        /* single object link resource, Read only*/

#define   DEMO_STRING_RW_RES_ID         11       /* single string resource, Read + Write*/
#define   DEMO_INT_RW_RES_ID            12       /* single integer resource, Read + Write*/
#define   DEMO_FLOAT_RW_RES_ID          13       /* single floating point resource, Read + Write*/
#define   DEMO_BOOL_RW_RES_ID           14       /* single boolean resource, Read + Write*/
#define   DEMO_OPAQUE_RW_RES_ID         15       /* single opaque (raw data) resource, Read + Write*/
#define   DEMO_TIME_RW_RES_ID           16       /* single time resource, Read + Write*/
#define   DEMO_OBJLINK_RW_RES_ID        17       /* single object link resource, Read + Write*/

#define   DEMO_MULTI_STRING_R_RES_ID    21       /* multiple string resource, Read only*/
#define   DEMO_MULTI_INT_R_RES_ID       22       /* multiple integer resource, Read only*/
#define   DEMO_MULTI_FLOAT_R_RES_ID     23       /* multiple floating point resource, Read only*/
#define   DEMO_MULTI_BOOL_R_RES_ID      24       /* multiple boolean resource, Read only*/
#define   DEMO_MULTI_OPAQUE_R_RES_ID    25       /* multiple opaque (raw data) resource, Read only*/
#define   DEMO_MULTI_TIME_R_RES_ID      26       /* multiple time resource, Read only*/
#define   DEMO_MULTI_OBJLINK_R_RES_ID   27       /* multiple object link resource, Read only*/

#define   DEMO_MULTI_STRING_RW_RES_ID   31       /* multiple string resource, Read + Write*/
#define   DEMO_MULTI_INT_RW_RES_ID      32       /* multiple integer resource, Read + Write*/
#define   DEMO_MULTI_FLOAT_RW_RES_ID    33       /* multiple floating point resource, Read + Write*/
#define   DEMO_MULTI_BOOL_RW_RES_ID     34       /* multiple boolean resource, Read + Write*/
#define   DEMO_MULTI_OPAQUE_RW_RES_ID   35       /* multiple opaque (raw data) resource, Read + Write*/
#define   DEMO_MULTI_TIME_RW_RES_ID     36       /* multiple time resource, Read + Write*/
#define   DEMO_MULTI_OBJLINK_RW_RES_ID  37       /* multiple object link resource, Read + Write*/

#define   DEMO_STRING_W_RES_ID          41       /* single string resource, Write only*/
#define   DEMO_INT_W_RES_ID             42       /* single integer resource, Write only*/
#define   DEMO_FLOAT_W_RES_ID           43       /* single floating point resource, Write only*/
#define   DEMO_BOOL_W_RES_ID            44       /* single boolean resource, Write only*/
#define   DEMO_OPAQUE_W_RES_ID          45       /* single opaque (raw data) resource, Write only*/
#define   DEMO_TIME_W_RES_ID            46       /* single time resource, Write only*/
#define   DEMO_OBJLINK_W_RES_ID         47       /* single object link resource, Write only*/

#define   DEMO_MULTI_STRING_W_RES_ID    51       /* multiple string resource, Write only*/
#define   DEMO_MULTI_INT_W_RES_ID       52       /* multiple integer resource, Write only*/
#define   DEMO_MULTI_FLOAT_W_RES_ID     53       /* multiple floating point resource, Write only*/
#define   DEMO_MULTI_BOOL_W_RES_ID      54       /* multiple boolean resource, Write only*/
#define   DEMO_MULTI_OPAQUE_W_RES_ID    55       /* multiple opaque (raw data) resource, Write only*/
#define   DEMO_MULTI_TIME_W_RES_ID      56       /* multiple time resource, Write only*/
#define   DEMO_MULTI_OBJLINK_W_RES_ID   57       /* multiple object link resource, Write only*/

#define   DEMO_EXEC1_RES_ID             101      /* execute resource 1*/


#define OBJECT_XML_NAME "object_32010.xml"


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
