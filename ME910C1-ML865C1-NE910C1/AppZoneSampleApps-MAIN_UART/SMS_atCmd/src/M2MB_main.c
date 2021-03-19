/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing how to receive an SMS containing an AT command, process the AT command and send its answer to sender (configurable in sms_config.txt). A SIM card and antenna must be present. Debug prints on MAIN UART

  @version 
    1.0.2
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author
	Roberta Galeazzo

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
#include "m2mb_ati.h"

#include "azx_log.h"
#include "azx_utils.h"

#include "app_cfg.h"

#include "ul_gsm_pdu.h"
#include "azx_pduEnc.h"
#include "azx_pduDec.h"

#include "azx_tasks.h"

#include "callbacks.h"

#include "at_utils.h"

/* Local defines ================================================================================*/

/* Local typedefs ===============================================================================*/

/* Local statics ================================================================================*/

INT8 sendAnswerSms = 0;  //if 1 an SMS with command answer is sent back to sender, can be configured in sms_config.txt
INT8 deleteSMS = 0; //if 1 SMS will be deleted after reading, can be configured in sms_config.txt

M2MB_SMS_HANDLE h_sms_handle;

UINT8 *pdu_provv, *pdu;
INT32 pdulen;

M2MB_OS_EV_HANDLE sms_evHandle = NULL;

INT16 instanceID = 0; /*AT0, bound to UART by default config*/
M2MB_OS_SEM_HANDLE taskSemHandle = NULL;

extern M2MB_SMS_STORAGE_E memory;
extern const CHAR *storage[];
INT8 smsParsingTaskID;

const CHAR *cnfVal[]={"DISABLED","ENABLED"};

/* Local function prototypes ====================================================================*/

extern int readConfigFromFile(void);

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

  //char PhoneNumber[32];

  M2MB_OS_EV_ATTR_HANDLE  evAttrHandle;
  M2MB_OS_SEM_ATTR_HANDLE semAttrHandle;

  azx_sleep_ms(2000);



  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting SMS with AT command demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  azx_tasks_init();  //Init task

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


#if 0 //moved to msgSMSparse
  pdu_provv = (UINT8*) m2mb_os_malloc(SMS_PDU_MAX_SIZE * sizeof (UINT8));
  pdu = (UINT8*) m2mb_os_malloc(SMS_PDU_MAX_SIZE * sizeof (UINT8));

  sprintf(PhoneNumber, SENDER_NUMBER); //remember to store the phone number in international format

  memset(pdu_provv, 0x00, SMS_PDU_MAX_SIZE);
  pdulen = azx_pdu_encode(PhoneNumber, (CHAR*) MESSAGE, pdu_provv, PDU_DCS_7);
  
  /* pdulen will be changed after the pdu is converted into a binary stream */
  pdulen = azx_pdu_convertZeroPaddedHexIntoByte(pdu_provv, pdu, pdulen);
#endif

  //Init SMS
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

  //Enabling incoming SMS indication
  retVal = m2mb_sms_enable_ind(h_sms_handle, M2MB_SMS_INCOMING_IND, 1);
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_DEBUG("M2MB_SMS_INCOMING_IND indication enabled\r\n");
  }
  else
  {
    AZX_LOG_ERROR("M2MB_SMS_INCOMING_IND indication failed\r\n");
  }

  //Enable memory full
  retVal = m2mb_sms_enable_ind(h_sms_handle, M2MB_SMS_MEMORY_FULL_IND, 1);
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_DEBUG("M2MB_SMS_INCOMING_IND MEMORY FULL indication enabled\r\n");
  }
  else
  {
    AZX_LOG_ERROR("M2MB_SMS_INCOMING_IND MEMORY FULL indication failed\r\n");
  }

  //Set preferred memory (where SMS will be saved to) in this case SIM memory
  memory = M2MB_SMS_STORAGE_SM;

  retVal = m2mb_sms_set_storage(h_sms_handle, memory);
  if ( retVal != M2MB_RESULT_SUCCESS )
  {
    AZX_LOG_ERROR( "Set storage to %d failed!\r\n", memory);
  }
  else
  {
    AZX_LOG_DEBUG("Storage set to %s\r\n", storage[memory]);
  }

/*
  M2MB_SMS_DISCARD         -> incoming SMS will be discarded
  M2MB_SMS_STORE_AND_ACK   -> incoming SMS will be stored and ack managed by Modem -> transactionID = -1
  M2MB_SMS_FORWARD_AND_ACK -> incoming SMS will be forwarded to app and ack managed by Modem -> transactionID = -1
  M2MB_SMS_FORWARD_ONLY    -> incoming SMS will be forwarded to app and ack NOT managed by Modem
                             -> transactionID >= 0 to demand ack management to application logic.
*/

  // m2mb_sms_set_route: corresponding to +CNMI's <mt> parameter
  // M2MB_SMS_STORE_AND_ACK: an SMS of any class will be stored on memory and ACK handled by modem
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

  //Read SMS configuration files
  if(readConfigFromFile() < 0){
	  AZX_LOG_INFO("Default: SMS with answer sending %s, delete sms %s\r\n", cnfVal[sendAnswerSms], cnfVal[deleteSMS]);
  } else {
	  AZX_LOG_INFO("Config file: SMS with answer sending %s, delete sms %s\r\n", cnfVal[sendAnswerSms], cnfVal[deleteSMS]);
  }

  //Set AT commands interface
  retVal = at_cmd_async_init(instanceID);
  if ( retVal == M2MB_RESULT_SUCCESS )
  {
	  AZX_LOG_TRACE( "at_cmd_async_init() returned success value\r\n" );
  }
  else
  {
	  AZX_LOG_ERROR( "at_cmd_async_init() returned failure value\r\n" );
	  return;
  }

  //create task to handle SMS parsing
  /*Creating an InterProcess Communication (IPC) semaphore*/
	if (taskSemHandle == NULL)
	{
		 m2mb_os_sem_setAttrItem( &semAttrHandle, CMDS_ARGS(
		 M2MB_OS_SEM_SEL_CMD_CREATE_ATTR,  NULL,
		 M2MB_OS_SEM_SEL_CMD_COUNT, 0 /*IPC*/,
		 M2MB_OS_SEM_SEL_CMD_TYPE, M2MB_OS_SEM_GEN,M2MB_OS_SEM_SEL_CMD_NAME, "taskSem"));
		 m2mb_os_sem_init( &taskSemHandle, &semAttrHandle );
	}

	smsParsingTaskID = azx_tasks_createTask((char*) "SMSparsingTask", AZX_TASKS_STACK_M, 1, AZX_TASKS_MBOX_M, msgSMSparse);


   //waiting for an incoming SMS
	AZX_LOG_INFO("Please send an SMS with a configuration as (\"ATCMD: <atcmd>\")...\r\n");

}

