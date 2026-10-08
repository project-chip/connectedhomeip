/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/network-identity-management-server/RawKeyNetworkIdentityKeystore.h>

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafePointerCast.h>

#include <type_traits>

namespace chip::Crypto {

struct RawNassHkdfKeyHandle
{
    uint8_t data[kNetworkAdministratorRawSecretLength];
};

CHIP_ERROR RawKeyNetworkIdentityKeystore::ImportNetworkAdministratorSecret(const NetworkAdministratorRawSecret & secret,
                                                                           HkdfKeyHandle & outHandle)
{
    auto & rawHandle = outHandle.AsMutable<RawNassHkdfKeyHandle>();
    static_assert(sizeof(rawHandle.data) == std::remove_reference_t<decltype(secret)>::Length());
    memcpy(rawHandle.data, secret.ConstBytes(), sizeof(rawHandle.data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR RawKeyNetworkIdentityKeystore::ExportNetworkAdministratorSecret(const HkdfKeyHandle & handle,
                                                                           NetworkAdministratorRawSecret & outRawSecret)
{
    auto & rawHandle = handle.As<RawNassHkdfKeyHandle>();
    static_assert(sizeof(rawHandle.data) == std::remove_reference_t<decltype(outRawSecret)>::Length());
    memcpy(outRawSecret.Bytes(), rawHandle.data, sizeof(rawHandle.data));
    return CHIP_NO_ERROR;
}

void RawKeyNetworkIdentityKeystore::DestroyNetworkAdministratorSecret(HkdfKeyHandle & handle)
{
    ClearSecretData(handle.AsMutable<RawNassHkdfKeyHandle>().data);
}

CHIP_ERROR RawKeyNetworkIdentityKeystore::DeriveECDSANetworkIdentity(const HkdfKeyHandle & nassHandle,
                                                                     P256KeypairHandle & outKeypairHandle,
                                                                     MutableByteSpan & outIdentity)
{
    // The HKDF_SHA256 API in CHIPCryptoPAL.h does not support streaming. Simply generate
    // enough HKDF output to allow for up to 8 key derivation attempts. The probability of
    // needing more than this number of attempts is approximately 1 in 2^256 (10^77).
    static constexpr int kMaxRounds = 8;
    SensitiveDataBuffer<kP256_FE_Length * kMaxRounds> keyMaterial;

    // See "Network Identity Derivation for ECDSA" in the Matter Specification
    HKDF_sha hkdf;
    auto & rawNassHandle            = nassHandle.As<RawNassHkdfKeyHandle>();
    static constexpr CharSpan kInfo = "NASS-ECDSA-secp256r1"_span;
    ReturnErrorOnFailure(hkdf.HKDF_SHA256(rawNassHandle.data, sizeof(rawNassHandle.data),     // input
                                          nullptr, 0,                                         // salt
                                          Uint8::from_const_char(kInfo.data()), kInfo.size(), // info
                                          keyMaterial.Bytes(), keyMaterial.Capacity()));

    for (unsigned i = 0; i < kMaxRounds; i++)
    {
        P256Keypair keypair;
        FixedByteSpan<kP256_PrivateKey_Length> keyBits =
            FixedByteSpan<kP256_PrivateKey_Length>(keyMaterial.Bytes() + i * kP256_FE_Length);
        CHIP_ERROR err = keypair.InitializeFromBitsOrReject(keyBits);
        if (err == CHIP_ERROR_INVALID_ARGUMENT)
        {
            continue; // try again
        }
        ReturnErrorOnFailure(err);

        ReturnErrorOnFailure(Credentials::DeriveChipNetworkIdentity(keypair, outIdentity));
        ReturnErrorOnFailure(keypair.Serialize(outKeypairHandle));
        return CHIP_NO_ERROR;
    }

    ChipLogError(Crypto, "ECDSA Network Identity derivation by rejection sampling failed after max attempts");
    return CHIP_ERROR_INTERNAL;
}

void RawKeyNetworkIdentityKeystore::DestroyNetworkIdentityKeypair(P256KeypairHandle & handle)
{
    handle.Clear();
}

} // namespace chip::Crypto
