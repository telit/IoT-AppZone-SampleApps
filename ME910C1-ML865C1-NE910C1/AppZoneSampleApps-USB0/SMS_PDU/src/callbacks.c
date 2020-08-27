/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    callbacks.c

  @brief
    sample app callback functions
  @details
    this file contains the implementation of sms related callback functions
  @note
    Dependencies:
    m2mb_types.h

  @date
    11/02/2020
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

#include "app_cfg.h"

#include "ul_gsm_pdu.h"
#include "azx_pduEnc.h"
#include "azx_pduDec.h"

#include "callbacks.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

const CHAR *action[]={"M2MB_SMS_DISCARD","M2MB_SMS_STORE_AND_ACK","M2MB_FORWARD_ONLY","M2MB_FORWARD_AND_ACK"};
const CHAR *storage[]={"M2MB_SMS_STORAGE_NONE","M2MB_SMS_STORAGE_SM","M2MB_SMS_STORAGE_ME"};
const CHAR *tag[]={"M2MB_SMS_TAG_MT_READ","M2MB_SMS_TAG_MT_NOT_READ","M2MB_SMS_TAG_MO_SENT","M2MB_SMS_TAG_MO_NOT_SENT"};
const CHAR *failure[]={"M2MB_SMS_ACK_OK","M2MB_SMS_ACK_FAIL_NO_NETWORK_RESPONSE","M2MB_SMS_ACK_FAIL_NETWORK_RELEASED_LINK", "M2MB_SMS_ACK_FAIL_ACK_NOT_SENT"};

const CHAR *format[]={"M2MB_SMS_FORMAT_3GPP","M2MB_SMS_FORMAT_3GPP2"};

M2MB_SMS_STORAGE_E stType, memory;

UINT8 SMS_center [500];
UINT8 SMSC_retr [500];
UINT8 SMS_indexes [500];
UINT32 globalIndex;

UINT16 SMS_indexes_size;
UINT32 memoryFullFlag = 0;
UINT32 CheckSCACorrectnessFlag=0;
UINT32 i,j;
INT32 currentSMS = 0, maxSMS = 0, ack=0;
INT32 ToBeAcked;

M2MB_RESULT_E SMS_del;

extern M2MB_OS_EV_HANDLE sms_evHandle;

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/
/* Global functions =============================================================================*/

