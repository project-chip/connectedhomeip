/*
 *    Copyright (c) 2022 Project CHIP Authors
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
