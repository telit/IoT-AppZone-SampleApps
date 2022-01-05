/*
 * gps_task.h
 *
 *  Created on: 23 apr 2021
 *      Author: robertaga
 */

#ifndef HDR_GPS_TASK_H_
#define HDR_GPS_TASK_H_

#include "m2mb_types.h"

typedef enum {
	START_GPS,
	STOP_GPS

} GPS_APP_STATUS;


INT32 GPS_task(INT32 type, INT32 param1, INT32 param2);


#endif /* HDR_GPS_TASK_H_ */
