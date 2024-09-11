/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    at_hash_M2MWRITE.c

  @brief
    The file contains the implementation of custom command AT#M2MWRITE

  @author


  @date
    13/03/2020
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "m2mb_secure_ms.h"

#include "m2mb_socket.h"
#include "m2mb_crypto.h"

#include "m2mb_atp.h"

#include "azx_log.h"


#include "at_common.h"

#include "at_hash_M2MWRITE.h" /*For M2MWRITE_AT_Callback declaration*/
//

#define PASSWORD_SHA256 "89E01536AC207279409D4DE1E5253E01F4A1769E696DB0D6062CA9B8F56767C8"  //"mypassword" - if it has been changed the one saved in trust zone will be used
#define SHA256_LEN 32

#define PW_BLOB_ID 35000 /* random id for password*/

extern HANDLE   appAtptaskHandle;


typedef enum
{
  OPERATION_UNLOCK=0,
  OPERATION_LOCK,
  OPERATION_NEWPW,
  OPERATION_MAXVAL
} OPERATION_CODE_E;


static BOOLEAN s_M2MWRITE_ENABLED = FALSE;


static char s_PW_HASH[ (SHA256_LEN * 2) + 1 ] = {0};

static void init_pw_buf(void)
{

  /*if PW buffer is empty, initialize it*/
  if (strlen(s_PW_HASH) == 0)
  {

    INT32 res;
    BOOLEAN returnStatus = TRUE;

    strncpy(s_PW_HASH, PASSWORD_SHA256, (SHA256_LEN * 2));
    s_PW_HASH[(SHA256_LEN * 2)] = '\0';


    M2MB_SECURE_MS_HANDLE key_secure_ms_Handle;
    UINT8 *p_keyblob = NULL;
    UINT16 keyblob_length = 0;


    /* store the key into secure_ms */
    m2mb_secure_ms_init();

    res = m2mb_secure_ms_open( &key_secure_ms_Handle,
            M2MB_SYSTEM_FILE_ID, PW_BLOB_ID,
            M2MB_SECURE_MS_READ);
    AZX_LOG_INFO("Read form secure item res: %d\r\n", res);
    /*If it exists, read the content*/
    if( res == M2MB_RESULT_SUCCESS )
    {

      UINT32 act_len, max_len;

      AZX_LOG_INFO("Read form secure item\r\n");
      // get the complete byte size of item
      res = m2mb_secure_ms_read( key_secure_ms_Handle, 0, NULL, &act_len );
      if( res == M2MB_RESULT_SUCCESS )
      {
        // max_len must be a product of 16. select the immediate superior value
        max_len = M2MB_ALIGN( act_len, 16 );
        p_keyblob = (UINT8 *)m2mb_os_calloc( max_len );
      }

      if( !p_keyblob )
      {
        returnStatus = FALSE;
      }


      if(returnStatus)
      {
        res = m2mb_secure_ms_read( key_secure_ms_Handle, max_len, p_keyblob, &act_len );
        if( res == M2MB_RESULT_SUCCESS )
        {
          keyblob_length = act_len;
          LogDebug(("Securely loaded %u bytes of the keyblob from the SDA\r\n", keyblob_length));
        }
        else
        {
          LogError(("m2mb_secure_ms_read() failed \r\n"));
          returnStatus = TRUE;
        }
      }

      if(returnStatus)
      {
        memset(s_PW_HASH, 0, sizeof(s_PW_HASH));
        strncpy(s_PW_HASH, (const char *)p_keyblob, M2MB_MIN( (SHA256_LEN * 2), keyblob_length) );
      }

    }

  }
}


static BOOLEAN store_pw(char *new_pw)
{
  M2MB_SECURE_MS_HANDLE key_secure_ms_Handle;
  INT32 res;
  BOOLEAN resultStatus = FALSE;

  if(!new_pw)
  {
    return FALSE;
  }

  /* store the key into secure_ms */

  /*Open the file in write mode first. */
  res = m2mb_secure_ms_open( &key_secure_ms_Handle,
          M2MB_SYSTEM_FILE_ID, PW_BLOB_ID,
          M2MB_SECURE_MS_WRITE);

  /*If it exists, delete it to rewrite the content*/
  if( res == M2MB_RESULT_SUCCESS )
  {
    m2mb_secure_ms_close( key_secure_ms_Handle );
  }

  res = m2mb_secure_ms_open( &key_secure_ms_Handle,
          M2MB_SYSTEM_FILE_ID, PW_BLOB_ID,
          M2MB_SECURE_MS_CREATE | M2MB_SECURE_MS_WRITE );
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_write( key_secure_ms_Handle, (UINT8 *) new_pw, strlen(new_pw) );
    if( res == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_INFO("Securely saved the new password in Secure Data Area\r\n");
      resultStatus = TRUE;
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_write() failed for key\r\n");
    }

    AZX_LOG_TRACE("Releasing resources\r\n");
    m2mb_secure_ms_close(key_secure_ms_Handle);

  }
  else
  {
    AZX_LOG_ERROR("Could not open secure zone to store the key! errno: %d\r\n",
            m2mb_secure_ms_errno(NULL));
  }

  return resultStatus;
}


