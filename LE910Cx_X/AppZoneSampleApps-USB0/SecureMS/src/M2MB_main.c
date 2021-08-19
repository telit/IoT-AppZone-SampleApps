/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing how to manage secure microservice functionalities. Debug prints on USB0
  @version 
    1.0.1
  @note
    Start of Appzone: Entry point
    User code entry is in function M2MB_main()

  @author


  @date
    11/05/2020
 */

/* Include files ================================================================================*/
#include "stdio.h"
#include "string.h"

#include "m2mb_types.h"
#include "m2mb_os_api.h"
#include "m2mb_crypto.h"
#include "m2mb_secure_ms.h"
#include "m2mb_secure_ms_extend.h"

#include "azx_log.h"
#include "azx_utils.h"
#include "app_cfg.h"

/* Local defines ================================================================================*/
/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/
UINT16 secure_ms_item_id = 33;
UINT16 secure_ms_nontrusted_item_id = 34;

/* Local function prototypes ====================================================================*/
/* Static functions =============================================================================*/

/* hexlen is in bytes , 1 byte is FF*/
static CHAR* hex2str(UINT8* hex, UINT32 hexlen)
{
  UINT32 i;
  CHAR* hexstr = (CHAR*) m2mb_os_calloc(1+2*hexlen);
  for(i = 0; i < hexlen; i++)
  {
    /* this format prepends 0 to hex number
     * in case of 0x02.
     * with %2x , space is used instead of
     * printing 0
     * 0x02 => %2x ->  2
     * 0x02 => 0x%.2x -> 02
     */
    sprintf(hexstr+2*i,"%.2X",*(hex+i));
  }
  hexstr[2*hexlen] ='\0';
  return hexstr;
}

/* Global functions =============================================================================*/


INT32 write_normal(UINT16 id, void *data, UINT32 datalen)
{

  INT32 res;
  M2MB_SECURE_MS_HANDLE secure_ms_Handle;


  /*Open the file in write first. */
  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_WRITE);

  /*If it exists, delete it to rewrite the content*/
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_delete( secure_ms_Handle );
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot reset item in secure TZ\r\n");
    }
    m2mb_secure_ms_close( secure_ms_Handle );
  }
  AZX_LOG_TRACE("Item was already in Secure Data Area, it was removed to create a new one\r\n");

  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_CREATE | M2MB_SECURE_MS_WRITE );
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_write( secure_ms_Handle, (UINT8*)data, datalen );
    if( res == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_INFO("Stored input data in Secure Data Area\r\n");
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_write() failed\r\n");
    }

    m2mb_secure_ms_close(secure_ms_Handle);
  }
  return res;
}

INT32 write_trusted(UINT16 id, void *data, UINT32 datalen)
{

  INT32 res;
  M2MB_SECURE_MS_HANDLE secure_ms_Handle;


  /*Open the file in write first. */
  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_WRITE);

  /*If it exists, delete it to rewrite the content*/
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_delete( secure_ms_Handle );
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot reset item in secure TZ\r\n");
    }
    m2mb_secure_ms_close( secure_ms_Handle );
  }
  AZX_LOG_TRACE("Item was already in Secure Data Area, it was removed to create a new one\r\n");

  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_CREATE | M2MB_SECURE_MS_WRITE | M2MB_SECURE_MS_TRUSTED );
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_write( secure_ms_Handle, (UINT8*)data, datalen );
    if( res == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_INFO("Stored input data in Secure Data Area\r\n");
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_write() failed\r\n");
    }

    m2mb_secure_ms_close(secure_ms_Handle);
  }
  return res;
}



