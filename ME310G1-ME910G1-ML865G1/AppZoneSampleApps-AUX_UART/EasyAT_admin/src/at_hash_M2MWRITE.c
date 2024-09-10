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


#include "at_common.h"





void M2MWRITE_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI )
{
M2MB_ATP_PARAM_T *atpParam;
m2mb_atp_get_input_data( atpHandle, atpI, &atpParam );
M2MB_RESULT_E res;


  if( NULL == atpHandle )
  {
    AZX_LOG_ERROR( "atpHandle is NULL\r\n" );
  }

  switch( atpParam->type )
  {
    /*AT#M2MWRITE=<param><CR>*/
    case M2MB_ATP_CMDTYP_SET:
      res = m2mb_atp_release( atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      if (res != M2MB_RESULT_SUCCESS)
      {
       AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
      }
      break;

    /*AT#M2MWRITE?<CR>*/
    case M2MB_ATP_CMDTYP_READ:
      /*Release AT instance with failure*/
      res = m2mb_atp_release( atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      if (res != M2MB_RESULT_SUCCESS)
      {
       AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
      }
      break;

    /*AT#M2MWRITE=?<CR>*/
    case M2MB_ATP_CMDTYP_TEST:
      AZX_LOG_ERROR( "Operation not allowed!\r\n" );
      res = m2mb_atp_release( atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      if (res != M2MB_RESULT_SUCCESS)
      {
       AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
      }
      break;

    /*AT#M2MWRITE<CR>*/
    case M2MB_ATP_CMDTYP_EXEC:
      /*Release AT instance with failure*/
      res = m2mb_atp_release( atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      if (res != M2MB_RESULT_SUCCESS)
      {
       AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
      }
      break;

    /*AT#M2MWRITE=<CR>*/
    case M2MB_ATP_CMDTYP_NOPARAMS:
      /*Release AT instance with failure*/
      res = m2mb_atp_release( atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      if (res != M2MB_RESULT_SUCCESS)
      {
       AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
      }
      break;

    default:
      /*Release AT instance with failure*/
      res = m2mb_atp_release( atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      if (res != M2MB_RESULT_SUCCESS)
      {
       AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
      }
      break;
  } // end switch
}


