/*
 *
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

#include "LinuxCommissionableDataProvider.h"

#include <string.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_utils.h>

using namespace chip::Crypto;

namespace {

#if ENABLE_SE05X_SPAKE_VERIFIER_USE_TP_VALUES

constexpr size_t kSpake2p_PBKDF_Salt_Length_SE05x = 32;
constexpr size_t kSpake2p_Passcode_Length_SE05x   = 4;
constexpr uint32_t kSpake2p_Pwd_Salt_Bin_File_id  = 0x7FFF2000;
uint32_t setUpPINCode_se05x                       = 0;

#define BCD_TO_DEC(x) (x - 6 * (x >> 4))

CHIP_ERROR GeneratePaseSalt(std::vector<uint8_t> & spake2pSaltVector)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    constexpr size_t kSaltLen = kSpake2p_PBKDF_Salt_Length_SE05x;
    uint8_t cert[128]         = {
        0,
    };
    size_t certLen = sizeof(cert);
    /* 3 set of verifiers are provisioned in se05x. Each with 4 bytes passcode and 32 bytes salt */
    uint8_t offset = (SE05X_SPAKE_VERIFIER_TP_SET_NO - 1) * (kSpake2p_PBKDF_Salt_Length_SE05x + kSpake2p_Passcode_Length_SE05x);

    err = se05x_get_certificate(kSpake2p_Pwd_Salt_Bin_File_id, cert, &certLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    // To ensure we turn off the gpio and be ready for NFC comm
    VerifyOrReturnError(se05x_close_session() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(certLen >= (offset + kSpake2p_PBKDF_Salt_Length_SE05x + kSpake2p_Passcode_Length_SE05x),
                        CHIP_ERROR_INTERNAL);

    spake2pSaltVector.resize(kSaltLen);
    memcpy(spake2pSaltVector.data(), cert + offset + kSpake2p_Passcode_Length_SE05x, kSaltLen);

    setUpPINCode_se05x = (BCD_TO_DEC(cert[offset + 3])) + (100 * BCD_TO_DEC(cert[offset + 2])) +
        (10000 * BCD_TO_DEC(cert[offset + 1])) + (1000000 * BCD_TO_DEC(cert[offset]));

    return CHIP_NO_ERROR;
}

#else
CHIP_ERROR GeneratePaseSalt(std::vector<uint8_t> & spake2pSaltVector)
{
    constexpr size_t kSaltLen = kSpake2p_Max_PBKDF_Salt_Length;
    spake2pSaltVector.resize(kSaltLen);
    return DRBG_get_bytes(spake2pSaltVector.data(), spake2pSaltVector.size());
}
#endif

} // namespace

