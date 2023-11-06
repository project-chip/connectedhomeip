/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <crypto/SessionKeystore.h>

namespace chip {
namespace Crypto {

class RawKeySessionKeystore : public SessionKeystore
{
public:
    CHIP_ERROR CreateKey(const Aes128KeyByteArray & keyMaterial, Aes128KeyHandle & key) override;
    CHIP_ERROR DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                         Aes128KeyHandle & key) override;
    CHIP_ERROR DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info, Aes128KeyHandle & i2rKey,
                                 Aes128KeyHandle & r2iKey, AttestationChallenge & attestationChallenge) override;
    void DestroyKey(Aes128KeyHandle & key) override;
};

} // namespace Crypto
} // namespace chip
