/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    lwm2m_utils.h

  @brief
    lwm2m utilities definitions

  @details
    

  @note
    Dependencies:
    m2mb_types.h
    m2mb_lwm2m.h

  @author
    

  @date
    23/11/2021
*/


#ifndef HDR_LWM2M_UTILS_H_
#define HDR_LWM2M_UTILS_H_


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

#define EV_LWM2M_GET_REG_ENTRIES_RES_BIT  (UINT32)0x00010000
#define EV_LWM2M_REG_RES_BIT              (UINT32)0x00020000

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
/* Global typedefs ==============================================================================*/

/* Used for event responses in the indication callback */
typedef struct
{
  M2MB_LWM2M_RESULT_E  result;
  M2MB_LWM2M_OBJ_URI_T uri;
  UINT16 resp_len;
} LWM2M_EVENT_RES_S;

/* Used for event responses in the indication callback */


/* Global functions =============================================================================*/

/**
  @brief        Returns the Critical section semaphore handle
  @retval       CS semaphore handle
 */
M2MB_OS_SEM_HANDLE get_lwm2mCSsem_handle(void);


/**
  @brief        Returns the events section semaphore handle
  @retval       events handle
 */
M2MB_OS_EV_HANDLE get_lwm2mEvents_handle(void);

/**
  @brief        Returns the static event res structure pointer 
  @retval       events result structure pointer
 */
LWM2M_EVENT_RES_S *get_event_res_p(void);

/**
  @brief        Initializes the synchronization related resources
  @retval       0 if OK, other values error
 */
INT32 init_sync(void);

/**
  @brief        Releases the synchronization related resources
  @retval       none
 */
void deinit_sync(void);

/**
  @brief        Sets a generic read-only resource to the provided URI, and waits the response of the LWM2M Client

  @param[in]    h           LWM2M handle
  @param[in]    uri         pointer to the uri structure
  @param[in]    inbuf       generic pointer to the data to be set
  @param[in]    inbuflen    size of the data to be set

  @retval               0 if OK, other values error
 */
INT32 set_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 inbuflen  );

/**
  @brief        Writes a generic r/w resource to the provided URI, and waits the response of the LWM2M Client

  @param[in]    h           LWM2M handle
  @param[in]    uri         pointer to the uri structure
  @param[in]    inbuf       generic pointer to the data to be written
  @param[in]    inbuflen    size of the data to be written

  @retval               0 if OK, other values error
 */
INT32 write_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 inbuflen);

/**
  @brief        Gets a generic read-only resource from the provided URI, and waits the response of the LWM2M Client

  @param[in]    h           LWM2M handle
  @param[in]    uri         pointer to the uri structure
  @param[in]    inbuf       generic pointer to the data to be received
  @param[inout]    inbuflen    max size of the data to be received. In case of opaque, it returns the amount of gotten data

  @retval               0 if OK, other values error
 */
INT32 get_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 *inbuflen);

/**
  @brief        Reads a generic r/w resource from the provided URI, and waits the response of the LWM2M Client

  @param[in]    h           LWM2M handle
  @param[in]    uri         pointer to the uri structure
  @param[in]    inbuf       generic pointer to the data to be read
  @param[inout] inbuflen    max size of the data to be read. In case of opaque, it returns the amount of read data

  @retval               0 if OK, other values error
 */
INT32 read_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *uri, void *inbuf, UINT16 *inbuflen);

/*=====================================
 * SET READ ONLY RESOURCES UTILITIES
 * ===================================*/

/*!
 * @brief        Used to set a read only integer resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 set_read_only_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 value);

/*!
 * @brief        Used to set a read only timestamp resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 set_read_only_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 value);

/*!
 * @brief        Used to set a read only double resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 set_read_only_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double value);

/*!
 * @brief        Used to set a read only boolean resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 set_read_only_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN value);

/*!
 * @brief        Used to set a read only opaque resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 set_read_only_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 datalen);

/*!
 * @brief        Used to set a read only string resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 set_read_only_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *data);


/*=====================================
 * WRITE RW (OR WRITE-ONLY) RESOURCES UTILITIES
 * ===================================*/
 
/*!
 * @brief        Used to write a write only or read-write integer resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 write_rw_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 value);

/*!
 * @brief        Used to write a write only or read-write timestamp resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 write_rw_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 value);

/*!
 * @brief        Used to write a write only or read-write double resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 write_rw_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double value);

/*!
 * @brief        Used to write a write only or read-write boolean resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 write_rw_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN value);

/*!
 * @brief        Used to write a write only or read-write opaque resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 write_rw_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 datalen);

/*!
 * @brief        Used to write a write only or read-write string resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    value to be set
 * @retval               0 if OK, other values error
 *
 */
INT32 write_rw_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *data);


/*=====================================
 * GET WRITE ONLY RESOURCES UTILITIES
 * ===================================*/

/*!
 * @brief        Used to get a write only integer resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 get_write_only_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 *value);

/*!
 * @brief        Used to get a write only timestamp resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 get_write_only_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 *value);

/*!
 * @brief        Used to get a write only double resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 get_write_only_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double *value);

/*!
 * @brief        Used to get a write only boolean resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 get_write_only_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN *value);

/*!
 * @brief        Used to get a write only opaque resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 get_write_only_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 *datalen);

/*!
 * @brief        Used to get a write only string resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 get_write_only_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *string, UINT16 string_max_size);


/*=====================================
 * READ RW (OR READ-ONLY) RESOURCES UTILITIES
 * ===================================*/
 
 
/*!
 * @brief        Used to read a read-only or read-write integer resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 read_rw_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 *value);

/*!
 * @brief        Used to read a read-only or read-write timestamp resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 read_rw_timestamp_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT64 *value);

/*!
 * @brief        Used to read a read-only or read-write double resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 read_rw_double_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double *value);

/*!
 * @brief        Used to read a read-only or read-write boolean resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 read_rw_boolean_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, BOOLEAN *value);

/*!
 * @brief        Used to read a read-only or read-write opaque resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 read_rw_opaque_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, UINT8 *data, UINT16 *datalen);

/*!
 * @brief        Used to read a read-only or read-write string resource on LwM2M
 *
 * @param[in]    h        The agent handle
 * @param[in]    pUri     pointer to uri structure to be set
 * @param[in]    value    pointer to the variable that will hold the retrieved value
 * @retval               0 if OK, other values error
 *
 */
INT32 read_rw_string_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, CHAR *string, UINT16 string_max_size);


#endif /* HDR_LWM2M_UTILS_H_ */