CHIP_ERROR LinuxCommissionableDataProvider::Init(chip::Optional<std::vector<uint8_t>> serializedSpake2pVerifier,
                                                 chip::Optional<std::vector<uint8_t>> spake2pSalt, uint32_t spake2pIterationCount,
                                                 chip::Optional<uint32_t> setupPasscode, uint16_t discriminator)
{
    VerifyOrReturnError(mIsInitialized == false, CHIP_ERROR_UNINITIALIZED);

    if (discriminator > chip::kMaxDiscriminatorValue)
    {
        ChipLogError(Support, "Discriminator value invalid: %u", static_cast<unsigned>(discriminator));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if ((spake2pIterationCount < kSpake2p_Min_PBKDF_Iterations) || (spake2pIterationCount > kSpake2p_Max_PBKDF_Iterations))
    {
        ChipLogError(Support, "PASE Iteration count invalid: %u", static_cast<unsigned>(spake2pIterationCount));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

#if ENABLE_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
    /* When using the trust provisioned verifiers, the possible values of iterationCount are 1000, 5000, 10000, 50000, 100000 */
    if (!(spake2pIterationCount == 1000 || spake2pIterationCount == 5000 || spake2pIterationCount == 10000 ||
          spake2pIterationCount == 50000 || spake2pIterationCount == 100000))
    {
        ChipLogError(Support,
                     "When using the trust provisioned verifiers, the possible values of iterationCount are 1000, 5000, 10000, "
                     "50000, 100000");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

#endif

    bool havePaseVerifier = serializedSpake2pVerifier.HasValue();
    Spake2pVerifier providedVerifier;
    CHIP_ERROR err;
    std::vector<uint8_t> finalSerializedVerifier(kSpake2p_VerifierSerialized_Length);
    if (havePaseVerifier)
    {
        if (serializedSpake2pVerifier.Value().size() != kSpake2p_VerifierSerialized_Length)
        {
            ChipLogError(Support, "PASE verifier size invalid: %zu", (serializedSpake2pVerifier.Value().size()));
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        chip::MutableByteSpan verifierSpan{ serializedSpake2pVerifier.Value().data(), serializedSpake2pVerifier.Value().size() };
        err = providedVerifier.Deserialize(verifierSpan);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to deserialized PASE verifier: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }

        ChipLogProgress(Support, "Got externally provided verifier, using it.");
    }

    bool havePaseSalt = spake2pSalt.HasValue();
    if (havePaseVerifier && !havePaseSalt)
    {
        ChipLogError(Support, "LinuxCommissionableDataProvider didn't get a PASE salt, but got a verifier: ambiguous data");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    size_t spake2pSaltLength = havePaseSalt ? spake2pSalt.Value().size() : 0;
    if (havePaseSalt &&
        ((spake2pSaltLength < kSpake2p_Min_PBKDF_Salt_Length) || (spake2pSaltLength > kSpake2p_Max_PBKDF_Salt_Length)))
    {
        ChipLogError(Support, "PASE salt length invalid: %u", static_cast<unsigned>(spake2pSaltLength));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!havePaseSalt)
    {
        ChipLogProgress(Support, "LinuxCommissionableDataProvider didn't get a PASE salt, generating one.");
        std::vector<uint8_t> spake2pSaltVector;
        err = GeneratePaseSalt(spake2pSaltVector);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to generate PASE salt: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
        spake2pSalt.SetValue(std::move(spake2pSaltVector));
    }

    bool havePasscode = setupPasscode.HasValue();
    std::vector<uint8_t> serializedPasscodeVerifier(kSpake2p_VerifierSerialized_Length);
    chip::MutableByteSpan saltSpan{ spake2pSalt.Value().data(), spake2pSalt.Value().size() };
    if (havePasscode)
    {
#if ENABLE_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
        ChipLogError(
            Support,
            "*** WARNING: Overriding the passcode (default / passcode from command line) with value provisioned in SE05x *** ");
        setupPasscode.SetValue(setUpPINCode_se05x);
#else
        Spake2pVerifier passcodeVerifier{};
        err = passcodeVerifier.Generate(spake2pIterationCount, saltSpan, setupPasscode.Value());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to generate PASE verifier from passcode: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }

        chip::MutableByteSpan verifierSpan{ serializedPasscodeVerifier.data(), serializedPasscodeVerifier.size() };
        err = passcodeVerifier.Serialize(verifierSpan);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Support, "Failed to serialize PASE verifier from passcode: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
#endif
    }

    // Make sure we actually have a verifier
    if (!havePasscode && !havePaseVerifier)
    {
        ChipLogError(Support, "Missing both externally provided verifier and passcode: cannot produce final verifier");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // If both passcode and external verifier were provided, validate they match, otherwise
    // it's ambiguous.
    if (havePasscode && havePaseVerifier)
    {
        if (serializedPasscodeVerifier != serializedSpake2pVerifier.Value())
        {
            ChipLogError(Support, "Mismatching verifier between passcode and external verifier. Validate inputs.");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ChipLogProgress(Support, "Validated externally provided passcode matches the one generated from provided passcode.");
    }

    // External PASE verifier takes precedence when present (even though it is identical to passcode-based
    // one when the latter is present).
    if (havePaseVerifier)
    {
        finalSerializedVerifier = serializedSpake2pVerifier.Value();
    }
    else
    {
        finalSerializedVerifier = serializedPasscodeVerifier;
    }

    mDiscriminator          = discriminator;
    mSerializedPaseVerifier = std::move(finalSerializedVerifier);
    mPaseSalt               = std::move(spake2pSalt.Value());
    mPaseIterationCount     = spake2pIterationCount;
    if (havePasscode)
    {
        mSetupPasscode.SetValue(setupPasscode.Value());
    }
    mIsInitialized = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxCommissionableDataProvider::GetSetupDiscriminator(uint16_t & setupDiscriminator)
{
    VerifyOrReturnError(mIsInitialized == true, CHIP_ERROR_UNINITIALIZED);
    setupDiscriminator = mDiscriminator;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxCommissionableDataProvider::GetSpake2pIterationCount(uint32_t & iterationCount)
{
    VerifyOrReturnError(mIsInitialized == true, CHIP_ERROR_UNINITIALIZED);
    iterationCount = mPaseIterationCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxCommissionableDataProvider::GetSpake2pSalt(chip::MutableByteSpan & saltBuf)
{
    VerifyOrReturnError(mIsInitialized == true, CHIP_ERROR_UNINITIALIZED);

    VerifyOrReturnError(saltBuf.size() >= kSpake2p_Max_PBKDF_Salt_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(saltBuf.data(), mPaseSalt.data(), mPaseSalt.size());
    saltBuf.reduce_size(mPaseSalt.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxCommissionableDataProvider::GetSpake2pVerifier(chip::MutableByteSpan & verifierBuf, size_t & outVerifierLen)
{
    VerifyOrReturnError(mIsInitialized == true, CHIP_ERROR_UNINITIALIZED);

    // By now, serialized verifier from Init should be correct size
    VerifyOrReturnError(mSerializedPaseVerifier.size() == kSpake2p_VerifierSerialized_Length, CHIP_ERROR_INTERNAL);

    outVerifierLen = mSerializedPaseVerifier.size();
    VerifyOrReturnError(verifierBuf.size() >= outVerifierLen, CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(verifierBuf.data(), mSerializedPaseVerifier.data(), mSerializedPaseVerifier.size());
    verifierBuf.reduce_size(mSerializedPaseVerifier.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxCommissionableDataProvider::GetSetupPasscode(uint32_t & setupPasscode)
{
    VerifyOrReturnError(mIsInitialized == true, CHIP_ERROR_UNINITIALIZED);

    // Pretend not implemented if we don't have a passcode value externally set
    if (!mSetupPasscode.HasValue())
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    setupPasscode = mSetupPasscode.Value();
    return CHIP_NO_ERROR;
}
