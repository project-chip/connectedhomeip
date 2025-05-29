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

// DIC Specific Configurations
#ifdef DIC_ENABLE
#define PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY
#define PSA_WANT_ALG_CBC_NO_PADDING
#define PSA_WANT_ALG_RSA_PKCS1V15_SIGN
#endif // DIC_ENABLE

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
