/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */

/**
  @file
    m2mb_HTTP_test.h

  @brief
    demo related definitions

  @details


  @note
    Dependencies:
    m2mb_types.h

  @author
    Cristina Desogus

  @date
    10/10/2022
 */

#ifndef HDR_M2MB_HTTP_TEST_H_
#define HDR_M2MB_HTTP_TEST_H_
/* Global declarations ==========================================================================*/


/* Global typedefs ==============================================================================*/

typedef enum {
  INIT=0,
  WAIT_FOR_REGISTRATION,
  REGISTERED,
  LAUNCH_TEST,
  SOCKET_CREATE,
  SOCKET_CONNECT,
  SOCKET_SEND,
  SOCKET_RECV,
  APPLICATION_EXIT
} APP_STATES;

typedef enum {
  HTTPS_AND_CHUNKED_GET = 0,
  HTTPS_SERVER_AUTH_GET = 1,
  HTTP_GET = 2,
  HTTP_BASIC_AUTH_GET = 3,
  HTTP_HEAD = 4,
  HTTP_POST = 5,
  HTTP_RANGE_GET = 6
} OPERATION_TYPE;

/* Global functions =============================================================================*/

INT32 M2M_msgHTTPTask(INT32 type, INT32 param1, INT32 param2);


#endif /* HDR_M2MB_HTTP_TEST_H_ */