INT32 read_normal(UINT16 id, UINT8 *data, UINT32 *datalen)
{

  INT32 res;
  M2MB_SECURE_MS_HANDLE secure_ms_Handle;
  UINT32 to_read;
  UINT32 act_len;

  //AZX_LOG_DEBUG("Opening file id %d to read %u bytes\r\n", id, to_read);
  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_READ );
  if( res == M2MB_RESULT_SUCCESS )
  {

    res = m2mb_secure_ms_read( secure_ms_Handle, 0, NULL, &act_len );
    if( res == M2MB_RESULT_SUCCESS )
    {
      to_read = act_len;
      AZX_LOG_INFO("Data length in SDA: %u bytes\r\n", to_read);

    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_read() to get data size failed\r\n");
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    /*Align value to 16 bytes*/
    to_read = M2MB_ALIGN( to_read, 16 );
    res = m2mb_secure_ms_read( secure_ms_Handle, to_read, data, &act_len );
    if( res == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_INFO("Securely loaded the data from the SDA\r\n");
      *datalen = act_len;
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_read() failed\r\n");
    }
    m2mb_secure_ms_close(secure_ms_Handle);
  }
  else
  {
    AZX_LOG_ERROR("Could not open SDA to access data!\r\n");
  }
  return res;
}



/*-----------------------------------------------------------------------------------------------*/
INT32 read_trusted(UINT16 id, UINT8 *data, UINT32 *datalen)
{

  INT32 res;
  M2MB_SECURE_MS_HANDLE secure_ms_Handle;
  UINT32 to_read ;
  UINT32 act_len;

  //AZX_LOG_DEBUG("Opening file id %d to read %u bytes\r\n", id, to_read);
  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_READ );
  if( res == M2MB_RESULT_SUCCESS )
  {

    res = m2mb_secure_ms_read( secure_ms_Handle, 0, NULL, &act_len );
    if( res == M2MB_RESULT_SUCCESS )
    {
      to_read = act_len;
      AZX_LOG_INFO("Data length in SDA: %u bytes\r\n", act_len);
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_read() to get data size failed\r\n");
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    /*Align value to 16 bytes*/
    to_read = M2MB_ALIGN( to_read, 16 );
    res = m2mb_secure_ms_read( secure_ms_Handle, to_read, data, &act_len );
    if( res == M2MB_RESULT_SUCCESS )
    {
      *datalen = act_len;
      AZX_LOG_ERROR("Read succeeded for a trusted item! Expected failure\r\n");
      res = M2MB_RESULT_FAIL;
    }
    else
    {
      AZX_LOG_INFO("m2mb_secure_ms_read() failed for trusted item, as expected!\r\n");
      res = M2MB_RESULT_SUCCESS;
    }
    m2mb_secure_ms_close(secure_ms_Handle);
  }
  else
  {
    AZX_LOG_ERROR("Could not open SDA to access data!\r\n");
  }
  return res;
}




INT32 read_rotate( UINT16 id, UINT8 *data, UINT32 *datalen)
{
  (void)datalen;
  INT32 res;
  M2MB_SECURE_MS_HANDLE secure_ms_Handle;
  UINT32 to_read;
  UINT32 act_len;
  M2MB_SECURE_MS_BUFFER bfr;


  //AZX_LOG_DEBUG("Opening file id %d to read %u bytes\r\n", id, to_read);
  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_READ | M2MB_SECURE_MS_WRITE );
  if( res == M2MB_RESULT_SUCCESS )
  {

    res = m2mb_secure_ms_read( secure_ms_Handle, 0, NULL, &act_len );
    if( res == M2MB_RESULT_SUCCESS )
    {
      to_read = act_len;
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_read() to get data size failed\r\n");
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    /* Create a buffer in TZ */
    res =  m2mb_secure_ms_crypto_alloc( to_read, &bfr );
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot allocate SECURE_MS_BUFFER\r\n");
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    /*Load SDA item content in TZ buffer*/
    res =  m2mb_secure_ms_crypto_add_item(secure_ms_Handle, bfr, 0);
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot add secured item data to SECURE_MS_BUFFER\r\n");
      m2mb_secure_ms_crypto_free(bfr);
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }
    /*Rotate content in TZ buffer*/
    res = m2mb_secure_ms_crypto_rotate(bfr, 1);
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot add rotate data in SECURE_MS_BUFFER\r\n");
      m2mb_secure_ms_crypto_free(bfr);
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }


    /*update item in SDA with TZ buffer content*/
    res = m2mb_secure_ms_crypto_write(bfr, 0, act_len, secure_ms_Handle);
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot update data from SECURE_MS_BUFFER to SDA\r\n");
      m2mb_secure_ms_crypto_free(bfr);
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    /*export TZ buffer content in user buffer*/
    res = m2mb_secure_ms_crypto_read(bfr, 0, to_read, data, &act_len);
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot read data from SECURE_MS_BUFFER to user buffer\r\n");
      m2mb_secure_ms_crypto_free(bfr);
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }
    m2mb_secure_ms_crypto_free(bfr);
    m2mb_secure_ms_close(secure_ms_Handle);
  }
  else
  {
    AZX_LOG_ERROR("Could not open secure zone to load data!\r\n");
  }
  return res;

}



