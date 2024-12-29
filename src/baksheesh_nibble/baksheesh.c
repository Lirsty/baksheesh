#include "baksheesh.h"
#include "baksheesh_hardcode.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define ROUND 35

/**
 * SBox used for substitution in encryption, derived from the paper 
 * "BAKSHEESH: Similar Yet Different From GIFT". 
 */
const unsigned char SBox[16] = {3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1};
const unsigned char InvSBox[16] = {1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9};

/* Round Constants derived from the paper "BAKSHEESH: Similar Yet Different From GIFT". */
const unsigned char RC[ROUND] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};


/**
 * @brief BAKSHEESH context structure.
 * @note The structure is typically allocated and initialized using 
 *       `BAKSHEESH_CTX_new()` and freed using `BAKSHEESH_CTX_free()`.
 */
struct BAKSHEESH_CTX {
    uint8_t key[32];                    /* <<< The original encryption key.              */
    uint8_t shifted_keys[32 * ROUND];   /* <<< Precomputed shifted keys for each round.  */
};


/**
 * @brief Allocates and initializes a new BAKSHEESH context.
 *
 * This function creates a new instance of the BAKSHEESH context, which is used
 * for encryption and decryption. It initializes the context with the provided key.
 *
 * @param key A pointer to the encryption key (32 bytes). Each byte in the key 
 *            contains only 4 valid bits (a nibble), meaning the values are 
 *            restricted to the range 0x0 to 0xF for each nibble.
 * @return A pointer to the newly created BAKSHEESH context, or NULL if allocation fails.
 */
struct BAKSHEESH_CTX *
BAKSHEESH_CTX_new(const unsigned char *key)
{   
    struct BAKSHEESH_CTX *ctx = malloc(sizeof(struct BAKSHEESH_CTX));
    if (ctx == NULL) return ctx;

    /* Copy the key into the context */
    memcpy(ctx->key, key, 32);

    /* Generate and store the shifted keys for all rounds. */
    uint8_t *p = (unsigned char *) key;
    for (int i=0; i<ROUND; ++i)
    {   
        memcpy(ctx->shifted_keys + (i << 5), p, 32);    /* (i<<5) <-> (i*32) */
        RIGHT_SHIFT_KEY((ctx->shifted_keys + (i << 5)))
        p = ctx->shifted_keys + (i << 5);
    }

    /* Return */
    return ctx;
}


/**
 * @brief Performs the BAKSHEESH encryption algorithm on a single block of data.
 *
 * This function encrypts a 32-byte block (`state`) using the provided
 * BAKSHEESH context (`ctx`). The result is stored in `ret`.
 *
 * @param ctx   A pointer to the BAKSHEESH context.
 * @param state A pointer to the 32-byte input block to be encrypted.
 * @param ret   A pointer to the 32-byte buffer where the encrypted result will be stored.
 */
static inline void 
__baksheesh_encrypt(BAKSHEESH_CTX *ctx, uint8_t *state, uint8_t *ret)
{ 
    uint8_t buf[32];

    /* Whitening */
    for (int i=0; i<32; ++i)
        ret[i] = state[i] ^ ctx->key[i];
    
    /* 35 Rounds */
    for (int r=0; r<ROUND; ++r)
    {
        /* Sbox */
        for (int i=0; i<32; ++i)
            ret[i] = SBox[ret[i]];

        /* PermBits: permute the bits using a hardcoded P-box. */
        memset(buf, 0, 32);
        PBOX_HC(buf, ret) /* hardcoded */ 

        /* AddRoundConstants: XORing a 6-bit constant as well as another bit to the state. */
        ADD_CONSTANT_HC(buf, RC[r]) /* hardcoded */

        /* AddRoundKey: XORing the round key to the state. */
        for (int i=0; i<32; ++i)
            ret[i] = buf[i] ^ (ctx->shifted_keys[(r << 5) + i] & 0xF);
    }
}


/**
 * @brief Performs the BAKSHEESH decryption algorithm on a single block of data.
 *
 * This function decrypts a 32-byte block (`state`) using the provided
 * BAKSHEESH context (`ctx`). The result is stored in `ret`.
 *
 * @param ctx   A pointer to the BAKSHEESH context.
 * @param state A pointer to the 32-byte input block to be decrypted.
 * @param ret   A pointer to the 32-byte buffer where the decrypted result will be stored.
 */
