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

  @author
    

  @date
    21/05/2020
*/


#ifndef HDR_LWM2M_DEMO_H_
#define HDR_LWM2M_DEMO_H_


/* Global declarations ==========================================================================*/
/*cellular event bits*/
#define EV_NET_BIT         (UINT32)0x1
#define EV_PDP_BIT         (UINT32)0x2

/*LWM2M event bits*/
#define EV_LWM2M_ENABLE_RES_BIT         (UINT32)0x00000001
#define EV_LWM2M_SET_RES_BIT            (UINT32)0x00000002
#define EV_LWM2M_GET_RES_BIT            (UINT32)0x00000004
#define EV_LWM2M_MON_RES_BIT            (UINT32)0x00000008
#define EV_LWM2M_WRITE_RES_BIT          (UINT32)0x00000010
#define EV_LWM2M_READ_RES_BIT           (UINT32)0x00000020
#define EV_LWM2M_SRV_REG_BIT            (UINT32)0x00000040
#define EV_LWM2M_GET_STAT_RES_BIT       (UINT32)0x00000080


#define EV_MON_URC_RECEIVED  2
#define EV_URC_TO_BE_ENABLED 3



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

INT32 set_read_only_integer_resource(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, int value);
M2MB_RESULT_E read_integer_from_uri(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, INT32 *result);
M2MB_RESULT_E read_double_from_uri(M2MB_LWM2M_HANDLE h, M2MB_LWM2M_OBJ_URI_T *pUri, double *result);


INT32 msgLWM2MTask(INT32 type, INT32 param1, INT32 param2);
#endif /* HDR_LWM2M_DEMO_H_ */