INT32 trusted_to_nontrusted(UINT16 id_trusted, UINT16 id_untrusted)
{

  INT32 res;
  M2MB_SECURE_MS_HANDLE secure_ms_Handle = NULL;
  M2MB_SECURE_MS_HANDLE secure_ms_nontrusted_Handle = NULL;
  UINT32 to_read = 0;
  UINT32 act_len;
  M2MB_SECURE_MS_BUFFER bfr = NULL;

  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id_trusted, M2MB_SECURE_MS_READ );
  if( res == M2MB_RESULT_SUCCESS )
  {

    /*Open the non-trusted file in write first. */
    res = m2mb_secure_ms_open( &secure_ms_nontrusted_Handle, M2MB_SYSTEM_FILE_ID, id_untrusted, M2MB_SECURE_MS_WRITE);

    /*If it exists, delete it to rewrite the content*/
    if( res == M2MB_RESULT_SUCCESS )
    {
      res = m2mb_secure_ms_delete( secure_ms_nontrusted_Handle );
      if( res != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("Cannot reset item in secure TZ\r\n");
        goto END;
      }
      m2mb_secure_ms_close( secure_ms_nontrusted_Handle );
      secure_ms_nontrusted_Handle = NULL;
    }
    AZX_LOG_TRACE("Item was already in Secure Data Area, it was removed to create a new one\r\n");

    res = m2mb_secure_ms_open( &secure_ms_nontrusted_Handle, M2MB_SYSTEM_FILE_ID, id_untrusted, M2MB_SECURE_MS_CREATE | M2MB_SECURE_MS_WRITE );
    if( res == M2MB_RESULT_SUCCESS )
    {
      /*Get data size from trusted item*/
      res = m2mb_secure_ms_read( secure_ms_Handle, 0, NULL, &act_len );
      if( res == M2MB_RESULT_SUCCESS )
      {
        to_read = act_len;
      }
      else
      {
        AZX_LOG_ERROR("m2mb_secure_ms_read() to get data size failed\r\n");
        goto END;
      }

      /* Create a buffer in TZ */
      res =  m2mb_secure_ms_crypto_alloc( to_read, &bfr );
      if( res != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("Cannot allocate SECURE_MS_BUFFER\r\n");
        goto END;
      }

      /*Load SDA item content in TZ buffer*/
      res =  m2mb_secure_ms_crypto_add_item(secure_ms_Handle, bfr, 0);
      if( res != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("Cannot add secured item data to SECURE_MS_BUFFER\r\n");
        goto END;
      }

      /*update item in SDA with TZ buffer content*/
      res = m2mb_secure_ms_crypto_write(bfr, 0, act_len, secure_ms_nontrusted_Handle);
      if( res != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_INFO("Cannot store data from SECURE_MS_BUFFER to SDA 'non-trusted', as expected\r\n");
        res = M2MB_RESULT_SUCCESS;
        goto END;
      }
    }
    else
    {
      AZX_LOG_ERROR("Could not open 'non-trusted' item to store data!\r\n");
      goto END;
    }

  }
  else
  {
    AZX_LOG_ERROR("Could not open trusted item to load data!\r\n");
  }

  END:
  if ( secure_ms_Handle)
  {
    m2mb_secure_ms_close(secure_ms_Handle);
  }

  if ( secure_ms_nontrusted_Handle)
  {
    m2mb_secure_ms_close(secure_ms_nontrusted_Handle);
  }

  if ( bfr)
  {
    m2mb_secure_ms_crypto_free(bfr);
  }

  return res;
}




