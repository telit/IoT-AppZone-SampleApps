/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ssl_test.h

  @brief
    demo related definitions

  @details
    

  @note
    Dependencies:
    m2mb_types.h

  @author
    

  @date
    09/04/2018
*/


#ifndef HDR_SSL_TEST_H_
#define HDR_SSL_TEST_H_


/* Global declarations ==========================================================================*/
#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define EV_PDP_BIT         (UINT32)0x2    /*0x0000000000000010*/

/* Global typedefs ==============================================================================*/
typedef enum {
  INIT=0,
  WAIT_FOR_REGISTRATION,
  REGISTERED,
  PDP_ACTIVATION,
  SOCKET_CREATE,
  SOCKET_CONNECT,
  SSL_SECURE_SOCKET,     // SECURE_SOCKET
  SSL_CONNECT,           // SSL_CONNECT
  SSL_SEND_RECV,         // SSL_ENCODE_SEND, SSL_DECODE
  SSL_DISCONNECT,
  APPLICATION_EXIT
} APP_STATES;

/* Global functions =============================================================================*/
INT32 msgHTTPSTask(INT32 type, INT32 param1, INT32 param2);
#endif /* HDR_SSL_TEST_H_ */
