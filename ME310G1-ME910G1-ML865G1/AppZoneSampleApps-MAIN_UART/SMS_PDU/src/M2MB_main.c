/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details

  @description
    Sample application showcasing how to create and decode PDUs to be used with m2mb_sms_* API set. A SIM card and antenna must be present. Debug prints on MAIN UART

  @version
    1.0.8
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    02/03/2017
 */
/* Include files ================================================================================*/
#include <stdio.h>
#include <string.h>
#include "m2mb_types.h"

#include "m2mb_os_types.h"
#include "m2mb_os_api.h"

#include "m2mb_fs_posix.h"

#include "m2mb_sms.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "azx_tasks.h"

#include "app_cfg.h"

#include "azx_pduEnc.h"
#include "azx_pduDec.h"

#include "callbacks.h"

#include "read_parameters.h"

/* Local defines ================================================================================*/

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/


M2MB_SMS_HANDLE h_sms_handle;

UINT8 *pdu_provv, *pdu;
INT32 pdulen;

M2MB_OS_EV_HANDLE sms_evHandle = NULL;
static UINT32  curEvBits;

extern M2MB_SMS_STORAGE_E memory;

/* Local function prototypes ====================================================================*/

/* Static functions =============================================================================*/


/* Global functions =============================================================================*/

/*-----------------------------------------------------------------------------------------------*/

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;

  M2MB_OS_RESULT_E  osRes;

  M2MB_RESULT_E     retVal;

  char PhoneNumber[32];

  INT32 smsDelTaskID;

  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;

  azx_sleep_ms(2000);

  azx_tasks_init();  //Init tasks wrap subsystem

  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting SMS PDU demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  AZX_LOG_DEBUG("INIT\r\n");

  configureParameters(); /*set default values first*/
  readConfigFromFile(); /*try to read configuration from file (if present)*/


  /* Init events handler */
  osRes  = m2mb_os_ev_setAttrItem( &evAttrHandle, CMDS_ARGS(M2MB_OS_EV_SEL_CMD_CREATE_ATTR, NULL, M2MB_OS_EV_SEL_CMD_NAME, "sms_ev"));
  osRes = m2mb_os_ev_init( &sms_evHandle, &evAttrHandle );

  if ( osRes != M2MB_OS_SUCCESS )
  {
    m2mb_os_ev_setAttrItem( &evAttrHandle, M2MB_OS_EV_SEL_CMD_DEL_ATTR, NULL );
    AZX_LOG_CRITICAL("m2mb_os_ev_init failed!\r\n");
    return;
  }
  else
  {
    AZX_LOG_DEBUG("m2mb_os_ev_init success\r\n");
  }



  pdu_provv = (UINT8*) m2mb_os_malloc(SMS_PDU_MAX_SIZE * sizeof (UINT8));
  pdu = (UINT8*) m2mb_os_malloc(SMS_PDU_MAX_SIZE * sizeof (UINT8));

  /* As per spec, SENDER_NUMBER is the number of destination entity*/
  sprintf(PhoneNumber, "%s", gSENDER_NUMBER); /*remember to store the phone number in international format*/

  memset(pdu_provv, 0x00, SMS_PDU_MAX_SIZE);

  pdulen = azx_pdu_encode_generic(PDU_TYPE_INTERNATIONAL, PhoneNumber, (CHAR*) gMESSAGE, pdu_provv, PDU_DCS_7);
  //pdulen = azx_pdu_encode(PhoneNumber, (CHAR*) gMESSAGE, pdu_provv, PDU_DCS_7);

  /* pdulen will be changed after the pdu is converted into a binary stream */
  pdulen = azx_pdu_convertZeroPaddedHexIntoByte(pdu_provv, pdu, pdulen);

  retVal = m2mb_sms_init(&h_sms_handle, Sms_Callback, NULL);
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_INFO( "m2mb_sms_init() succeeded\r\n");
  }
  else
  {
    AZX_LOG_ERROR("m2mb_sms_init()failed\r\n");
    return;
  }

  smsDelTaskID = azx_tasks_createTask((char*) "SMSDelTask", AZX_TASKS_STACK_S, 10, AZX_TASKS_MBOX_S, sms_delTaskCB);
  if(-1 == smsDelTaskID)
  {
    AZX_LOG_ERROR("Cannot create sms delete task!!\r\n");
  }
  else
  {
    azx_tasks_sendMessageToTask(smsDelTaskID, (INT32) h_sms_handle,0,0);
  }

  retVal = m2mb_sms_enable_ind(h_sms_handle, M2MB_SMS_INCOMING_IND, 1);
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_DEBUG("M2MB_SMS_INCOMING_IND indication enabled\r\n");
  }
  else
  {
    AZX_LOG_ERROR("M2MB_SMS_INCOMING_IND indication failed\r\n");
  }

  retVal = m2mb_sms_enable_ind(h_sms_handle, M2MB_SMS_MEMORY_FULL_IND, 1);
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_DEBUG("M2MB_SMS_INCOMING_IND MEMORY FULL indication enabled\r\n");
  }
  else
  {
    AZX_LOG_ERROR("M2MB_SMS_INCOMING_IND MEMORY FULL indication failed\r\n");
  }

  memory = M2MB_SMS_STORAGE_SM;

  retVal = m2mb_sms_set_storage(h_sms_handle, memory);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Set storage to %d failed!\r\n", memory);
  }
  else
  {
    AZX_LOG_DEBUG("Storage set to %d\r\n", memory);
  }

