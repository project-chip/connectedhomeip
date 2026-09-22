/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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

// Ensure device feature macros (e.g. SEMAILBOX_PRESENT) are available before
// any Silabs security headers that key off them. TF-PSA build_info pulls this
// file in very early — before em_device.h would otherwise be reached.
#include "em_device.h"

// mbedTLS 4 includes tf-psa-crypto/build_info.h before MBEDTLS_CONFIG_FILE.
// The Silabs autogen that enables MBEDTLS_PSA_CRYPTO_C (and related symbols)
// must be visible during that first TF-PSA finalize pass.
#include "sli_mbedtls_config_autogen.h"

// Allow use of legacy mbedtls_* crypto primitives that moved under private/
// headers in mbedTLS 4 / TF-PSA-Crypto (e.g. Spake2p).
#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS

// MATTER AWS Specific Configurations
#ifdef SL_MATTER_ENABLE_AWS
#define PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY
#define PSA_WANT_ALG_CBC_NO_PADDING
#define PSA_WANT_ALG_RSA_PKCS1V15_SIGN
#endif // SL_MATTER_ENABLE_AWS

// Configurations necessary for ot coap cert libs
#if SL_USE_COAP_CONFIG
#define PSA_WANT_ALG_CBC_NO_PADDING
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC
#define PSA_WANT_ALG_JPAKE
#define PSA_WANT_ECC_SECP_R1_256
#endif // SL_USE_COAP_CONFIG

// Multi-chip OTA encryption processing
#ifdef SL_MATTER_ENABLE_OTA_ENCRYPTION
#define PSA_WANT_ALG_CTR
#endif // SL_MATTER_ENABLE_OTA_ENCRYPTION

// Include Generated fies
#include "psa_crypto_config.h"
