#include "mbedtls_hw_interface.h"

/* ========================== function pointers of ECC ========================== */
void (*set_pke_clock)(bool enable) = hw_pke_clock;
void (*init_pke)(bool byte_swap_en, bool word_swap_en, uint32_t word_swap_base) = hw_pke_init;
void (*hw_ecp_init)(uint32_t key_bits, PKE_MODE mode, bool go_to_end_loop,
                    bool RR_mod_n_ready) = hw_ecc_init;
void (*hw_ecp_set_sub_operand)(uint32_t operand_addr, uint32_t *operands,
                               uint32_t byte_len) = hw_ecc_set_sub_operand;
bool (*hw_ecp_set_all_operands)(ECC_GROUP *grp, uint32_t *e,
                                uint32_t e_byte_size) = hw_ecc_set_all_operands;
ERR_CODE(*hw_ecp_compute)(void *result, uint32_t output_addr, uint16_t func_id) = hw_ecc_compute;
/* ============================================================================== */

/* ========================== function pointers of hash ========================= */
void (*hw_sha2_init)(void) = hw_sha256_init;
void (*hw_sha2_start)(HW_SHA256_CTX *ctx, uint32_t *iv) = hw_sha256_start;
bool (*hw_sha2_cpu_update)(HW_SHA256_CTX *ctx, uint8_t *input,
                           uint32_t byte_len) = hw_sha256_cpu_update;
bool (*hw_sha2_finish)(HW_SHA256_CTX *ctx, uint32_t *result) = hw_sha256_finish;
/* ============================================================================== */

/* ========================== function pointers of aes ========================== */
bool (*hw_aes_ecb128_encrypt)(uint8_t *plaintext, const uint8_t *key,
                              uint8_t *ciphertext) = aes128_ecb_encrypt;
bool (*hw_aes_ecb128_decrypt)(uint8_t *ciphertext, const uint8_t *key,
                              uint8_t *plaintext) = aes128_ecb_decrypt;
bool (*hw_aes_ecb256_encrypt)(uint8_t *plaintext, const uint8_t *key,
                              uint8_t *ciphertext) = aes256_ecb_encrypt;
bool (*hw_aes_ecb256_decrypt)(uint8_t *ciphertext, const uint8_t *key,
                              uint8_t *plaintext) = aes256_ecb_decrypt;
/* ============================================================================== */
