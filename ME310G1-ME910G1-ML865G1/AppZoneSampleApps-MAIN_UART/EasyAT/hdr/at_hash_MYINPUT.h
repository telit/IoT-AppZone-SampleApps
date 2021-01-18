/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    at_hash_MYINPUT.h

  @brief
    The file contains the implementation of custom command AT#MYINPUT

  @author


  @date
    13/03/2020
*/

#ifndef HDR_AT_HASH_MYINPUT_H_
#define HDR_AT_HASH_MYINPUT_H_

void MYINPUT_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI );
void MYINPUT_INPUT_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI, M2MB_ATP_DELEGATION_IND_E delegationEvent, UINT16 msg_size, void *delegationEventMsg );

#endif /* HDR_AT_HASH_MYINPUT_H_ */
