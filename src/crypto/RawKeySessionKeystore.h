/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
