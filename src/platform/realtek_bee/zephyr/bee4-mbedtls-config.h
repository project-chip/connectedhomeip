#ifndef BEE4_MBEDTLS_CONFIG_H
#define BEE4_MBEDTLS_CONFIG_H

#define MBEDTLS_CIPHER_MODE_CTR

#define MBEDTLS_ERROR_C
#define MBEDTLS_ERROR_STRERROR_DUMMY
#define MBEDTLS_HKDF_C
#define MBEDTLS_PKCS5_C
#define MBEDTLS_X509_CSR_PARSE_C
#define MBEDTLS_X509_CSR_WRITE_C
#define MBEDTLS_X509_CREATE_C

#define MBEDTLS_ECP_ALT
#include "hw_acc/crypto_engine_nsc.h"

#define ENABLE_HW_RSA_VERIFY    (1)
#define ENABLE_HW_ECC_VERIFY    (1)
#define ENABLE_HW_EDDSA_VERIFY  (1)
#define ENABLE_HW_AES_VERIFY    (1)
#define ENABLE_HW_TRNG          (1)
#define ENABLE_HW_SHA256_VERIFY (1)

#if ENABLE_HW_ECC_VERIFY == 1 || ENABLE_HW_EDDSA_VERIFY == 1 || ENABLE_HW_RSA_VERIFY == 1
#define RTK_PUBKEY_ECC_MUL_ENTRY            0x1
#define RTK_PUBKEY_MOD_MUL_ENTRY            0x2
#define RTK_PUBKEY_MOD_ADD_ENTRY            0x3
#define RTK_PUBKEY_Px_MOD_ENTRY             0x4
#define RTK_PUBKEY_R_SQAR_ENTRY             0x5
#define RTK_PUBKEY_N_INV_ENTRY              0x6
#define RTK_PUBKEY_K_INV_ENTRY              0x7
#define RTK_PUBKEY_ECC_ADD_POINT_ENTRY      0x8
#define RTK_PUBKEY_ECC_POINT_CHECK_ENTRY    0x9
#define RTK_PUBKEY_SET_A_FROM_P_ENTRY       0xa
#define RTK_PUBKEY_K_INV_BIN_ENTRY          0x11
#define RTK_PUBKEY_MOD_MUL_BIN_ENTRY        0xb
#define RTK_PUBKEY_MOD_ADD_BIN_ENTRY        0xc
#define RTK_PUBKEY_MOD_COMP_BIN_ENTRY       0xd
#define RTK_PUBKEY_X_MOD_N_ENTRY            0xe
#define RTK_PUBKEY_MOD_SUB_ENTRY            0xf

#define GO_TO_END_LOOP                      1
#define RR_MOD_N_READY                      1

#define ECC_PRIME_MODE                      0   // 3'b000
#define ECC_BINARY_MODE                     1   // 3'b001
#define RSA_MODE                            2   // 3'b010
#define ECC_EDWARDS_CURVE                   3   // 3'b011
#define ECC_MONTGOMERY_CURVE                7   // 3'b111

#define PKE_MMEM_ADDR                       0x50090000

#define RSA_N_ADDR                          (PKE_MMEM_ADDR + 0)
#define RSA_E_ADDR                          (PKE_MMEM_ADDR + 0x180)
#define RSA_A_ADDR                          (PKE_MMEM_ADDR + 0x300)
#define RSA_RESULT_ADDR                     (PKE_MMEM_ADDR + 0x480)
#define RSA_GEN_KEY_RESULT_ADDR             (PKE_MMEM_ADDR + 0x3CC)

#define ECC_N_ADDR                          (PKE_MMEM_ADDR + 0)
#define ECC_E_ADDR                          (PKE_MMEM_ADDR + 0x40)
#define ECC_CURVE_A_ADDR                    (PKE_MMEM_ADDR + 0x80)
#define ECC_RR_MOD_N_ADDR                   (PKE_MMEM_ADDR + 0xC0)
#define ECC_CURVE_B_ADDR                    (PKE_MMEM_ADDR + 0x100)
#define ECC_X_ADDR                          (PKE_MMEM_ADDR + 0x140)
#define ECC_Y_ADDR                          (PKE_MMEM_ADDR + 0x180)
#define ECC_Z_ADDR                          (PKE_MMEM_ADDR + 0x1C0)
#define ECC_X_RESULT_ADDR                   (PKE_MMEM_ADDR + 0x200)
#define ECC_Y_RESULT_ADDR                   (PKE_MMEM_ADDR + 0x240)
#define ECC_Z_RESULT_ADDR                   (PKE_MMEM_ADDR + 0x280)
#endif

#endif // BEE4_MBEDTLS_CONFIG_H
