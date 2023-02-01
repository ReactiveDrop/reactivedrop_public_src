This pair of keys is for demonstration only. Secret Key should be kept secret by a moderator.
Public key are fine to direcly hard coded into the source code. there is no need to change key pair unless secret key leaks to the public.
Any change made to the Signed Message or Public Key will result an error in decrypting.
When signing, put the secret key file in the same folder with SignTools_SignMessage.exe, then paste message needs to be signed and copy the signed message to clipboard or file.