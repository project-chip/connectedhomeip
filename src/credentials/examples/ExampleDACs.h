/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once
#include <cstdint>

#include <lib/support/Span.h>
#include <platform/CHIPDeviceConfig.h>

namespace chip {
namespace DevelopmentCerts {

extern ByteSpan kDacCert;
extern ByteSpan kDacPublicKey;
extern ByteSpan kDacPrivateKey;

#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8000
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8000[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8000[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8000[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8001
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8001[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8001[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8001[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8002
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8002[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8002[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8002[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8003
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8003[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8003[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8003[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8004
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8004[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8004[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8004[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8005
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8005[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8005[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8005[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8006
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8006[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8006[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8006[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8007
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8007[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8007[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8007[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8008
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8008[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8008[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8008[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8009
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8009[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8009[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8009[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x800A
extern const uint8_t kDevelopmentDAC_Cert_FFF1_800A[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_800A[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_800A[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x800B
extern const uint8_t kDevelopmentDAC_Cert_FFF1_800B[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_800B[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_800B[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x800C
extern const uint8_t kDevelopmentDAC_Cert_FFF1_800C[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_800C[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_800C[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x800D
extern const uint8_t kDevelopmentDAC_Cert_FFF1_800D[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_800D[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_800D[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x800E
extern const uint8_t kDevelopmentDAC_Cert_FFF1_800E[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_800E[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_800E[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x800F
extern const uint8_t kDevelopmentDAC_Cert_FFF1_800F[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_800F[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_800F[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8010
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8010[491];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8010[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8010[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8011
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8011[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8011[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8011[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8012
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8012[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8012[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8012[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8013
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8013[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8013[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8013[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8014
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8014[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8014[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8014[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8015
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8015[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8015[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8015[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8016
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8016[490];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8016[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8016[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8017
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8017[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8017[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8017[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8018
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8018[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8018[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8018[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x8019
extern const uint8_t kDevelopmentDAC_Cert_FFF1_8019[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_8019[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_8019[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x801A
extern const uint8_t kDevelopmentDAC_Cert_FFF1_801A[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_801A[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_801A[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x801B
extern const uint8_t kDevelopmentDAC_Cert_FFF1_801B[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_801B[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_801B[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x801C
extern const uint8_t kDevelopmentDAC_Cert_FFF1_801C[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_801C[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_801C[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x801D
extern const uint8_t kDevelopmentDAC_Cert_FFF1_801D[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_801D[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_801D[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x801E
extern const uint8_t kDevelopmentDAC_Cert_FFF1_801E[493];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_801E[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_801E[32];
#endif
#if CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == 0x801F
extern const uint8_t kDevelopmentDAC_Cert_FFF1_801F[492];
extern const uint8_t kDevelopmentDAC_PublicKey_FFF1_801F[65];
extern const uint8_t kDevelopmentDAC_PrivateKey_FFF1_801F[32];
#endif
} // namespace DevelopmentCerts
} // namespace chip
