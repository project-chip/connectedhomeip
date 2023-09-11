/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
#endif // SL_USE_COAP_CONFIG

// Include Generated fies
#include "psa_crypto_config.h"
