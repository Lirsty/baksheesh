#ifndef BAKSHEESH_H

#include <stdlib.h>

/**
 * @brief Defines the BAKSHEESH context structure.
 *
 * This typedef creates an alias `BAKSHEESH_CTX` for the structure type
 * `struct BAKSHEESH_CTX`. It is used to encapsulate the state and parameters
 * required by the BAKSHEESH encryption and decryption algorithms.
 *
 * The actual definition of `struct BAKSHEESH_CTX` is not provided here, making
 * this a forward declaration. This allows the implementation details of the
 * structure to be hidden from the user, enabling encapsulation and abstraction.
 *
 * @note The structure is typically allocated and initialized using 
 *       `BAKSHEESH_CTX_new()` and freed using `BAKSHEESH_CTX_free()`.
 */
typedef struct BAKSHEESH_CTX BAKSHEESH_CTX;

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
struct BAKSHEESH_CTX *BAKSHEESH_CTX_new(const unsigned char *key);

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
);

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
);

/**
 * @brief Frees the memory associated with a BAKSHEESH context.
 * @param ctx A pointer to the BAKSHEESH context to be freed.
 */
void BAKSHEESH_CTX_free(struct BAKSHEESH_CTX *ctx);

#endif /* BAKSHEESH_H */