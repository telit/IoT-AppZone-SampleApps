/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    M2MB_main.c

  @brief
    The file contains the main user entry point of Appzone

  @details
  
  @description
    Sample application showcasing how to manage Elliptic Curve Cryptography functionalities. Debug prints on AUX UART
  @version 
    1.0.0
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

#include "azx_log.h"
#include "app_cfg.h"

/* Local defines ================================================================================*/
/**************
 * REFERENCE:
 * https://cryptobook.nakov.com/digital-signatures/ecdsa-sign-verify-messages
 * RFC 6979
 */

#define MESSAGELEN  16

/* ECDSA signatures are 2 times longer than the signer's private key for the curve used during the signing process.
 * In this case, private key of SECP256R1 is 256 bit, so signature is (2*256)/8 equal to 64 BYTES
 * The same applies for BRAIN POOL 256, whose private key is 256 bit
 *  */
#define SECP256R1_SIGNATURE_LENGTH  64
#define BP256R1_SIGNATURE_LENGTH  64

#define AES_BLOCKSIZE       16


/* Local typedefs ===============================================================================*/
/* Local statics ================================================================================*/

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

/*We are Bob, Alice is a remote entity whose public key is already known*/
void crypto_bob_and_alice(void)
{
  INT32 res;
  M2MB_CRYPTO_ECC_CONTEXT bob = NULL, alice = NULL;
  M2MB_CRYPTO_AES_CONTEXT bob_aes = NULL, alice_aes = NULL;
  UINT8 *p_keyblob = NULL;
  UINT16 keyblob_length;
  UINT8 bob_signature[SECP256R1_SIGNATURE_LENGTH]={0x00};

  M2MB_CRYPTO_ECC_AFFINE_POINT_T bob_pubkey;

  M2MB_CRYPTO_ECC_AFFINE_POINT_T alice_pubkey;

  UINT8 message[MESSAGELEN]={0x41,0x40,0x94,0x94,0x1e,0x89,0x42,0xa4,0x44,0x55,0x48,0x03,0x5b,0xfa,0xe9,0x43};
  UINT8 aes_secret[AES_BLOCKSIZE] ={0x00};
  UINT8 aes_decrypted[AES_BLOCKSIZE]={0x00};

  UINT8 *bob_shared_keyblob = NULL;
  UINT16 bob_shared_keyblob_length;

  UINT8 *alice_shared_keyblob = NULL;
  UINT16 alice_shared_keyblob_length;


  M2MB_SECURE_MS_HANDLE bob_secure_ms_Handle;
  UINT16 bob_secure_ms_item_id = 33;
  UINT32 act_len;
  
  CHAR *hex_string = NULL;
  

  /***************************
   * BOB INITIALIZATION (local host)
   * */

  res = m2mb_crypto_ecc_init(&bob, M2MB_CRYPTO_ECC_DP_SECP256R1);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_init() failed for Bob!\r\n" );
    goto ECC_FREE;
  }

  res = m2mb_crypto_ecc_keypair_generate(bob);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_keypair_generate() failed\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob's keypair generated\r\n" );


  /* export keyblob from Bob. Since keyblob is null, the API function will return the required size */
  res = m2mb_crypto_ecc_keyblob_export(bob, p_keyblob, &keyblob_length);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_keyblob_export() Stage A (retrieve data length) failed for Bob\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO("Bob's keyblob length is %d \r\n",keyblob_length);

  p_keyblob = (UINT8*)m2mb_os_calloc(keyblob_length*sizeof(UINT8));
  if(!p_keyblob)
  {
    AZX_LOG_ERROR("Cannot allocate keyblob buffer\r\n");
    goto ECC_FREE;
  }

  /* export keyblob for Bob */
  res = m2mb_crypto_ecc_keyblob_export(bob, p_keyblob, &keyblob_length);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_keyblob_export() Stage B (retrieve data) failed for Bob\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob exported the keyblob to be securely stored.\r\n\r\n" );


  m2mb_secure_ms_init();
  
  /*Let Bob save the keyblob in SDA */

  /*Open the file in write first. */
  res = m2mb_secure_ms_open( &bob_secure_ms_Handle, M2MB_SYSTEM_FILE_ID, bob_secure_ms_item_id, M2MB_SECURE_MS_WRITE);

  /*If it exists, delete it to rewrite the content*/
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_delete( bob_secure_ms_Handle );
    if( res != M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_ERROR("Cannot reset item in secure TZ\r\n");
    }
    m2mb_secure_ms_close( bob_secure_ms_Handle );
  }
  AZX_LOG_INFO("Bob already had an item in Secure Data Area, it was removed to create a new one\r\n");

  res = m2mb_secure_ms_open( &bob_secure_ms_Handle, M2MB_SYSTEM_FILE_ID, bob_secure_ms_item_id, M2MB_SECURE_MS_CREATE | M2MB_SECURE_MS_WRITE );
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_write( bob_secure_ms_Handle, p_keyblob, keyblob_length );
    if( res == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_INFO("Bob securely saved the keyblob in Secure Data Area\r\n");
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_write() failed for Bob\r\n");
    }
    AZX_LOG_INFO("Releasing resources\r\n\r\n");
    m2mb_secure_ms_close(bob_secure_ms_Handle);
    m2mb_os_free(p_keyblob);
    p_keyblob = NULL;

  }
  else
  {
    AZX_LOG_ERROR("Bob could not open secure zone to store the keyblob! errno: %d\r\n", m2mb_secure_ms_errno(NULL));
    goto ECC_FREE;
  }

  AZX_LOG_INFO("Close Bob's context...\r\n");

  res = m2mb_crypto_ecc_deinit(bob);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("[2] m2mb_crypto_ecc_deinit() failed for Bob\r\n" );
    return;
  }

  AZX_LOG_INFO("Done. Now Bob context does not exist anymore.\r\n\r\n");

  /***************************
   * BOB REINITIALIZATION FROM SECURE_MS(local host)
   * */

  res = m2mb_crypto_ecc_init(&bob, M2MB_CRYPTO_ECC_DP_SECP256R1);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_init() failed for Bob!\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO("Re-initialize Bob Context and load the keyblob from the secure zone\r\n");

  /*Re-allocate the keyblob buffer with the last size*/
  p_keyblob = (UINT8*)m2mb_os_calloc(keyblob_length*sizeof(UINT8));
  if(!p_keyblob)
  {
    AZX_LOG_ERROR("Cannot allocate keyblob buffer\r\n");
    goto ECC_FREE;
  }

  /*Let Bob load the keyblob from SDA */
  res = m2mb_secure_ms_open( &bob_secure_ms_Handle, M2MB_SYSTEM_FILE_ID, bob_secure_ms_item_id, M2MB_SECURE_MS_READ );
  if( res == M2MB_RESULT_SUCCESS )
  {
    res = m2mb_secure_ms_read( bob_secure_ms_Handle, keyblob_length, p_keyblob, &act_len );
    if( res == M2MB_RESULT_SUCCESS )
    {
      AZX_LOG_INFO("Bob securely loaded the keyblob from the SDA\r\n");
    }
    else
    {
      AZX_LOG_ERROR("m2mb_secure_ms_read() failed for Bob\r\n");
    }
    m2mb_secure_ms_close(bob_secure_ms_Handle);
  }
  else
  {
    AZX_LOG_ERROR("Bob could not open secure zone to load the keyblob!\r\n");
    goto ECC_FREE;
  }

  /* Import Bob's keyblob */
  AZX_LOG_INFO("Import keyblob in Bob's context..\r\n");


  res = m2mb_crypto_ecc_keyblob_import(bob, p_keyblob, keyblob_length);
   if( M2MB_RESULT_SUCCESS != res)
   {
     AZX_LOG_ERROR("[3] m2mb_crypto_ecc_keyblob_import() failed for Bob\r\n" );
     goto ECC_FREE;
   }
  AZX_LOG_INFO("Done. Now export Bob's public key...\r\n");


  /* export Bob's public key */
  memset(&bob_pubkey,0x00,sizeof(bob_pubkey));
  res = m2mb_crypto_ecc_public_key_export(bob, &bob_pubkey);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("[3] m2mb_crypto_ecc_public_key_export() failed for Bob\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob's public key successfully exported\r\n\r\n" );


  /***************************
   * ALICE INITIALIZATION (mimicking a remote host)
   * */


  res = m2mb_crypto_ecc_init(&alice, M2MB_CRYPTO_ECC_DP_SECP256R1);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_init() failed for Alice!\r\n" );
    goto ECC_FREE;
  }

  res = m2mb_crypto_ecc_keypair_generate(alice);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_keypair_generate() failed\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Alice's keypair generated\r\n" );


  /* export Alice's public key */
  memset(&alice_pubkey,0x00,sizeof(alice_pubkey));
  res = m2mb_crypto_ecc_public_key_export(alice, &alice_pubkey);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecc_public_key_export() failed for Alice\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Alice's public key successfully exported\r\n\r\n" );


  /*Let Bob sign a message*/

  res = m2mb_crypto_ecdsa_sign(bob, message, MESSAGELEN, bob_signature, SECP256R1_SIGNATURE_LENGTH);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecdsa_sign() failed for Bob\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob's message signed with ECDSA!\r\n" );



  /*Let Alice verify the message with Bob's public key*/
  /***** verify signed md using Alice *****/
  res = m2mb_crypto_ecdsa_verify(alice, message, MESSAGELEN, &bob_pubkey, bob_signature, SECP256R1_SIGNATURE_LENGTH);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecdsa_verify() failed for Alice\r\n" );
    goto ECC_FREE;
  }

  AZX_LOG_INFO("Alice verified bob's message with his pubkey and signature!\r\n\r\n" );

  AZX_LOG_INFO("-----------------------------------------------------------\r\n");
  AZX_LOG_INFO("Bob and Alice will now exchange a message with AES encrypt \r\n");
  AZX_LOG_INFO("-----------------------------------------------------------\r\n\r\n");

  /*Now Bob and Alice will generate a shared key and exchange some data encrypted with it*/

  /* Let Bob derive a shared key applying Alice's pubkey */
  bob_shared_keyblob = NULL;
  res = m2mb_crypto_ecdh_shared_key_derive(bob, bob_shared_keyblob, &bob_shared_keyblob_length, &alice_pubkey);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecdh_shared_key_derive() Stage A (retrieve data length) failed for Bob\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob retrieved the generated shared key size\r\n" );

  AZX_LOG_INFO("Bob's shared keyblob length is: %d. Allocate the required memory to store it.\r\n",bob_shared_keyblob_length);

  bob_shared_keyblob = (UINT8*) m2mb_os_calloc(bob_shared_keyblob_length*sizeof(UINT8));
  res = m2mb_crypto_ecdh_shared_key_derive(bob, bob_shared_keyblob, &bob_shared_keyblob_length, &alice_pubkey);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecdh_shared_key_derive() Stage B (retrieve data) failed for Bob\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob created a shared key using Alice's public key!\r\n\r\n" );


  res = m2mb_crypto_aes_init(&bob_aes);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_aes_init() failed for Bob's AES context\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob created an AEX context to exchange encrypted data with Alice\r\n" );

  /* the result is an AES256 symmetric KEY to be used in an AES context */
  res = m2mb_crypto_aes_keyblob_import(bob_aes, bob_shared_keyblob, bob_shared_keyblob_length);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_aes_keyblob_import() failed for Bob's AES!\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob's AES context imported the shared keyblob\r\n" );

  res = m2mb_crypto_aes_ecb_encdec(bob_aes, M2MB_CRYPTO_AES_MODE_ENCRYPT, message, aes_secret);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_aes_ecb_encdec() encrypt failed for Bob's AES\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Bob Encrypted the message using AES and the shared key!\r\n" );
  hex_string = hex2str( aes_secret, AES_BLOCKSIZE );
  AZX_LOG_INFO("Encrypted data: \r\n %s \r\n\r\n", hex_string);
  m2mb_os_free(hex_string);



  /* Let Alice derive a shared key applying Bob's pubkey */
  bob_shared_keyblob = NULL;
  res = m2mb_crypto_ecdh_shared_key_derive(alice, alice_shared_keyblob, &alice_shared_keyblob_length, &bob_pubkey);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecdh_shared_key_derive() Stage A (retrieve data length) failed for Alice\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Alice retrieved the generated shared key size\r\n" );

  AZX_LOG_INFO("Alice's shared keyblob length is: %d. Allocate the required memory to store it.\r\n",alice_shared_keyblob_length);

  alice_shared_keyblob = (UINT8*) m2mb_os_calloc(alice_shared_keyblob_length*sizeof(UINT8));
  res = m2mb_crypto_ecdh_shared_key_derive(alice, alice_shared_keyblob, &alice_shared_keyblob_length, &bob_pubkey);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_ecdh_shared_key_derive() Stage B (retrieve data) failed for Alice\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Alice created a shared key using Bob's public key!\r\n\r\n" );

  /* the result is an AES256 symmetric KEY to be used in an AES context */

  res = m2mb_crypto_aes_init(&alice_aes);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_aes_init() failed for Alice's AES context\r\n" );
    goto ECC_FREE;
  }

  AZX_LOG_INFO( "Alice created an AEX context to exchange encrypted data with Bob\r\n" );


  res = m2mb_crypto_aes_keyblob_import(alice_aes, alice_shared_keyblob, alice_shared_keyblob_length);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_aes_keyblob_import() failed for Alice's AES!\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Alice's AES context imported the shared keyblob\r\n" );

  res = m2mb_crypto_aes_ecb_encdec(alice_aes, M2MB_CRYPTO_AES_MODE_DECRYPT, aes_secret, aes_decrypted);
  if( M2MB_RESULT_SUCCESS != res)
  {
    AZX_LOG_ERROR("m2mb_crypto_aes_ecb_encdec() decrypt failed for Alice's AES\r\n" );
    goto ECC_FREE;
  }
  AZX_LOG_INFO( "Alice decrypted the message using AES and the shared key!\r\n" );
  
  hex_string = hex2str(aes_decrypted, AES_BLOCKSIZE);
  AZX_LOG_INFO("Decrypted: \r\n %s \r\n\r\n", hex_string);
  m2mb_os_free(hex_string);
  
  hex_string = hex2str(message, AES_BLOCKSIZE);
  AZX_LOG_INFO("Original, plain message: \r\n %s \r\n\r\n", hex_string);
  m2mb_os_free(hex_string);
  
  res = memcmp(aes_decrypted, message, AES_BLOCKSIZE * sizeof(UINT8));
  if( res == 0)
  {
    AZX_LOG_INFO( "Plain and decrypted messages match!\r\n" );
  }
  else
  {
    AZX_LOG_ERROR("Plain and decrypted messages do NOT match!\r\n" );
  }


  ECC_FREE:

  if( bob_shared_keyblob != NULL)
  {
    m2mb_os_free(bob_shared_keyblob);
  }
  if( alice_shared_keyblob != NULL)
  {
    m2mb_os_free(alice_shared_keyblob);
  }

  if( p_keyblob != NULL)
  {
    m2mb_os_free(p_keyblob);
  }
  if (bob != NULL)
  {
    res = m2mb_crypto_ecc_deinit(bob);
    if( M2MB_RESULT_SUCCESS != res)
    {
      AZX_LOG_ERROR("[2] m2mb_crypto_ecc_deinit() failed for Bob\r\n" );
      return;
    }
  }
  
  if (alice != NULL)
  {
    res = m2mb_crypto_ecc_deinit(alice);
    if( M2MB_RESULT_SUCCESS != res)
    {
      AZX_LOG_ERROR("[2] m2mb_crypto_ecc_deinit() failed for Alice\r\n" );
      return;
    }
  }

  if (bob_aes != NULL)
  {
    res = m2mb_crypto_aes_deinit(bob_aes);
    if( M2MB_RESULT_SUCCESS != res)
    {
      AZX_LOG_ERROR("[3] m2mb_crypto_aes_deinit() failed for Bob's AES\r\n" );
      return;
    }
  }
  
  if (alice_aes != NULL)
  {
    res = m2mb_crypto_aes_deinit(alice_aes);
    if( M2MB_RESULT_SUCCESS != res)
    {
      AZX_LOG_ERROR("[3] m2mb_crypto_aes_deinit() failed for Alice's AES\r\n" );
      return;
    }
  }
}
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


  /*SET output channel */
  AZX_LOG_INIT();
  AZX_LOG_INFO("Starting Crypto ECC demo app. This is v%s built on %s %s.\r\n",
        VERSION, __DATE__, __TIME__);
		
  AZX_LOG_INFO( "\r\nBob (local) and Alice (remote) scenario\r\n" );
  crypto_bob_and_alice();

}

