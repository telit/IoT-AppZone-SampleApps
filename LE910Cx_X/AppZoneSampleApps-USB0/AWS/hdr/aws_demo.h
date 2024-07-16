/*Copyright (C) 2021 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    aws_demo.h

  @brief
    demo related definitions

  @details


  @note
    Dependencies:
    m2mb_types.h

  @author


  @date
    23/04/2021
*/


#ifndef HDR_SSL_TEST_H_
#define HDR_SSL_TEST_H_


/* Global declarations ==========================================================================*/
#define EV_NET_BIT              (UINT32)0x1    /*0x0000000000000001*/
#define EV_PDP_UP_BIT           (UINT32)0x2    /*0x0000000000000010*/
#define EV_PDP_DOWN_BIT         (UINT32)0x4    /*0x0000000000000100*/
#define EV_NET_SIGNAL_BIT       (UINT32)0x8    /*0x0000000000001000*/

/* Global typedefs ==============================================================================*/
typedef enum
{
  INIT = 0,
  WAIT_FOR_REGISTRATION,
  REGISTERED,
  PDP_ACTIVATION,
  MQTT_START,
  SSL_DISCONNECT,
  APPLICATION_EXIT
} APP_STATES;

/* Global functions =============================================================================*/
INT32 AWS_Task( INT32 type, INT32 param1, INT32 param2 );
#endif /* HDR_SSL_TEST_H_ */
