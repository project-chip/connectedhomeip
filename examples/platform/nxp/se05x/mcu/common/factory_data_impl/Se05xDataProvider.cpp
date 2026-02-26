/*
 *
 *    Copyright (c) 2022, 2026 Project CHIP Authors
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

#include "Se05xDataProvider.h"
#include <credentials/CHIPCert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>
#include <lib/support/Span.h>
#include <platform/ConfigurationManager.h>
#include <platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_utils.h>

#include <cctype>

using namespace chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace DeviceLayer {

static Se05xDataProviderImpl sInstance;

#if CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
static constexpr size_t kSpake2pSalt_MaxBase64Len = BASE64_ENCODED_LEN(chip::Crypto::kSpake2p_Max_PBKDF_Salt_Length) + 1;
constexpr size_t kSpake2p_PBKDF_Salt_Length_SE05x = 32;
constexpr size_t kSpake2p_Passcode_Length_SE05x   = 4;
constexpr uint32_t kSpake2p_Pwd_Salt_Bin_File_id  = 0x7FFF2000;

#define BCD_TO_DEC(x) (x - 6 * (x >> 4))
#endif

#if CONFIG_CHIP_SE05X_DEVICE_ATTESTATION
/* Device attestation key ids */
#define DEV_ATTESTATION_KEY_SE05X_ID 0x7FFF3007
#define DEV_ATTESTATION_CERT_SE05X_ID 0x7FFF3003
#endif

#if CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
CHIP_ERROR Se05xDataProviderImpl::GetSpake2pSaltBuffer(char * buf, uint16_t bufLen, uint16_t * outLen)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    constexpr size_t kSaltLen = kSpake2p_PBKDF_Salt_Length_SE05x;

    VerifyOrReturnError(SE05X_SPAKE_VERIFIER_TP_SET_NO >= 1 && SE05X_SPAKE_VERIFIER_TP_SET_NO <= 3, CHIP_ERROR_INVALID_ARGUMENT);

    /* 3 set of verifiers are provisioned in se05x. Each with 4 bytes passcode and 32 bytes salt */
    uint8_t offset = (SE05X_SPAKE_VERIFIER_TP_SET_NO - 1) * (kSpake2p_PBKDF_Salt_Length_SE05x + kSpake2p_Passcode_Length_SE05x);

    if (certLen == 0)
    {
        certLen = sizeof(cert);

        err = se05x_get_certificate(kSpake2p_Pwd_Salt_Bin_File_id, cert, &certLen);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    }

    VerifyOrReturnError(certLen >= (offset + kSpake2p_PBKDF_Salt_Length_SE05x + kSpake2p_Passcode_Length_SE05x),
                        CHIP_ERROR_INTERNAL);

    if (buf != NULL && outLen != NULL)
    {
        VerifyOrReturnError(bufLen >= kSaltLen, CHIP_ERROR_INTERNAL);
        memcpy(buf, cert + offset + kSpake2p_Passcode_Length_SE05x, kSaltLen);
        *outLen = kSaltLen;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Se05xDataProviderImpl::GetSpake2pSalt(MutableByteSpan & saltBuf)
{

    CHIP_ERROR err                          = CHIP_NO_ERROR;
    char saltB64[kSpake2pSalt_MaxBase64Len] = { 0 };
    uint16_t saltB64Len                     = 0;
    err                                     = GetSpake2pSaltBuffer(saltB64, sizeof(saltB64), &saltB64Len);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "Failed to generate PASE salt: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    VerifyOrReturnError(saltB64Len <= saltBuf.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), saltB64, saltB64Len);
    saltBuf.reduce_size(saltB64Len);
    return err;
}

CHIP_ERROR Se05xDataProviderImpl::GetSetupPasscode(uint32_t & setupPasscode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (certLen == 0)
    {
        // Call GetSpake2pSaltBuffer to read the certificate
        err = GetSpake2pSaltBuffer(NULL, 0, NULL);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to generate PASE salt: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
    }

    /* 3 set of verifiers are provisioned in se05x. Each with 4 bytes passcode and 32 bytes salt */
    size_t offset = (SE05X_SPAKE_VERIFIER_TP_SET_NO - 1) * (kSpake2p_PBKDF_Salt_Length_SE05x + kSpake2p_Passcode_Length_SE05x);

    VerifyOrReturnError(certLen >= offset + 4, CHIP_ERROR_INTERNAL);
    setupPasscode = (BCD_TO_DEC(cert[offset + 3])) + (100 * BCD_TO_DEC(cert[offset + 2])) + (10000 * BCD_TO_DEC(cert[offset + 1])) +
        (1000000 * BCD_TO_DEC(cert[offset]));
    return err;
}

CHIP_ERROR Se05xDataProviderImpl::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    iterationCount = SE05X_SPAKE_VERIFIER_TP_ITER_CNT;
    return CHIP_NO_ERROR;
}
#endif

