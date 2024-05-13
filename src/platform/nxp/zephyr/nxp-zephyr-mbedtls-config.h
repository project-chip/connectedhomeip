/*
 *
 *    Copyright (c) 2024 NXP
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

#if CONFIG_CHIP_CRYPTO_PSA
#define MBEDTLS_PSA_CRYPTO_DRIVERS
#endif /* CONFIG_CHIP_CRYPTO_PSA */

#if CONFIG_MCUX_ELS_PKC

#define MBEDTLS_CIPHER_PADDING_PKCS7

#if CONFIG_WIFI_NXP && CONFIG_WPA_SUPP
#include "wpa_supp_els_pkc_mbedtls_config.h"
#endif /* CONFIG_WIFI_NXP && CONFIG_WPA_SUPP */

#if CONFIG_MCUX_PSA_CRYPTO_DRIVER_ELS_PKC
#define PSA_CRYPTO_DRIVER_ELS_PKC
#endif /* CONFIG_MCUX_PSA_CRYPTO_DRIVER_ELS_PKC */

#endif /* CONFIG_MCUX_ELS_PKC */

#undef MBEDTLS_MD4_C
#undef MBEDTLS_ARC4_C