INT32 md_test( UINT16 id, UINT8 *data, UINT32 *datalen)
{
  INT32 res;
  M2MB_SECURE_MS_HANDLE secure_ms_Handle;
  UINT32 to_read;
  UINT32 act_len;
  M2MB_SECURE_MS_BUFFER bfr, out_bfr;


  //AZX_LOG_DEBUG("Opening file id %d to read %u bytes\r\n", id, to_read);
  res = m2mb_secure_ms_open( &secure_ms_Handle, M2MB_SYSTEM_FILE_ID, id, M2MB_SECURE_MS_READ );
  if( res == M2MB_RESULT_SUCCESS )
  {

    res = m2mb_secure_ms_read( secure_ms_Handle, 0, NULL, &act_len );
    if( res == M2MB_RESULT_SUCCESS )
    {
      to_read = act_len;
      AZX_LOG_INFO("Data length in SDA: %u bytes\r\n", to_read);

    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_read() to get data size failed\r\n");
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    res =  m2mb_secure_ms_crypto_alloc( to_read, &bfr );
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot allocate SECURE_MS_BUFFER\r\n");
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    res =  m2mb_secure_ms_crypto_alloc( 32, &out_bfr );
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot allocate SECURE_MS_BUFFER\r\n");
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    res =  m2mb_secure_ms_crypto_add_item(secure_ms_Handle, bfr, 0);
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot add secured item data to SECURE_MS_BUFFER\r\n");
      m2mb_secure_ms_crypto_free(bfr);
      m2mb_secure_ms_close(secure_ms_Handle);
      return res;
    }

    res = m2mb_secure_ms_crypto_md(M2MB_CRYPTO_MD_ALG_MD5, bfr, 0, to_read, out_bfr, 0);
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("cannot compute md\r\n");
    }
    else
    {
      res = m2mb_secure_ms_crypto_read(out_bfr, 0, 16, data, &act_len);
      if( res != M2MB_RESULT_SUCCESS )
      {
        AZX_LOG_ERROR("cannot read md from SECURE_MS_BUFFER\r\n");
      }
      else
      {
        AZX_LOG_TRACE("Read MD5 hash from MS. Len: %d\r\n", act_len);
        *datalen = act_len;
      }

    }
    m2mb_secure_ms_crypto_free(bfr);
    m2mb_secure_ms_crypto_free(out_bfr);
    m2mb_secure_ms_close(secure_ms_Handle);
  }
  else
  {
    AZX_LOG_ERROR("Could not open secure zone to load data!\r\n");
  }
  return res;
}

/***************************************************************************************************
   \User Entry Point of Appzone

   \param [in] Module Id

   \details Main of the appzone user
 **************************************************************************************************/
