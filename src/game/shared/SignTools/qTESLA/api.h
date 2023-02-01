/*************************************************************************************
* qTESLA: an efficient post-quantum signature scheme based on the R-LWE problem
*
* Abstract: API header file
**************************************************************************************/

#include "params.h"
#include <stdint.h>


#if defined(_qTESLA_p_I_)
  #define CRYPTO_ALGNAME "qTesla-p-I"
#elif defined(_qTESLA_p_III_)
  #define CRYPTO_ALGNAME "qTesla-p-III"
#endif

#define CRYPTO_RANDOMBYTES 32
#define CRYPTO_SEEDBYTES 32
#define CRYPTO_C_BYTES 32
#define HM_BYTES 40

// Contains signature (z,c). z is a polynomial bounded by B, c is the output of a hashed string
#define CRYPTO_BYTES ((PARAM_N*(PARAM_B_BITS+1)+7)/8 + CRYPTO_C_BYTES)
// Contains polynomial s and e, and seeds seed_a and seed_y
#define CRYPTO_SECRETKEYBYTES ((PARAM_K+1)*PARAM_S_BITS*PARAM_N/8 + 2*CRYPTO_SEEDBYTES + HM_BYTES)
// Contains seed_a and polynomials t
#define CRYPTO_PUBLICKEYBYTES ((PARAM_K*PARAM_Q_LOG*PARAM_N+7)/8 + CRYPTO_SEEDBYTES)


/// <summary>
/// generates a public and private key pair
/// </summary>
/// <param name="unsigned char* pk">---OUT, public key</param>
/// <param name="unsigned char* sk">---OUT, secret key</param>
/// <returns>0 for successful execution</returns>
int crypto_sign_keypair(unsigned char* pk, unsigned char* sk);

/// <summary>
/// outputs a signature for a given message m
/// </summary>
/// <param name="const unsigned char *m">---IN, message to be signed</param>
/// <param name="unsigned long long mlen">---IN, message length</param>
/// <param name="const unsigned char* sk">---IN, secret key</param>
/// <param name="unsigned long long *smlen">---OUT, signature</param>
/// <param name="unsigned char *sm">---OUT, signature length*</param>
/// <returns>0 for successful execution</returns>
int crypto_sign(unsigned char* sm, unsigned long long* smlen, const unsigned char* m, unsigned long long mlen, const unsigned char* sk);

/// <summary>
/// verification of a signature sm
/// </summary>
/// <param name="const unsigned char *sm">---IN, signature</param>
/// <param name="unsigned long long smlen">---IN, signature length</param>
/// <param name="const unsigned char* pk">---IN, public Key</param>
/// <param name="unsigned char *m">---OUT, original (signed) message</param>
/// <param name="unsigned long long *mlen">---OUT, const unsigned char *sm</param>
/// <returns>0 for valid signature, negative for invalid signature</returns>
int crypto_sign_open(unsigned char* m, unsigned long long* mlen, const unsigned char* sm, unsigned long long smlen, const unsigned char* pk);