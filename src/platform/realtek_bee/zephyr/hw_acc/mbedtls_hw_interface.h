/**
 * Copyright (c) 2015, Realsil Semiconductor Corporation. All rights reserved.
 */

#ifndef _MBEDTLS_HW_INTERFACE_H_
#define _MBEDTLS_HW_INTERFACE_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "crypto_engine_nsc.h"
#include "aes_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ Patch of RSA and ECC ============================ */
extern void (*set_pke_clock)(bool enable);
extern void (*init_pke)(bool byte_swap_en, bool word_swap_en, uint32_t word_swap_base);
extern void (*hw_ecp_init)(uint32_t key_bits, PKE_MODE mode, bool go_to_end_loop,
                           bool RR_mod_n_ready);
extern void (*hw_ecp_set_sub_operand)(uint32_t operand_addr, uint32_t *operands, uint32_t byte_len);
extern bool (*hw_ecp_set_all_operands)(ECC_GROUP *grp, uint32_t *e, uint32_t e_byte_size);
extern ERR_CODE(*hw_ecp_compute)(void *result, uint32_t output_addr, uint16_t func_id);
/* ============================================================================== */

/* ========================== function pointers of hash ========================= */
extern void (*hw_sha2_init)(void);
extern void (*hw_sha2_start)(HW_SHA256_CTX *ctx, uint32_t *iv);
extern bool (*hw_sha2_cpu_update)(HW_SHA256_CTX *ctx, uint8_t *input, uint32_t byte_len);
extern bool (*hw_sha2_finish)(HW_SHA256_CTX *ctx, uint32_t *result);
/* ============================================================================== */

/* ========================== function pointers of aes ========================== */
extern bool (*hw_aes_ecb128_encrypt)(uint8_t *plaintext, const uint8_t *key, uint8_t *ciphertext);
extern bool (*hw_aes_ecb128_decrypt)(uint8_t *ciphertext, const uint8_t *key, uint8_t *plaintext);
extern bool (*hw_aes_ecb256_encrypt)(uint8_t *plaintext, const uint8_t *key, uint8_t *ciphertext);
extern bool (*hw_aes_ecb256_decrypt)(uint8_t *ciphertext, const uint8_t *key, uint8_t *plaintext);
/* ============================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* _MBEDTLS_HW_INTERFACE_H_ */