void M2MB_main( int argc, char **argv )
{
  (void)argc;
  (void)argv;

  INT32 res;
  UINT8 buf[] = "hello world";
  UINT8 read_buf[128] = {0};

  UINT8 cryptokey[] = "AA_THIS_IS_MY_SECRET_KEY_BB";
  UINT8 rotated_cryptokey[64] = {0};

  UINT8 md5_dig[32];
  char *md5_hash_str = NULL;

  /*Rotated*/
  UINT8 expected_md5_digest[] = "8EDAD26E26E1C74C7C02386C1C7F541D";

  /*non-rotated: 2E0DEC985E50C96855E45B47F98EA26F*/

  UINT32 datalen = strlen((const char*) buf);

  azx_sleep_ms(2000);

  m2mb_secure_ms_init();

  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting secure ms demo app. This is v%s built on %s %s.\r\n",
      VERSION, __DATE__, __TIME__);

  AZX_LOG_INFO("Writing data in normal item\r\n");
  res = write_normal(secure_ms_item_id,buf, datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Could not write data\r\n");
    return;
  }

  AZX_LOG_INFO("Reading data from normal item\r\n");
  res = read_normal(secure_ms_item_id, read_buf, &datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Could not read data\r\n");
    return;
  }

  AZX_LOG_INFO("Read %d bytes: <%.*s>\r\n", datalen, datalen, read_buf);

  if(0 == strncmp((const char*)read_buf, (const char*)buf, datalen))
  {
    AZX_LOG_INFO("original and retrieved strings are the same\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Retrieved string is different from original one! <%s>", read_buf);
  }

  AZX_LOG_INFO("\r\nWriting key in normal item\r\n");
  datalen = strlen((const char*) cryptokey);
  res = write_normal(secure_ms_item_id, cryptokey, datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Could not write data\r\n");
    return;
  }
  AZX_LOG_INFO("\r\nRotate data in normal item\r\n");
  res = read_rotate(secure_ms_item_id, rotated_cryptokey, &datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Could not read data\r\n");
    return;
  }

  AZX_LOG_INFO("Original key: %s\r\n"
      "Rotated key:  %s\r\n", cryptokey, rotated_cryptokey);

  AZX_LOG_INFO("\r\nCompute MD5 of data in normal item\r\n");
  res = md_test(secure_ms_item_id, md5_dig, &datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Could not read data\r\n");
    return;
  }

  md5_hash_str = hex2str(md5_dig, datalen);
  AZX_LOG_INFO("MD5: %s\r\n", md5_hash_str);
  if(0 == strcmp((const char *)expected_md5_digest, (const char *)md5_hash_str ))
  {
    AZX_LOG_INFO("hash is the expected one!\r\n");
  }
  else
  {
    AZX_LOG_ERROR("unexpected hash!\r\n");
  }
  m2mb_os_free(md5_hash_str);


  /****************************
   *
   * Trusted object
   *
   ****************************/

  datalen = strlen((const char*) cryptokey);
  AZX_LOG_INFO("\r\nWriting data in trusted item\r\n");
  res = write_trusted(secure_ms_item_id, cryptokey, datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Could not write data in trusted item\r\n");
    return;
  }

  /*Read from trusted object. should fail!*/
  AZX_LOG_INFO("Reading data from trusted item (should fail!)\r\n");
  res = read_trusted(secure_ms_item_id, read_buf, &datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Read trusted succeeded (and should have failed)!\r\n");
    return;
  }

  AZX_LOG_INFO("\r\nRotate data in trusted item\r\n");
  memset(rotated_cryptokey, 0, sizeof(rotated_cryptokey));
  res = read_rotate(secure_ms_item_id, rotated_cryptokey, &datalen);
  if (M2MB_RESULT_SUCCESS == res)
  {
    AZX_LOG_ERROR("Rotate trusted succeeded (and should have failed)\r\n");
    return;
  }

  AZX_LOG_INFO("Original key: %s\r\n"
      "Rotated key:  %s\r\n", cryptokey, rotated_cryptokey);

  AZX_LOG_INFO("\r\nCompute MD5 of data in trusted item\r\n");
  res = md_test(secure_ms_item_id, md5_dig, &datalen);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Could not read data\r\n");
    return;
  }

  md5_hash_str = hex2str(md5_dig, datalen);
  AZX_LOG_INFO("\r\nMD5: %s\r\n", md5_hash_str);
  if(0 == strcmp((const char *)expected_md5_digest, (const char *)md5_hash_str ))
  {
    AZX_LOG_INFO("Hash is the expected one!\r\n");
  }
  else
  {
    AZX_LOG_ERROR("Unexpected hash!\r\n");
  }
  m2mb_os_free(md5_hash_str);



  AZX_LOG_INFO("\r\nTry to pass data from trusted to untrusted through TZ buffers\r\n");
  res = trusted_to_nontrusted(secure_ms_item_id, secure_ms_nontrusted_item_id);
  if (M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("Trusted to nontrusted failed!\r\n");
    return;
  }


}

