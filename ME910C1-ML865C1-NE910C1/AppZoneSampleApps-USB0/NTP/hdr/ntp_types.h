/*
 * m2mb_ntp.h
 *
 *  Created on: Apr 13, 2021
 *      Author: robertaga
 */

#ifndef HDR_NTP_TYPES_H_
#define HDR_NTP_TYPES_H_

typedef enum {
  INIT,
  SET_MODULE_RTC

} APP_STATUS;


INT32 NTP_task(INT32 type, INT32 param1, INT32 param2);

#endif /* HDR_NTP_TYPES_H_ */
