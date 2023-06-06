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

/* Global typedefs ==============================================================================*/
typedef enum {
  INIT=0,

  APPLICATION_EXIT
} APP_STATES;
/* Global functions =============================================================================*/
/*-----------------------------------------------------------------------------
*!
* @brief
*   Initialize OneEdge connection
*
* @details
*
* @param
* @param[in]    obj_id      Object ID to be used for initialization
*
* @return
*   Eesult of initialization
*
* @see
*
* @note
*
* @code
*
* @endcode
*
-----------------------------------------------------------------------------*/
UINT8 oneedge_init( void );

/*-----------------------------------------------------------------------------
*!
* @brief
*   LWM2M example task
*
* @details
*
* @param
* @param[in]    obj_id      Object ID to be used for initialization
*
* @return
*   Eesult of initialization
*
* @see
*
* @note
*
* @code
*
* @endcode
*
-----------------------------------------------------------------------------*/
INT32 msgLWM2MTask(INT32 type, INT32 param1, INT32 param2);
#endif /* HDR_LWM2M_DEMO_H_ */
