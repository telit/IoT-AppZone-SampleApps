/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    fota.h

  @brief
    demo related definitions

  @details
    

  @note
    Dependencies:
    m2mb_types.h

  @author
    

  @date
    25/05/2018
*/

#ifndef HDR_FOTA_H_
#define HDR_FOTA_H_

#define EV_NET_BIT         (UINT32)0x1    /*0x0000000000000001*/
#define EV_PDP_BIT         (UINT32)0x2    /*0x0000000000000010*/

typedef enum {
	INIT=0,
	APPLICATION_EXIT
} APP_STATES;

typedef enum {
	INITFOTA = 0,
	RESET,
	GETSTATE,
	DOWNLOAD,
	UPDATE,
	CHECKDELTA,
	END
} FOTA_STATES;

INT32 mainTask(INT32 type, INT32 param1, INT32 param2);
INT32 fotaTask(INT32 type, INT32 param1, INT32 param2);

#endif /* HDR_FOTA_H_ */
