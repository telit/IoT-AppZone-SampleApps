/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
    hwtimer_class.h

  @brief
    declaration of timer class
  @details
    the timer class shows how to manage non-static methods as function pointers
  @note
    Dependencies:
    m2mb_types.h
    m2mb_hwTmr.h
  @author
		FabioPi
  @date
    09/07/2020
 */

/*=================================================================
#Telit Extensions
#
#Copyright (C) 2019, Telit Communications S.p.A.
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
#Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
#Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in
#the documentation and/or other materials provided with the distribution.
#
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS``AS IS'' AND ANY EXPRESS OR IMPLIED
#WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
#DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#POSSIBILITY OF SUCH DAMAGE.
#
==============================================================*/


#ifndef HDR_HWTIMER_CLASS_H_
#define HDR_HWTIMER_CLASS_H_

namespace HwTimerClass
{

/* Global declarations ==========================================================================*/
class HwTimer
{
public:

  HwTimer(UINT32 timeout_ms, const char *p_name);
  ~HwTimer(void);
  INT32 start(void);
  void timer_cb(M2MB_HWTMR_HANDLE handle, void *arg);
private:
  M2MB_HWTMR_HANDLE handle;
  char name[16];
  int data;
};



/* Global typedefs ==============================================================================*/
/* Global functions =============================================================================*/

}
#endif /* HDR_HWTIMER_CLASS_H_ */
