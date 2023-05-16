/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    at_hash_MYCMD.c

  @brief
    The file contains the implementation of custom command AT#MYCMD

  @author


  @date
    13/03/2020
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_socket.h"


#include "m2mb_atp.h"

#include "azx_log.h"


#include "azx_easy_at.h"

#include "at_common.h"


#define BUFF_OF 0xFF

#define MAX_DATA_LEN 256

typedef struct
{
  BOOLEAN           InputModeEnabled;
  UINT8             *Buffer;
  UINT8             TmpBuffer[5];
  UINT16            TotalCharNum;
  UINT16            BufferIndex;
  UINT16            BufferLength;
  BOOLEAN           BufferValid;
  UINT16            BufferNotValidChars;
  M2MB_ATP_HANDLE   handle;
  UINT16            atpI;

} AT_CMD_INPUT_DATA_T;



static AT_CMD_INPUT_DATA_T at_cmd_input;



static void at_cmd_InputModeStart( AT_CMD_INPUT_DATA_T *pInput, AZX_EASY_AT_HANDLES_T *pHdls, UINT16 BuffLength )
{
  if(  pInput->BufferValid == TRUE)
  {
    return; /*Already allocated*/
  }
  pInput->InputModeEnabled = TRUE;
  pInput->Buffer = ( UINT8 * )m2mb_os_malloc( BuffLength + 1 );
  memset( pInput->Buffer, 0x00, BuffLength + 1 );

  pInput->BufferLength = BuffLength;
  pInput->BufferIndex = 0;
  pInput->TotalCharNum = 0;
  memset( pInput->TmpBuffer, 0x00, sizeof( pInput->TmpBuffer ) );

  pInput->BufferValid = TRUE;
  pInput->BufferNotValidChars = 0;
  pInput->handle = pHdls->handle;
  pInput->atpI = pHdls->atpI;
}

static void at_cmd_InputModeEnd(  AT_CMD_INPUT_DATA_T *pInput )
{
  pInput->InputModeEnabled = FALSE;
  if( pInput->Buffer )
  {
    m2mb_os_free( pInput->Buffer );
    pInput->Buffer = NULL;
  }
  pInput->BufferValid = FALSE;
  m2mb_atp_change_input_mode( pInput->handle, pInput->atpI, M2MB_ATP_INPUT_MODE_NONE );
}

static void  at_cmd_HandleInputAddData( AT_CMD_INPUT_DATA_T *pInput, UINT8 byte )
{

  AZX_LOG_TRACE("Index: %u; length: %u; total: %u\r\n", pInput->BufferIndex, pInput->BufferLength, pInput->TotalCharNum);

  if( pInput->BufferIndex < pInput->BufferLength )
  {
    AZX_LOG_TRACE("++\r\n");
    pInput->Buffer[pInput->BufferIndex++] = byte;
  }
  pInput->TotalCharNum++;
}

static void  at_cmd_HandleInputBackspace( AT_CMD_INPUT_DATA_T *pInput )
{

  if( pInput->BufferIndex > 0 )
  {
    AZX_LOG_DEBUG("--\r\n");
    pInput->Buffer[pInput->BufferIndex--] = 0x00;
  }
  pInput->TotalCharNum--;
}


static UINT8 at_cmd_HandleInput(  AT_CMD_INPUT_DATA_T *pInput )
{
  UINT8 tmp_buf[8];
  AZX_LOG_TRACE( "Handle data from user...\r\n" );

  while( m2mb_atp_rx( pInput->handle, pInput->atpI, tmp_buf, 1 ) == 1 )
  {
    AZX_LOG_TRACE("Input char is %c, %X\r\n", tmp_buf[0], tmp_buf[0] );
    switch( tmp_buf[0] )
    {
    case CTRL_Z:
      pInput->Buffer[pInput->BufferIndex] = 0x00;
      return CTRL_Z;

    case ESC:
      pInput->Buffer[pInput->BufferIndex] = 0x00;
      return ESC;
    case BS:
      at_cmd_HandleInputBackspace( pInput );
      break;

    case CR:
      AZX_LOG_DEBUG("Carriage Return\r\n");
      /* TODO, add to buffer (see default case) or manage according to the application logic */
      break;

    default:
      at_cmd_HandleInputAddData( pInput, tmp_buf[0] );
      if( pInput->TotalCharNum > pInput->BufferLength )
      {
        return BUFF_OF;
      }

      break;
    }
  }
  return 0;
}


