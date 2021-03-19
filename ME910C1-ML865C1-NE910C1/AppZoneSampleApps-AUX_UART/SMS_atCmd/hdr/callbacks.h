/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    callbacks.h

  @brief
    <brief description>
  @details
    <detailed description>
  @note
    Dependencies:
    m2mb_types.h

  @author
		FabioPi
  @date
    11/02/2020
 */


#ifndef HDR_CALLBACKS_H_
#define HDR_CALLBACKS_H_

/* Global declarations ==========================================================================*/
#define EV_SMS_SEND         (UINT32)0x0001
#define EV_SMS_SEND_FROM_MEM    (UINT32)0x0010
#define EV_SMS_RECV         (UINT32)0x0011
#define EV_SMS_DELETE       (UINT32)0x0100
#define EV_SMS_WRITE        (UINT32)0x0101
#define EV_SMS_READ         (UINT32)0x0110
#define EV_SMS_SET_TAG        (UINT32)0x0111
#define EV_SMS_GET_STORAGE_STAT   (UINT32)0x1000
#define EV_SMS_GET_SCA        (UINT32)0x1001
#define EV_SMS_SET_SCA        (UINT32)0x1010
#define EV_SMS_GET_STORAGE_IDXS     (UINT32)0x1011
#define EV_SMS_MEM_FULL             (UINT32)0x1100
#define EV_SMS_MEM_FULL_REACHED     (UINT32)0x1101



/* Global typedefs ==============================================================================*/
/* Global functions =============================================================================*/
void Sms_Callback(M2MB_SMS_HANDLE h, M2MB_SMS_IND_E sms_event, UINT16 resp_size, void *resp_struct, void *myUserdata);
INT32 msgSMSparse(INT32 type, INT32 param1, INT32 param2);


#endif /* HDR_CALLBACKS_H_ */