#if CONFIG_CHIP_SE05X_DEVICE_ATTESTATION
// ===== Device Attestation Credentials Implementation using SE05X =====
CHIP_ERROR Se05xDataProviderImpl::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    size_t buflen = out_dac_buffer.size();
    ChipLogDetail(Crypto, "Get DA certificate from se05x");
    ReturnErrorOnFailure(se05x_get_certificate(DEV_ATTESTATION_CERT_SE05X_ID, out_dac_buffer.data(), &buflen));
    out_dac_buffer.reduce_size(buflen);
    if (buflen > 4 && out_dac_buffer.data()[0] == 0x31)
    {
        out_dac_buffer = out_dac_buffer.SubSpan(4); // ignoring TLV of DA certificate
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Se05xDataProviderImpl::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign,
                                                               MutableByteSpan & out_signature_buffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256KeypairSE05x keypair;
    Crypto::P256SerializedKeypair serialized_keypair;
    uint8_t magic_bytes[] = NXP_CRYPTO_KEY_MAGIC;

    ChipLogDetail(Crypto, "Sign using DA key from se05x");

    VerifyOrReturnError(!out_signature_buffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!message_to_sign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature_buffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Add public key + reference private key (ref to key inside SE)

    serialized_keypair.SetLength(Crypto::kP256_PublicKey_Length + Crypto::kP256_PrivateKey_Length);

    memset(serialized_keypair.Bytes(), 0, Crypto::kP256_PublicKey_Length);
    memcpy(serialized_keypair.Bytes() + Crypto::kP256_PublicKey_Length, magic_bytes, sizeof(magic_bytes));
    *(serialized_keypair.Bytes() + Crypto::kP256_PublicKey_Length + sizeof(magic_bytes) + 0) =
        (DEV_ATTESTATION_KEY_SE05X_ID & 0xFF000000) >> (8 * 3);
    *(serialized_keypair.Bytes() + Crypto::kP256_PublicKey_Length + sizeof(magic_bytes) + 1) =
        (DEV_ATTESTATION_KEY_SE05X_ID & 0x00FF0000) >> (8 * 2);
    *(serialized_keypair.Bytes() + Crypto::kP256_PublicKey_Length + sizeof(magic_bytes) + 2) =
        (DEV_ATTESTATION_KEY_SE05X_ID & 0x0000FF00) >> (8 * 1);
    *(serialized_keypair.Bytes() + Crypto::kP256_PublicKey_Length + sizeof(magic_bytes) + 3) =
        (DEV_ATTESTATION_KEY_SE05X_ID & 0x000000FF) >> (8 * 0);

    ReturnErrorOnFailure(keypair.Deserialize(serialized_keypair));

    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(message_to_sign.data(), message_to_sign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, out_signature_buffer);
}
#endif

FactoryDataProvider & FactoryDataPrvdImpl()
{
    return sInstance;
}

} // namespace DeviceLayer
} // namespace chip