/*
  M2MB_SMS_DISCARD         -> incoming SMS will be discarded
  M2MB_SMS_STORE_AND_ACK   -> incoming SMS will be stored and ack managed by Modem -> transactionID = -1, the SMS message will be stored
  M2MB_SMS_FORWARD_AND_ACK -> incoming SMS will be forwarded to app and ack managed by Modem -> transactionID = -1, the SMS message will not be stored
  M2MB_SMS_FORWARD_ONLY    -> incoming SMS will be forwarded to app and ack NOT managed by Modem, the SMS message will not be stored
                             -> transactionID >= 0 to demand ack management to application logic.
*/

  // m2mb_sms_set_route: set M2MB_SMS_FORWARD_AND_ACK for all SMS classes
  retVal = m2mb_sms_set_route(h_sms_handle, M2MB_SMS_CLASS_0, memory, M2MB_SMS_STORE_AND_ACK);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Set route for M2MB_SMS_CLASS_0 setting failed!\r\n");
  }
  retVal = m2mb_sms_set_route(h_sms_handle, M2MB_SMS_CLASS_1, memory, M2MB_SMS_STORE_AND_ACK);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Set route for M2MB_SMS_CLASS_1 setting failed!\r\n");
  }

  retVal = m2mb_sms_set_route(h_sms_handle, M2MB_SMS_CLASS_2, memory, M2MB_SMS_STORE_AND_ACK);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Set route for M2MB_SMS_CLASS_2 setting failed!\r\n");
  }

  retVal = m2mb_sms_set_route(h_sms_handle, M2MB_SMS_CLASS_3, memory, M2MB_SMS_STORE_AND_ACK);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Set route for M2MB_SMS_CLASS_3 setting failed!\r\n");
  }

  retVal = m2mb_sms_set_route(h_sms_handle, M2MB_SMS_CLASS_NONE, memory, M2MB_SMS_STORE_AND_ACK);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Set route for M2MB_SMS_CLASS_NONE setting failed!\r\n");
  }

  AZX_LOG_INFO("\r\nSending message <%s>...\r\n", gMESSAGE);
  retVal = m2mb_sms_send(h_sms_handle, pdulen, pdu);
  if (retVal == M2MB_RESULT_SUCCESS)
  {
    AZX_LOG_INFO(" m2mb_sms_send() - succeeded\r\n");
  }
  else
  {
    AZX_LOG_ERROR("m2mb_sms_send() - failure\r\n");
  }

  /*Wait for sms send event to occur (released in Sms_Callback function) */
  osRes = m2mb_os_ev_get(sms_evHandle, EV_SMS_SEND, M2MB_OS_EV_GET_ANY_AND_CLEAR, &curEvBits, M2MB_OS_MS2TICKS( 40000 ));
  if (osRes == M2MB_OS_SUCCESS)
  {
    AZX_LOG_INFO( "SMS correctly sent!\r\n" );
  }
  else if (osRes == M2MB_OS_NO_EVENTS)
  {
    AZX_LOG_ERROR("SMS not sent! - exit for timeout\r\n" );
  }
  else
  {
    AZX_LOG_ERROR("SMS not sent! - unexpected value %d returned\r\n", osRes);
  }

  //App waiting for incoming SMSs
  AZX_LOG_INFO( "Waiting for an incoming SMS...\r\n" );


#ifdef LE910CXL
  /* On Linux based LE910CX Linux, the end fo M2MB_main causes the return of the application.
  Add a loop to allow reception of messages*/
  while(1)
  {
    azx_sleep_ms(1000);
  }
#endif
}

