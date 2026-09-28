/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

// Allow use of legacy mbedtls_* crypto primitives under mbedtls/private/.
#define MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS

// mbedTLS 4 includes tf-psa-crypto/build_info.h before MBEDTLS_CONFIG_FILE.
// Pull Silabs mbedtls autogen so TF-PSA finalize (and tinycrypt builtin
// modules) see MBEDTLS_*_C / MBEDTLS_PLATFORM_*. PSA_CRYPTO_C remains
// disabled for tinycrypt via the guard inside the autogen header.
#include "sli_mbedtls_config_autogen.h"

// MATTER AWS Specific Configurations
#ifdef SL_MATTER_ENABLE_AWS
#define MBEDTLS_PSA_BUILTIN_ALG_TLS12_PRF

#define PSA_WANT_ALG_CBC_NO_PADDING
#define PSA_WANT_ALG_RSA_PKCS1V15_SIGN
#define PSA_WANT_ALG_SOME_PAKE
#define PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY
#endif // SL_MATTER_ENABLE_AWS

// SLC GENERATED
#include "psa_crypto_config.h"
