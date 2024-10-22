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

#include <app/icd/server/ICDServerConfig.h>
#include <crypto/CHIPCryptoPALPSA.h>
#include <crypto/SessionKeystore.h>

namespace chip {
namespace Crypto {

class PSASessionKeystore : public SessionKeystore
{
public:
    CHIP_ERROR CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Aes128KeyHandle & key) override;
    CHIP_ERROR CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Hmac128KeyHandle & key) override;
    CHIP_ERROR CreateKey(const ByteSpan & keyMaterial, HkdfKeyHandle & key) override;
    CHIP_ERROR DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                         Aes128KeyHandle & key) override;
    CHIP_ERROR DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info, Aes128KeyHandle & i2rKey,
                                 Aes128KeyHandle & r2iKey, AttestationChallenge & attestationChallenge) override;
    CHIP_ERROR DeriveSessionKeys(const HkdfKeyHandle & hkdfKey, const ByteSpan & salt, const ByteSpan & info,
                                 Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                 AttestationChallenge & attestationChallenge) override;
    void DestroyKey(Symmetric128BitsKeyHandle & key) override;
    void DestroyKey(HkdfKeyHandle & key) override;
#if CHIP_CONFIG_ENABLE_ICD_CIP
    CHIP_ERROR PersistICDKey(Symmetric128BitsKeyHandle & key) override;
#endif

private:
    CHIP_ERROR DeriveSessionKeys(PsaKdf & kdf, Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                 AttestationChallenge & attestationChallenge);
};

} // namespace Crypto
} // namespace chip