static inline void
__baksheesh_decrypt(BAKSHEESH_CTX *ctx, uint8_t *state, uint8_t *ret)
{
    uint8_t buf[32];
    memcpy(ret, state, 32);

    /* 35 Rounds */ 
    for (int r=ROUND-1; r>=0; --r)
    {
        /* AddRoundKey: XORing the round key to the state. */
        for (int i=0; i<32; ++i)
            ret[i] ^= ctx->shifted_keys[(r << 5) + i] & 0xF;
            
        /* AddRoundConstants: XORing a 6-bit constant as well as another bit to the state. */
        ADD_CONSTANT_HC(ret, RC[r]) /* hardcoded */

        /* PermBits: permute the bits using a hardcoded P-box. */ 
        memset(buf, 0, 32);
        INV_PBOX_HC(buf, ret) /* hardcoded */

        /* SBox */
        for (int i=0; i<32; ++i)
            ret[i] = InvSBox[buf[i]];
    }

    /* Whitening */
    for (int i=0; i<32; ++i)
        ret[i] ^= ctx->key[i];
}


/* To align the given value 'n' to the nearest 32-byte block size. (size <= n) */
#define ALIGN_TO_32_BLOCK_SIZE(n) (n & ~31)
/**
 * @brief Encrypts input data using the BAKSHEESH encryption algorithm.
 *
 * This function encrypts the provided input data using the specified BAKSHEESH
 * context. The output buffer must be large enough to hold the encrypted data.
 *
 * @param ctx A pointer to the initialized BAKSHEESH context.
 * @param input A pointer to the plaintext data to be encrypted.
 * @param input_len The length of the plaintext data in bytes.
 * @param output A pointer to the buffer that will hold the encrypted data.
 * @param output_len A pointer to a variable where the length of the encrypted data will be stored.
 * @return 0 on success, or a non-zero error code on failure.
 */
int BAKSHEESH_CTX_encrypt
(
    struct BAKSHEESH_CTX *ctx,
    const unsigned char *input, size_t input_len,
    unsigned char *output, size_t *output_len
)
{
    if (ctx == NULL || input == NULL || output == NULL) return -1;

    /* Align the input length to the nearest 32-byte block and store it in len.
       This ensures the output length is a multiple of 32.                      */
    size_t len = *output_len = ALIGN_TO_32_BLOCK_SIZE(input_len);
    size_t blocks = len >> 5; /* (len >> 5) <-> (len / 32) */
    uint8_t buf[32];

    /* Loop through each block of data to encrypt. */
    for (size_t done = 0; blocks; done += 32, blocks--)
    {
        __baksheesh_encrypt(ctx, (unsigned char *)input+done, buf);
        memcpy(output+done, buf, 32);
    }

    /* Return success. */
    return 0;
}


/**
 * @brief Decrypts input data using the BAKSHEESH encryption algorithm.
 *
 * This function decrypts the provided input data using the specified BAKSHEESH
 * context. The output buffer must be large enough to hold the decrypted data.
 *
 * @param ctx A pointer to the initialized BAKSHEESH context.
 * @param input A pointer to the ciphertext data to be decrypted.
 * @param input_len The length of the ciphertext data in bytes.
 * @param output A pointer to the buffer that will hold the decrypted data.
 * @param output_len A pointer to a variable where the length of the decrypted data will be stored.
 * @return 0 on success, or a non-zero error code on failure.
 */
int BAKSHEESH_CTX_decrypt
(
    struct BAKSHEESH_CTX *ctx,
    const unsigned char *input, size_t input_len,
    unsigned char *output, size_t *output_len
)
{
    if (ctx == NULL || input == NULL || output == NULL) return -1;

    /* Align the input length to the nearest 32-byte block and store it in len.
       This ensures the output length is a multiple of 32.                      */
    size_t len = *output_len = ALIGN_TO_32_BLOCK_SIZE(input_len);
    size_t blocks = len >> 5;
    uint8_t buf[32];

    /* Loop through each block of data to decrypt. */
    for (size_t done = 0; blocks; done += 32, blocks--)
    {
        __baksheesh_decrypt(ctx, (unsigned char *)input+done, buf);
        memcpy(output+done, buf, 32);
    }

    /* Return success. */
    return 0;
}


/**
 * @brief Frees the memory associated with a BAKSHEESH context.
 * @param ctx A pointer to the BAKSHEESH context to be freed.
 */
void BAKSHEESH_CTX_free(struct BAKSHEESH_CTX *ctx)
{
    if (ctx) free(ctx);
}