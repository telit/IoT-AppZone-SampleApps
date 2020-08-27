
###Secure MicroService 

Sample application showcasing how to manage secure microservice functionalities. Debug prints on **MAIN UART**


**Features**


- Write data in Secure Data Area (SDA), non protected
- Read the written data and compare with the original buffer
- Write a cripty key in Secure Data Area (SDA), non protected
- Perform a rotate of the written key data
- Perform MD5 sum of written data from TZ file
- Compare computed digest with expected one
- Write data in trust zone as a trusted object (it will not be possible to read it again but only use its content for crypto operations)
- Try to read the trusted object and verify it fails
- Rotate trusted item and verify retrieving the content fails
- compute MD5 sum of trusted item and compare with the expected one
- Try to pass data from a trusted item to a non trusted item using untrusted TZ buffers, and verify it fails


**Application workflow**

**`M2MB_main.c`**

- Write a buffer in a SDA item using `m2mb_secure_ms_write`
- Read the same item using `m2mb_secure_ms_read`
- Write a buffer containing some cripty key in a SDA item using `m2mb_secure_ms_write`
- Rotate the content of the key item
- Read it with `m2mb_secure_ms_read`
- Load the key content using `m2mb_secure_ms_crypto_alloc` and `m2mb_secure_crypto_add_item` in a SECURE_MS buffer
- Compute MD digest with `m2mb_secure_ms_crypto_md`
- Write a buffer containing some cripty key in a SDA item using `m2mb_secure_ms_write` but with **TRUSTED** option in `m2mb_secure_ms_open`
- Verify that `m2mb_secure_ms_read` on the trusted item fails
- Verify that `m2mb_secure_ms_crypto_rotate` fails for the trusted item
- Verify the MD5 digest
- Try to copy the trusted item data in a SECURE_MS buffer with `m2mb_secure_ms_crypto_alloc` and `m2mb_secure_crypto_add_item`, then load it in an untrusted object with `m2mb_secure_ms_crypto_write`, and verify it fails.



![](../../pictures/samples/secure_ms_bordered.png)

---------------------