void Sms_Callback(M2MB_SMS_HANDLE h, M2MB_SMS_IND_E sms_event, UINT16 resp_size, void *resp_struct, void *myUserdata)
{
  (void)resp_size;
  (void)resp_struct;
  (void)myUserdata;

  M2MB_OS_RESULT_E res;

  switch(sms_event)
  {

  case M2MB_SMS_SEND_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_SEND_RESP Callback\r\n\n");

    M2MB_SMS_SEND_RESP_T *resp = (M2MB_SMS_SEND_RESP_T*)resp_struct;
    AZX_LOG_INFO("Send resp msg ID %d \r\n", resp->messageRef);
    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_SEND, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }
  case M2MB_SMS_WRITE_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_WRITE_RESP Callback\r\n\n");

    M2MB_SMS_WRITE_RESP_T *resp = (M2MB_SMS_WRITE_RESP_T*)resp_struct;
    AZX_LOG_INFO("PDU index written is %ld \r\n", resp->index);
    globalIndex= resp->index;

    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_WRITE, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }


  case M2MB_SMS_READ_RESP:
  {
    pdu_struct packet;
    static char number[32];
    static char message[161];
    INT32 len;

    AZX_LOG_INFO("M2MB_SMS_READ_RESP Callback\r\n\n");

    M2MB_SMS_READ_RESP_T *resp = (M2MB_SMS_READ_RESP_T*)resp_struct;
    AZX_LOG_DEBUG("PDU index read is %ld \r\n", resp->index);
    AZX_LOG_DEBUG("PDU size is %d \r\n", resp->pdu_size);
    AZX_LOG_INFO("SMS tag %s \r\n", tag[resp->tag]);
    AZX_LOG_INFO("SMS format %s \r\n", format[resp->format]);

    memset(message,0, sizeof(message));
    len = azx_pdu_decode(resp->pdu, resp->pdu_size, &packet, number, message);


    AZX_LOG_DEBUG("Code type: %d\r\n", packet.tp_dcs);
    AZX_LOG_DEBUG("Sender type: %u\r\n", packet.sender.type);
    AZX_LOG_DEBUG("Msg len: %u\r\n", packet.msg.len);
    AZX_LOG_DEBUG("Msg bytes: %u\r\n", packet.msg.bytes);
    AZX_LOG_DEBUG("Protocol ID: %d\r\n", packet.tp_pid);

    AZX_LOG_INFO("Msg date %u/%u/%u %u:%u:%u (timezone: %.0f)\r\n", packet.year, packet.month, packet.date, packet.hour, packet.min, packet.sec, packet.tz / 4.0);

    AZX_LOG_INFO("Received SMS, content (len: %d): <<%s>>\r\n", len, message);
    AZX_LOG_INFO("Sender: %s\r\n", number);

    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_READ, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }

  case M2MB_SMS_SEND_FROM_MEM_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_SEND_FROM_MEM_RESP Callback\r\n\n");

    M2MB_SMS_SEND_RESP_T *resp = (M2MB_SMS_SEND_RESP_T*)resp_struct;
    AZX_LOG_INFO("Send resp msg ID %d \r\n", resp->messageRef);
    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_SEND_FROM_MEM, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }

  case M2MB_SMS_INCOMING_IND:
  {
    M2MB_SMS_INCOMING_T *resp = (M2MB_SMS_INCOMING_T*)resp_struct;

    AZX_LOG_INFO("M2MB_SMS_INCOMING_IND Callback\r\n\n");

    globalIndex=resp->index;

    /*initiate a read of the message*/
    m2mb_sms_read(h, globalIndex);

    break;
  }

  case M2MB_SMS_ACK_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_ACK_RESP Callback\r\n\n");

    M2MB_SMS_ACK_RESP_T *resp = (M2MB_SMS_ACK_RESP_T*)resp_struct;
    AZX_LOG_INFO("Ack resp is %s \r\n", failure[resp->failCause]);
    break;
  }

  case M2MB_SMS_DELETE_RESP:
  {

    M2MB_SMS_GENERIC_RESP_T *resp = (M2MB_SMS_GENERIC_RESP_T*)resp_struct;

    AZX_LOG_INFO("M2MB_SMS_DELETE_RESP Callback\r\n\n");

    AZX_LOG_INFO("Delete resp is %d \r\n", resp->response);
    SMS_del=resp->response;

    if(resp->response == M2MB_RESULT_SUCCESS)
    {
      memoryFullFlag = 0;
    }
    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_DELETE, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }

    break;
  }
  case M2MB_SMS_GET_SCA_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_GET_SCA_RESP Callback\r\n\n");

    M2MB_SMS_GET_SCA_RESP_T *resp = (M2MB_SMS_GET_SCA_RESP_T*)resp_struct;
    AZX_LOG_INFO("GET SCA resp is %s, %s \r\n", resp->addr_type, resp->sca);
    if(CheckSCACorrectnessFlag==1)
    {
      strcpy((char*)SMSC_retr,(char *)resp->sca);
      AZX_LOG_INFO("SMSC_retr: %s\r\n", SMSC_retr);
    }
    else
    {
      strcpy((char*)SMS_center,(char *)resp->sca);
      AZX_LOG_INFO("SCA SIM card center is %s \r\n", SMS_center);
    }

    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_GET_SCA, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }
  case M2MB_SMS_SET_SCA_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_SET_SCA_RESP Callback\r\n\n");

    M2MB_SMS_GENERIC_RESP_T *resp = (M2MB_SMS_GENERIC_RESP_T*)resp_struct;
    AZX_LOG_INFO( "SET SCA resp is %d \r\n", resp->response);

    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_SET_SCA, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    };
    break;
  }

  case M2MB_SMS_MEMORY_FULL_IND:
  {
    AZX_LOG_INFO("M2MB_SMS_MEMORY_FULL_IND Callback\r\n\n");

    M2MB_SMS_MEMORY_FULL_T *resp = (M2MB_SMS_MEMORY_FULL_T*)resp_struct;
    AZX_LOG_INFO( "## MEMORY FULL: format %d, storage %d \r\n",resp->format, resp->storage);
    memoryFullFlag = 1;
    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_MEM_FULL, M2MB_OS_EV_SET);

    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }


  case M2MB_SMS_GET_STORAGE_INDEXES_RESP:
  {
    UINT32 j;

    AZX_LOG_INFO("M2MB_SMS_GET_STORAGE_INDEXES_RESP Callback\r\n\n");

    M2MB_SMS_GET_STORAGE_INDEXES_RESP_T *resp = (M2MB_SMS_GET_STORAGE_INDEXES_RESP_T*)resp_struct;
    SMS_indexes_size = resp->indexes_size;
    AZX_LOG_INFO("PDU index is not NULL, indexes_size is %d \r\n", resp->indexes_size);
    if (resp->indexes_size != 0)
    {
      for(j=0; j<resp->indexes_size; j++)
      {
        SMS_indexes[j]=resp->indexes[j];
        AZX_LOG_INFO("PDU index %ld is %d \r\n ", j, resp->indexes[j]);
      }
    }
    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_READ, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }

  case M2MB_SMS_GET_STORAGE_STAT_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_GET_STORAGE_STAT_RESP Callback\r\n\n");

    M2MB_SMS_GET_STORAGE_STAT_RESP_T *resp = (M2MB_SMS_GET_STORAGE_STAT_RESP_T*)resp_struct;
    AZX_LOG_INFO("--- Storage status ---\r\n");
    AZX_LOG_INFO("Current SMS: %ld - Max capacity: %ld - Type: %d\r\n", resp->smsCounter, resp->maxStorageSize, resp->stType);
    currentSMS = resp->smsCounter;
    maxSMS = resp->maxStorageSize;
    memory= resp->stType;
    //AZX_LOG_INFO("type is %d \r\n", memory);
    AZX_LOG_INFO("--- End of storage status ---\r\n");

    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_READ, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }

  case M2MB_SMS_SET_TAG_RESP:
  {
    AZX_LOG_INFO("M2MB_SMS_SET_TAG_RESP Callback\r\n\n");

    M2MB_SMS_GENERIC_RESP_T *resp = (M2MB_SMS_GENERIC_RESP_T*)resp_struct;
    AZX_LOG_INFO("Set tag resp is %d \r\n", resp->response);
    res = m2mb_os_ev_set(sms_evHandle, EV_SMS_READ, M2MB_OS_EV_SET);
    if(M2MB_OS_SUCCESS != res)
    {
      AZX_LOG_ERROR("Event bits setting failure: %d\r\n", res);
    }
    break;
  }

  default:
  {
    AZX_LOG_INFO("SMS event %d\r\n", sms_event);
  }
  break;
  }
}


