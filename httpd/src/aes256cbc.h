/*
* This file is part of smarthomatic, http://www.smarthomatic.org.
* Copyright (c) 2013 Uwe Freese
*
* Original authors:
*    Copyright (c) 2007-2009 Ilya O. Levin, http://www.literatecode.com
*    Other contributors: Hal Finney
*
* Development for smarthomatic by Uwe Freese started by adding a
* function to encrypt and decrypt a byte buffer directly and adding CBC mode.
*
* smarthomatic is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License, or (at your
* option) any later version.
*
* smarthomatic is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with smarthomatic. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AES256_H
#define AES256_H

#include <inttypes.h>

#define AES256_ROUNDS			14
#define AES256_KEYSIZE			256
#define AES256_BLOCKSIZE		128

#define AES256_WORDSIZE			32


#define AES256_KEYBYTES			((AES256_KEYSIZE)/8)
#define AES256_KEYWORDS			((AES256_KEYSIZE)/(AES256_WORDSIZE))
#define AES256_BLOCKBYTES		((AES256_BLOCKSIZE)/8)
#define AES256_BLOCKWORDS		((AES256_BLOCKSIZE)/(AES256_WORDSIZE))

typedef struct aes256_context_t{
	uint8_t init;
	uint8_t key[AES256_KEYBYTES];
	uint8_t iv[AES256_BLOCKBYTES];
	uint8_t enckey[AES256_KEYBYTES];
	uint8_t deckey[AES256_KEYBYTES];
} aes256_context;


void aes256_init(aes256_context *, uint8_t * key , uint8_t * iv);
void aes256_done(aes256_context *);
void aes256_encrypt_ecb(aes256_context *, uint8_t * /* plaintext */);
void aes256_decrypt_ecb(aes256_context *, uint8_t * /* cipertext */);


uint32_t aes256_encrypt_cbc(aes256_context *aes_ctx, uint8_t *aes_key, uint8_t *aes_iv, uint8_t *buffer, uint32_t len); // UF
uint32_t aes256_decrypt_cbc(aes256_context *aes_ctx, uint8_t *aes_key, uint8_t *aes_iv, uint8_t *buffer, uint32_t len); // UF

#endif