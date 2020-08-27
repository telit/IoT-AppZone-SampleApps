/*===============================================================================================*/
/*         >>> Copyright (C) Telit Communications S.p.A. Italy All Rights Reserved. <<<          */
/*!
  @file
    my_timer_class.cpp

  @brief
    implementation of timer class
  @details
    C++ classes methods cannot be passed as function pointers unless they are defined as static .
   typically, the error
   "ISO C++ forbids taking the address of an unqualified or parenthesized non-static member function to form a pointer to member function."
   is raised.
   This happens because non static methods append as first parameter "this", that is, the class instance.

   so if some point in the code requires the reference to a function defined as below
   int myfunction(int a, int b);

   and the class MyClass method prototype is
   int MyClass::mymethod(int a, int b);

   Even if the prototype is the same at first glance, a generic function pointer will raise error because the method will be converted in something
   like
   int mymethod(MyClass this, int a, int b);

   thus, mymethod cannot be passed directly as a function pointer. A solution would be defining the method as static (common among instances).
   this removes the "MyClass this" in the parameters, however it carries some limitations (for example, shared resources among instances).

   A more general approach solution is showed in this demo code.
   Explanation:
    In the class workspace, a static function (NOT a method, a normal function) is defined.
    It will be registered as the callback in the m2mb_hwTmr_setAttrItem API (which expects a function pointer).
    Then, as argument for the callback, the class instance itself will be passed in the constructor:

    hwRes = m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle,
          CMDS_ARGS(
              M2MB_HWTMR_SEL_CMD_CREATE_ATTR, NULL,
              M2MB_HWTMR_SEL_CMD_CB_FUNC, &TimerCb,
              M2MB_HWTMR_SEL_CMD_ARG_CB, this, <<<<<<<-------the class instance
              M2MB_HWTMR_SEL_CMD_TIME_DURATION, 1000000, //set timer to fire every secs (time unit is usecs)
              M2MB_HWTMR_SEL_CMD_PERIODIC, M2MB_HWTMR_ONESHOT_TMR,
              M2MB_HWTMR_SEL_CMD_AUTOSTART, M2MB_HWTMR_NOT_START
          )
      );

    the static TimerCb function will then cast the argCb as a static reference to the class instance and call its method, passing the input parameters:
    static_cast<HwTimer*>(argCb)->timer_cb(hwTmrHandle, argCb);

  @note
    Dependencies:
    m2mb_types.h

  @author

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


/* Include files ================================================================================*/
#include "stdio.h"
#include "string.h"

#include "m2mb_types.h"
#include "m2mb_hwTmr.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"
#include "hwtimer_class.h"


namespace HwTimerClass
{
/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
static void static_timerCb( M2MB_HWTMR_HANDLE hwTmrHandle, void *argCb )
{

  if(NULL == argCb)
  {
    return;
  }
  AZX_LOG_INFO("In the static timer callback. Calling class method...\r\n");

  //cast the argument as pointer to class instance and call the instance method
  static_cast<HwTimer*>(argCb)->timer_cb(hwTmrHandle, argCb);
}


HwTimer::HwTimer(UINT32 timeout_ms, const char *p_name)
{
  M2MB_HWTMR_RESULT_E        hwRes;
  M2MB_HWTMR_ATTR_HANDLE     tmrHwAttrHandle;

  hwRes = m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle,
      CMDS_ARGS(
          M2MB_HWTMR_SEL_CMD_CREATE_ATTR, NULL,
          M2MB_HWTMR_SEL_CMD_CB_FUNC, &static_timerCb,
          M2MB_HWTMR_SEL_CMD_ARG_CB, this, /* <<<<<<<-------the class instance is passed as argument */
          M2MB_HWTMR_SEL_CMD_TIME_DURATION, timeout_ms * 1000, //set timer to fire every timeout_ms milliseconds (time unit is usecs)
          M2MB_HWTMR_SEL_CMD_PERIODIC, M2MB_HWTMR_ONESHOT_TMR,
          M2MB_HWTMR_SEL_CMD_AUTOSTART, M2MB_HWTMR_NOT_START
      )
  );

  if( hwRes != M2MB_HWTMR_SUCCESS )
  {
    AZX_LOG_CRITICAL("\r\nCannot create the timer attributes, error %d", hwRes);
  }

  hwRes = m2mb_hwTmr_init( &this->handle, &tmrHwAttrHandle );
  if( hwRes != M2MB_HWTMR_SUCCESS )
  {
    m2mb_hwTmr_setAttrItem( &tmrHwAttrHandle, 1, M2MB_HWTMR_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_CRITICAL("\r\nCannot create the timer, error %d", hwRes);
  }


  memset(this->name, 0, sizeof(this->name));
  strncpy(this->name, p_name, sizeof(this->name));

  AZX_LOG_INFO("Timer \"%s\" created with %u ms timeout\r\n", this->name, timeout_ms);


}

HwTimer::~HwTimer(void)
{
  m2mb_hwTmr_stop(this->handle);
  m2mb_hwTmr_deinit(this->handle);
}


INT32 HwTimer::start(void)
{
  M2MB_HWTMR_RESULT_E        hwRes;
  AZX_LOG_DEBUG("Starting \"%s\" timer\r\n", this->name);
  hwRes = m2mb_hwTmr_start(this->handle);
  if( hwRes != M2MB_HWTMR_SUCCESS )
   {
     AZX_LOG_ERROR("\r\nCannot start the timer, error %d", hwRes);
   }
  return hwRes;
}


void HwTimer::timer_cb(M2MB_HWTMR_HANDLE handle, void *arg)
{
  (void) arg;
  AZX_LOG_DEBUG("\r\nTimer \"%s\" class callback called. Class instance: %p; handle: %p\r\n", this->name, this, handle);
}

}

/* Global functions =============================================================================*/

