/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */

/**
  @file
    SMTP_test.h

  @brief
    demo related definitions

  @details
    
  @note
    Dependencies:
    m2mb_types.h

  @author
    Cristina Desogus

  @date
    21/12/2022
*/


#ifndef HDR_SMTP_TEST_H_
#define HDR_SMTP_TEST_H_

/* Global declarations ==========================================================================*/
#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define EV_PDP_BIT         (UINT32)0x2    /*0x0000000000000010*/

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

/* Global functions =============================================================================*/

INT32 M2M_msgSMTPTask(INT32 type, INT32 param1, INT32 param2);
#endif /* HDR_SMTP_TEST_H_ */
