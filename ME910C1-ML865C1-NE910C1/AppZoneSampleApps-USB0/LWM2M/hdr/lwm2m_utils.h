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