char *bin2hexstr_static(char *hexstr, uint8_t* hex, uint32_t hexlen)
{
    uint32_t i;
    int offset = 0;
    for(i = 0; i < hexlen; i++)
    {
        sprintf(hexstr + offset,"%.2X", *(hex + i));
        offset +=2;
    }
    hexstr[offset] ='\0';

    return hexstr;
}


void M2MADMIN_AT_Callback( M2MB_ATP_HANDLE atpHandle, UINT16 atpI )
{
  M2MB_ATP_PARAM_T *atpParam;
  char hash_buf[(SHA256_LEN * 2) + 1] = {0};
  UINT32 operation = OPERATION_UNLOCK;
  UINT8 *p_hash = NULL;
  char *p_param = NULL;
  M2MB_RESULT_E res;

  m2mb_atp_get_input_data( atpHandle, atpI, &atpParam );

  if( NULL == atpHandle )
  {
    AZX_LOG_ERROR( "atpHandle is NULL\r\n" );
  }

  //AZX_EASY_AT_CMD_INFO( &hdls );
  AZX_LOG_DEBUG( "admin callback\r\n");

  init_pw_buf();

  switch( atpParam->type )
  {
    /*AT#M2MADMIN=<param><CR>*/
    case M2MB_ATP_CMDTYP_SET:
      if( atpParam->itemNum < 1 )
      {
        AZX_LOG_ERROR( "Operation not allowed!\r\n" );
        res = m2mb_atp_release( atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
        if (res != M2MB_RESULT_SUCCESS)
        {
         AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
        }
      }

      TrimAndRemoveQuotes( atpParam->item[0] );
      p_param = atpParam->item[0];
      AZX_LOG_INFO( "parameter is <%s>\r\n", p_param );

      if( atpParam->itemNum > 1 )
      {
        if(strToUL(atpParam->item[1], &operation) < 0)
        {
          AZX_LOG_ERROR("%s undefined value\r\n", p_param);
          res = m2mb_atp_release( atpHandle,atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
          if (res != M2MB_RESULT_SUCCESS)
          {
           AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
          }
          break;
        }
        else
        {

          if (operation >= OPERATION_MAXVAL)
          {
            AZX_LOG_ERROR("Operation is out of range %u\r\n", operation);
            res = m2mb_atp_release( atpHandle,atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
            if (res != M2MB_RESULT_SUCCESS)
            {
             AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
            }
            break;
          }

          AZX_LOG_INFO( "Operation is <%u>\r\n", operation );
        }
      }



      p_hash = m2mb_crypto_md(M2MB_CRYPTO_MD_ALG_SHA256, (const UINT8*) p_param, strlen(p_param), NULL);
      if (!p_hash)
      {
        AZX_LOG_ERROR("cannot compute hash\r\n");
        /*Release AT instance with failure*/
        res = m2mb_atp_release( atpHandle,atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
         if (res != M2MB_RESULT_SUCCESS)
         {
          AZX_LOG_ERROR( "Failed sending the CMEE message. res: %d\r\n", res);
         }
        break;
      }
      else
      {
        bin2hexstr_static(hash_buf, p_hash, SHA256_LEN);
        AZX_LOG_INFO("hash: <%s>\r\n", hash_buf);
        //if(strcmp(hash_buf, PASSWORD_SHA256) == 0)
        if(strcmp(hash_buf, s_PW_HASH) == 0)
        {
          AZX_LOG_INFO("right password!\r\n");

          switch(operation)
          {
            case OPERATION_UNLOCK:
             /*Release AT command*/
             if(s_M2MWRITE_ENABLED == FALSE)
             {
               if(M2MB_RESULT_SUCCESS == m2mb_atp_deregister( atpHandle, (char*)"#M2MWRITE" ))
               {
                 s_M2MWRITE_ENABLED = TRUE;
                 if((res = m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_OK, -1, NULL)) == M2MB_RESULT_SUCCESS)
                 {
                   AZX_LOG_DEBUG("Release SUCCESS was OK\r\n");\
                 }
                 else
                 {
                   AZX_LOG_ERROR("Cannot release! error: %d\r\n", res);
                 }
               }
               else
               {
                 AZX_LOG_ERROR("Failed to deregister command\r\n");
                 /*Release AT instance with failure*/
                 m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
               }
             }
             else
             {
               if((res = m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_OK, -1, NULL)) == M2MB_RESULT_SUCCESS)
               {
                 AZX_LOG_DEBUG("Release SUCCESS was OK\r\n");\
               }
               else
               {
                 AZX_LOG_ERROR("Cannot release! error: %d\r\n", res);
               }
             }
             break;

            case OPERATION_LOCK:
              /*Lock AT command*/
              if(s_M2MWRITE_ENABLED == TRUE)
              {

                EASY_AT_TASK_HANDLE_T *tmp_easy_at_taskHandle = ( EASY_AT_TASK_HANDLE_T * )appAtptaskHandle;
                EASY_AT_TASK_USERDATA_T *easy_at_taskUserdata = ( EASY_AT_TASK_USERDATA_T * )m2mb_os_malloc( sizeof( EASY_AT_TASK_USERDATA_T ) );

                if( easy_at_taskUserdata == NULL )
                {
                  return;
                }

                easy_at_taskUserdata->easy_at_taskHandle =
                tmp_easy_at_taskHandle->easy_at_taskHandle;
                easy_at_taskUserdata->easy_at_taskQueue = tmp_easy_at_taskHandle->easy_at_taskQueue;
                easy_at_taskUserdata->taskCallback = M2MWRITE_AT_Callback;
                easy_at_taskUserdata->taskDelegation = NULL;
                easy_at_taskUserdata->module = get_at_module();

                if( M2MB_RESULT_SUCCESS == m2mb_atp_register( atpHandle, ( CHAR * )"#M2MWRITE", M2MB_ATP_NORML | M2MB_ATP_NOPIN | M2MB_ATP_NOSIM, _easy_at_task_group_callback,
                                           ( void * )easy_at_taskUserdata ))
                {
                  s_M2MWRITE_ENABLED = FALSE;
                  if((res = m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_OK, -1, NULL)) == M2MB_RESULT_SUCCESS)
                  {
                    AZX_LOG_DEBUG("Release SUCCESS was OK\r\n");\
                  }
                  else
                  {
                    AZX_LOG_ERROR("Cannot release! error: %d\r\n", res);
                  }
                }
                else
                {
                  AZX_LOG_ERROR("Failed to register command\r\n");
                  /*Release AT instance with failure*/
                  m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
                }
              }
              else
              {
                if((res = m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_OK, -1, NULL)) == M2MB_RESULT_SUCCESS)
                {
                  AZX_LOG_DEBUG("Release SUCCESS was OK\r\n");\
                }
                else
                {
                  AZX_LOG_ERROR("Cannot release! error: %d\r\n", res);
                }
              }
              break;

            case OPERATION_NEWPW:
              if( atpParam->itemNum < 3 )
              {
                AZX_LOG_ERROR("missing new password");
                /*Release AT instance with failure*/
                m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );

              }
              else
              {
                char * p_password = atpParam->item[2];
                TrimAndRemoveQuotes( p_password);

                /*compute password hash*/
                p_hash = m2mb_crypto_md(M2MB_CRYPTO_MD_ALG_SHA256, (const UINT8*) p_password, strlen(p_password), NULL);
                if (!p_hash)
                {
                  AZX_LOG_ERROR("cannot compute hash of new password\r\n");
                  /*Release AT instance with failure*/
                  m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
                  break;
                }
                else
                {
                  bin2hexstr_static(hash_buf, p_hash, SHA256_LEN);
                  AZX_LOG_DEBUG("Storing new password <%s> hash: <%s>", p_password, hash_buf);
                  if (TRUE != store_pw(hash_buf))
                  {
                    AZX_LOG_ERROR("cannot store password in trustzone\r\n");
                    m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
                    break;
                  }
                  else
                  {
                    /* store the new password in static variable */
                    strncpy(s_PW_HASH, hash_buf, (SHA256_LEN * 2));
                    s_PW_HASH[(SHA256_LEN * 2)] = '\0';
                    if((res = m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_OK, -1, NULL)) == M2MB_RESULT_SUCCESS)
                     {
                       AZX_LOG_DEBUG("Release SUCCESS was OK\r\n");\
                     }
                     else
                     {
                       AZX_LOG_ERROR("Cannot release! error: %d\r\n", res);
                     }
                  }
                }
              }
              break;

              default:
                AZX_LOG_ERROR("Unexpected operation %u\r\n", operation);
                /*Release AT instance with failure*/
                m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
                break;

          }

        }
        else
        {
          AZX_LOG_ERROR("wrong password\r\n");
          /*Release AT instance with failure*/
          m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_INCORRECT_PASSWORD, NULL );
        }

      }

      break;

    /*AT#M2MADMIN?<CR>*/
    case M2MB_ATP_CMDTYP_READ:
      /*Release AT instance with failure*/
      m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      break;

    /*AT#M2MWRITE=?<CR>*/
    case M2MB_ATP_CMDTYP_TEST:
      res = m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_OK, -1, NULL);
      //m2mb_atp_release(atpHandle,atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      break;

    /*AT#M2MADMIN<CR>*/
    case M2MB_ATP_CMDTYP_EXEC:
      /*Release AT instance with failure*/
      m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      break;

    /*AT#M2MADMIN=<CR>*/
    case M2MB_ATP_CMDTYP_NOPARAMS:
      /*Release AT instance with failure*/
      m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      break;

    default:
      /*Release AT instance with failure*/
      m2mb_atp_release(atpHandle, atpI, M2MB_ATP_FRC_CME_ERROR, M2MB_ATP_CME_OPERATION_NOT_ALLOWED, NULL );
      break;
  } // end switch
}
