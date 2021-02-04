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

void MYCMD_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI )
{
  M2MB_ATP_PARAM_T *atpParam;
  char rsp_buf[128];
  char *p_param = NULL;
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
    /*AT#MYCMD=<param><CR>*/
    case M2MB_ATP_CMDTYP_SET:
      if( atpParam->itemNum < 1 )
      {
        AZX_LOG_ERROR( "Operation not allowed!\r\n" );
        AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls,  M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
        return;
      }

      azx_easy_at_TrimAndRemoveQuotes( atpParam->item[0] );
      p_param = atpParam->item[0];
      AZX_LOG_INFO( "parameter is <%s>\r\n", p_param );
      /*Release AT instance*/
      AZX_EASY_AT_RELEASE_WITH_SUCCESS( &hdls );
      break;

    /*AT#MYCMD?<CR>*/
    case M2MB_ATP_CMDTYP_READ:
      /*Release AT instance with failure*/
      AZX_EASY_AT_RELEASE_WITH_CMEE( &hdls, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      break;

    /*AT#MYCMD=?<CR>*/
    case M2MB_ATP_CMDTYP_TEST:
      sprintf( rsp_buf, "%s: 64",
               atpParam->atpCmdString );
      m2mb_atp_msgout( atpHandle, atpI, rsp_buf );
      AZX_EASY_AT_RELEASE_WITH_SUCCESS( &hdls );
      break;

    /*AT#MYCMD<CR>*/
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
