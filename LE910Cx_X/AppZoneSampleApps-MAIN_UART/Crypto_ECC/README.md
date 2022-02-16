
### Crypto Elliptic Curve Cryptography (ECC) example 

Sample application showcasing how to manage Elliptic Curve Cryptography functionalities. Debug prints on **MAIN UART**


**Features**


- How to initialize ECC contexts A (Alice) and B (Bob). Alice is emulating a remote host, from which a public key is known.
- How to generate keypairs for contexts and export public keys
- how to export keyblobs from a context (a keyblob is encrypted with hw specific keys, and can only be used on the module where it was created)
- How to save a keyblob in secured TrustZone. 
- How to reload a keyblob from the TrustZone into an initialized context
- How to sign a message with ECDSA from context B (Bob) and verify it from another context A (Alice) with the signature and public key of Bob.
- How to make Bob and Alice derive a shared session keys using each other's public key.
- How to make Bob and Alice create an AES context with the newly created shared keys, encode data and decode it on the other side


#### Application workflow

**`M2MB_main.c`**

- Create Bob ECC context, create a keypair and export it in a keyblob
- Open a file in secured Trust Zone, then store the keyblob in it.
- Destroy Bob ECC context
- Recreate Bob ECC context, open the file from Trust Zone and read the keyblob. 
- Import the keyblob in Bob context.
- Export Bob public key
- Create Alice ECC context, to simulate an external host. Generate a keypair and export the public key.
- Sign a message with Bob context, generating a signature.
- Use Alice to verify the signed message using Bob's signature and public key
- Derive a shared key for Bob, using Alice's public key
- Create an AES context for Bob
- Import the shared key into the AES context
- Encrypt a message using Bob's AES context.

- Derive a shared key for Alice, using Bob's public key
- Create an AES context for Alice
- Import the shared key into the AES context
- Decrypt the message using Alice's AES context.
- Check the decrypted message and the original one match
- Clear all resources

![](../../pictures/samples/crypto_ecc_bordered.png)

---------------------

