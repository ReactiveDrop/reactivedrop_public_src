#ifndef PACK_H
#define PACK_H

#include "poly.h"
#include <stdint.h>

void hash_H(unsigned char *c_bin, poly v, const unsigned char *hm);
void encode_sk(unsigned char *sk, const poly s, const poly_k e, const unsigned char *seeds, const unsigned char *hash_pk);
void encode_pk(unsigned char *pk, const poly_k t, const unsigned char *seedA);
void decode_pk(int32_t *pk, unsigned char *seedA, const unsigned char *pk_in);
void encode_sig(unsigned char *sm, unsigned char *c, poly z);
void decode_sig(unsigned char *c, poly z, const unsigned char *sm);

#endif
