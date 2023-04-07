/*
 * M2M_WatchDog.h
 *
 *  Created on: 10 May 2021
 *      Author: Alberto Lendini
 */

#ifndef HDR_M2MB_WDOG_H_
#define HDR_M2MB_WDOG_H_

INT32 M2MB_msgTask1(INT32 type, INT32 param1, INT32 param2);
INT32 M2MB_msgTask2(INT32 type, INT32 param1, INT32 param2);

typedef enum {
  TASK_START=0,
  LOOP,
  WD_KICK
} STATES;

/* EVENTS */
#define EV_WDOG_KICK                   (UINT32)0x00000001
#define EV_WDOG_REMOVE                 (UINT32)0x00000002
#define EV_WDOG_RESET                  (UINT32)0x00000004
#define EV_WDOG_TEST                   (UINT32)0x00000008

/* TIMEOUTS */
#define WDOG_WAIT_TO                    120000
#define WDOG_REFERENCE_TIME             41500

/* FLAG FILE */
#define WDOG_FLAG "wdog_flag.txt"

/* FUNCTIONS */
void WDcallback( M2MB_WDOG_HANDLE hDog, M2MB_WDOG_IND_E wDog_event, UINT16 resp_size, void *resp_struct, void *userdata );
void WDog_test(void);

#endif /* HDR_M2MB_WDOG_H_ */