void MYINPUT_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI )
{
  M2MB_ATP_PARAM_T *atpParam;
  char rsp_buf[128];
  char *p_param = NULL;
  UINT32 data_len;
  AZX_EASY_AT_HANDLES_T hdls;

  m2mb_atp_get_input_data( atpHandle, atpI, &atpParam );
  hdls.handle = atpHandle;
  hdls.atpI = atpI;

  if( NULL == atpHandle )
  {
    AZX_LOG_ERROR( "atpHandle is NULL\r\n" );
  }

  AZX_EASY_AT_CMD_INFO( &hdls );

  switch( atpParam->type )
  {
  /*AT#MYINPUT=<param><CR>*/
  case M2MB_ATP_CMDTYP_SET:
    if( atpParam->itemNum < 1 )
    {
      AZX_LOG_ERROR( "Operation not allowed!\r\n" );
      AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls,  M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      return;
    }
    p_param = atpParam->item[0];
    if(azx_easy_at_strToUL(p_param, &data_len) < 0)
    {
      AZX_LOG_ERROR("%s undefined value\r\n", p_param);
      AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
    }
    else
    {
      AZX_LOG_INFO( "Data length is <%u>\r\n", data_len );
      if (data_len> MAX_DATA_LEN)
      {
        AZX_LOG_ERROR("Data length is too big\r\n");
        AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      }
      else
      {
        AZX_LOG_INFO("Start data mode..\r\n");
        /*Configure the at parser so input data will be managed by the application*/
        m2mb_atp_change_input_mode(atpHandle, atpI, M2MB_ATP_SMS_INPUT_MODE);
        at_cmd_InputModeStart(&at_cmd_input, &hdls, data_len); /*reserve a data_len bytes buffer*/
      }
    }
    break;

    /*AT#MYINPUT?<CR>*/
  case M2MB_ATP_CMDTYP_READ:
    /*Release AT instance with failure*/
    AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
    break;

    /*AT#MYINPUT=?<CR>*/
  case M2MB_ATP_CMDTYP_TEST:
    sprintf( rsp_buf, "%s: %d",
        atpParam->atpCmdString, MAX_DATA_LEN );
    m2mb_atp_msgout( atpHandle, atpI, rsp_buf );
    AZX_EASY_AT_RELEASE_WITH_SUCCESS( &hdls );
    break;

    /*AT#MYINPUT<CR>*/
  case M2MB_ATP_CMDTYP_EXEC:
    /*Release AT instance with failure*/
    AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
    break;

    /*AT#MYCMD=<CR>*/
  case M2MB_ATP_CMDTYP_NOPARAMS:
    /*Release AT instance with failure*/
    AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
    break;

  default:
    /*Release AT instance with failure*/
    AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
    break;
  } // end switch
}

void MYINPUT_INPUT_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI, M2MB_ATP_DELEGATION_IND_E delegationEvent, UINT16 msg_size, void *delegationEventMsg )
{

  UINT8 byte = 0;

  M2MB_ATP_PARAM_T *atpParam;
  m2mb_atp_get_input_data( atpHandle, atpI, &atpParam );

  AZX_EASY_AT_HANDLES_T hdls = {atpHandle, atpI};

  AT_CMD_INPUT_DATA_T *pInput = &at_cmd_input;

  if(NULL == atpHandle)
  {
    AZX_LOG_ERROR("atpHandle is NULL\r\n");
  }

  AZX_LOG_TRACE("Delegation message: %.*s\r\n", msg_size, delegationEventMsg);
  switch (delegationEvent)
  {
  case M2MB_ATP_DATA_IND:
    AZX_LOG_DEBUG("Data received!\r\n");
    byte = at_cmd_HandleInput( pInput );
    switch( byte )
    {
    case CTRL_Z:
      AZX_LOG_INFO( "CTRL_Z received! Data (%u bytes): <<%.*s>>\r\n", pInput->TotalCharNum, pInput->BufferLength, pInput->Buffer );
      at_cmd_InputModeEnd( pInput );
      AZX_EASY_AT_RELEASE_WITH_SUCCESS( &hdls );
      break;

    case ESC:
      at_cmd_InputModeEnd( pInput );
      AZX_EASY_AT_RELEASE_WITH_SUCCESS( &hdls );
      break;

    case BUFF_OF:
      AZX_LOG_ERROR("Buffer overflow\r\n");
      at_cmd_InputModeEnd( pInput );
      AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      break;

    default:
      break;//return ATP_CMD_RESULT_NO_CHANGE;   // No End Typing
    }
    break;
  case M2MB_ATP_ESCAPE_IND:
    AZX_LOG_DEBUG("Escape received!\r\n");
    AZX_EASY_AT_RELEASE_WITH_SUCCESS( &hdls );
    break;
    
  default:
    break;
  }

}
