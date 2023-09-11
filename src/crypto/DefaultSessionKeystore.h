/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if CHIP_HAVE_CONFIG_H
#include <crypto/CryptoBuildConfig.h>
#endif

#if CHIP_CRYPTO_PSA
#include <crypto/PSASessionKeystore.h>
#else
#include <crypto/RawKeySessionKeystore.h>
#endif

namespace chip {
namespace Crypto {

// Define DefaultSessionKeystore type alias to reduce boilerplate code related to the fact that
// when the PSA crypto backend is used, AES encryption/decryption function assume that the input
// key handle carries a key reference instead of raw key material, so PSASessionKeystore must be
// used instead of RawKeySessionKeystore to initialize the key handle.
#if CHIP_CRYPTO_PSA
using DefaultSessionKeystore = PSASessionKeystore;
#else
using DefaultSessionKeystore = RawKeySessionKeystore;
#endif

} // namespace Crypto
} // namespace chip
