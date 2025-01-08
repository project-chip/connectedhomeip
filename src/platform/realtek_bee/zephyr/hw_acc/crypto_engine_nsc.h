#ifndef _CRYPTO_ENGINE_NSC_H_
#define _CRYPTO_ENGINE_NSC_H_

#include <stdint.h>
#include <stdbool.h>

/** @defgroup CRYPTO_ENGINE Crypto Engine
  * @brief
  * @{
  */

/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup CRYPTO_ENGINE_Exported_Types Crypto Engine Exported Types
  * @brief
  * @{
  */
/* public key engine */
typedef enum
{
    ERR_NONE            = 0x0,
    ERR_PRIME           = 0x1,
    ERR_R2_MOD_N        = 0x2,
    ERR_ECC_ODD_POINT   = 0x4,
    ERR_ECC_Z           = 0x8,
    ERR_MODULAR_INV     = 0x10,
    ERR_N_ST_INPUT      = 0x20,
    ERR_NO_VALID_EXP    = 0x40,
    ERR_INVALID_INPUT   = 0x80,
} ERR_CODE;

typedef enum
{
    ECC_PRIME_MODE       = 0,   // 3'b000
    ECC_BINARY_MODE      = 1,   // 3'b001
    RSA_MODE             = 2,   // 3'b010
    ECC_EDWARDS_CURVE    = 3,   // 3'b011
    ECC_MONTGOMERY_CURVE = 7,   // 3'b111
} PKE_MODE;

typedef struct
{
    uint32_t x[16];
    uint32_t y[16];
    uint32_t z[16];
} ECC_POINT;

typedef struct
{
    uint32_t *N;    // modular, also called p
    uint32_t *A;    // curve parameter a
    uint32_t *B;    // curve parameter b
    uint32_t *n;    // order of G
    ECC_POINT G;    // base point

    uint32_t key_bits;
    PKE_MODE mode;

} ECC_GROUP;

/* SHA2-256 */

typedef enum
{
    HW_SHA256_CPU_MODE,
    HW_SHA256_DMA_MODE
} HW_SHA256_ACCESS_MODE;

typedef struct
{
    uint32_t total[2];          /*!< The number of Bytes processed.  */
    uint32_t state[8];          /*!< The intermediate digest state.  */
    uint8_t buffer[64];         /*!< The data block being processed. */
    int is224;                  /*!< unused, just align mbedtls structure */
} HW_SHA256_CTX;
/** End of CRYPTO_ENGINE_Exported_Types
  * @}
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup CRYPTO_ENGINE_Exported_Functions Crypto Engine Exported Functions
  * @brief
  * @{
  */

void hw_pke_clock(bool enable);
void hw_pke_init(bool byte_swap_en, bool word_swap_en, uint32_t word_swap_base);

bool get_mbedtls_rsa_private_key_blinding_en();
void hw_rsa_init(uint32_t key_bits);
void hw_rsa_set_sub_operand(uint32_t operand_addr, uint32_t *operands, uint32_t byte_len);
bool hw_rsa_set_all_operands(uint32_t *M, uint32_t *e, uint32_t *N, uint32_t exp_byte_size);
ERR_CODE hw_rsa_compute(uint32_t *result, uint32_t output_addr, uint16_t func_id);

void hw_ecc_init(uint32_t key_bits, PKE_MODE mode, bool go_to_end_loop, bool RR_mod_n_ready);
void hw_ecc_set_sub_operand(uint32_t operand_addr, uint32_t *operands, uint32_t byte_len);
bool hw_ecc_set_all_operands(ECC_GROUP *grp, uint32_t *e, uint32_t e_byte_size);
ERR_CODE hw_ecc_compute(void *result, uint32_t output_addr, uint16_t func_id);


bool hw_sha256(uint8_t *input, uint32_t byte_len, uint32_t *result, int mode);
void hw_sha256_init(void);
void hw_sha256_start(HW_SHA256_CTX *ctx, uint32_t *iv);
bool hw_sha256_cpu_update(HW_SHA256_CTX *ctx, uint8_t *input, uint32_t byte_len);
bool hw_sha256_finish(HW_SHA256_CTX *ctx, uint32_t *result);

/** End of CRYPTO_ENGINE_Exported_Functions
  * @}
  */

/** End of CRYPTO_ENGINE
  * @}
  */

#endif