/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {

// The largest value of the 12-bit Payload discriminator
constexpr uint16_t kMaxDiscriminatorValue = 0xFFF;

namespace DeviceLayer {

class CommissionableDataProvider
{
public:
    CommissionableDataProvider()          = default;
    virtual ~CommissionableDataProvider() = default;

    // Not copyable
    CommissionableDataProvider(const CommissionableDataProvider &) = delete;
    CommissionableDataProvider & operator=(const CommissionableDataProvider &) = delete;

    /**
     * @brief Obtain the default setup discriminator for the device used for commissionable advertising
     *
     * @param[in,out] setupDiscriminator Reference to location where discriminator will be copied
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) = 0;

    /**
     * @brief Set the setup discriminator *if supported*.
     *
     * This API is optional to support, but present to support integration with some examples
     * which use a common test PAKE verifier but want to disambiguate multiple instances at
     * runtime. This is generally not applicable to production usecases of commissionable
     * devices with nominal random discriminator. There are no guarantees that this API,
     * in general, will persist the discriminator across restarts.
     *
     * If not supported, return CHIP_ERROR_NOT_IMPLEMENTED.
     *
     * @param[in] setupDiscriminator Setup discriminator to set.
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT if discriminator is out of range,
     *          CHIP_ERROR_NOT_IMPLEMENTED if not implemented, or another CHIP_ERROR from the underlying
     *          implementation if storage fails.
     */
    virtual CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) = 0;

    /**
     * @brief Obtain the default PAKE verifier's PBKDF2 iteration count
     *
     * @param[in,out] iterationCount Reference to location where the iteration count will be stored
     * @returns CHIP_NO_ERROR on success, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) = 0;

    /**
     * @brief Obtain the default PAKE verifier's PBKDF2 salt in the provided `saltBuf`
     *
     * The `saltBuf` must be large enough to contain the salt. It will be resized on success to
     * reflect the actual size of the salt.
     *
     * @param[in,out] saltBuf  Reference to buffer where the salt value will be copied.
     *
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if saltBuf was too small, or another
     *          CHIP_ERROR from the underlying implementation if access fails.
     */
    virtual CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) = 0;

    /**
     * @brief Obtain the default PAKE verifier in the provided `verifierBuf`
     *
     * The `verifierBuf` must be large enough to contain the verifier. It will be resized on
     * success to reflect the actual size of the verifier and match the value in verifierLen.
     *
     * @param[in,out] verifierBuf  Reference to buffer where the verifier value will be copied.
     * @param[out] outVerifierLen Reference to location where the length of the verifier will be stored,
     *                            including if the buffer provided is too small. This value is valid
     *                            if the return value is either CHIP_NO_ERROR or CHIP_ERROR_BUFFER_TOO_SMALL.
     *
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_BUFFER_TOO_SMALL if verifierBuf was too small, or another
     *          CHIP_ERROR from the underlying implementation if access fails.
     */
    virtual CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & outVerifierLen) = 0;

    /**
     * @brief Obtain the default setup passcode for the device *if supported*
     *
     * This API is optional to support, but present to support integration with some examples
     * which allow runtime setting of a passcode to generate onboarding payloads and QR codes
     * as well as PAKE verifier. This is generally not applicable to production usecases of
     * commissionable devices with nominal random verifier/passcode and with usage of verifier
     * rather than passcode (which safeguards the passcode from ever leaking). There are no
     * guarantees that this API, in general, will persist the passcode across restarts.
     *
     * If not supported, return CHIP_ERROR_NOT_IMPLEMENTED.
     *
     * @param[in,out] setupPasscode Reference to location where passcode will be copied
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_NOT_IMPLEMENTED if not implemented
     *          or not available, or another CHIP_ERROR from the underlying implementation
     *          if access fails.
     */
    virtual CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) = 0;

    /**
     * @brief Set the setup discriminator *if supported*.
     *
     * This API is optional to support, but present to support integration with some examples
     * which allow runtime setting of a passcode to generate onboarding payloads and QR codes
     * as well as PAKE verifier. This is generally not applicable to production usecases of
     * commissionable devices with nominal random verifier/passcode and with usage of verifier
     * rather than passcode (which safeguards the passcode from ever leaking). There are no
     * guarantees that this API, in general, will persist the passcode across restarts.
     *
     * If not supported, return CHIP_ERROR_NOT_IMPLEMENTED.
     *
     * @param[in] setupPasscode Setup passcode to set
     * @returns CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT if discriminator is invalid
     *          according to spec (out of range or forbidden value), CHIP_ERROR_NOT_IMPLEMENTED
     *          if not implemented, or another CHIP_ERROR from the underlying implementation if
     *          storage fails.
     */
    virtual CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) = 0;
};

/**
 * @brief Version of a CommissionableDataProvider with just enough
 *        support for basic tests of the minimum required CommissionableDataProvider
 *        features.
 */
class TestCommissionableDataProvider : public CommissionableDataProvider
{
public:
    TestCommissionableDataProvider() {}

    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override
    {
        constexpr uint16_t kDefaultTestDiscriminator = 3840;
        setupDiscriminator                           = kDefaultTestDiscriminator;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override
    {
        (void) setupDiscriminator;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override
    {
        constexpr uint32_t kDefaultTestVerifierIterationCount = 1000;
        iterationCount                                        = kDefaultTestVerifierIterationCount;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override
    {
        const uint8_t kDefaultTestVerifierSalt[16] = {
            0x53, 0x50, 0x41, 0x4b, 0x45, 0x32, 0x50, 0x20, 0x4b, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6c, 0x74,
        };

        size_t saltLen = sizeof(kDefaultTestVerifierSalt);
        if (saltBuf.size() < saltLen)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(saltBuf.data(), &kDefaultTestVerifierSalt[0], saltLen);
        saltBuf.reduce_size(saltLen);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & outVerifierLen) override
    {
        const uint8_t kDefaultTestVerifier[97] = {
            0xb9, 0x61, 0x70, 0xaa, 0xe8, 0x03, 0x34, 0x68, 0x84, 0x72, 0x4f, 0xe9, 0xa3, 0xb2, 0x87, 0xc3, 0x03, 0x30, 0xc2, 0xa6,
            0x60, 0x37, 0x5d, 0x17, 0xbb, 0x20, 0x5a, 0x8c, 0xf1, 0xae, 0xcb, 0x35, 0x04, 0x57, 0xf8, 0xab, 0x79, 0xee, 0x25, 0x3a,
            0xb6, 0xa8, 0xe4, 0x6b, 0xb0, 0x9e, 0x54, 0x3a, 0xe4, 0x22, 0x73, 0x6d, 0xe5, 0x01, 0xe3, 0xdb, 0x37, 0xd4, 0x41, 0xfe,
            0x34, 0x49, 0x20, 0xd0, 0x95, 0x48, 0xe4, 0xc1, 0x82, 0x40, 0x63, 0x0c, 0x4f, 0xf4, 0x91, 0x3c, 0x53, 0x51, 0x38, 0x39,
            0xb7, 0xc0, 0x7f, 0xcc, 0x06, 0x27, 0xa1, 0xb8, 0x57, 0x3a, 0x14, 0x9f, 0xcd, 0x1f, 0xa4, 0x66, 0xcf,
        };

        outVerifierLen = sizeof(kDefaultTestVerifier);
        if (verifierBuf.size() < outVerifierLen)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        memcpy(verifierBuf.data(), &kDefaultTestVerifier[0], outVerifierLen);
        verifierBuf.reduce_size(outVerifierLen);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override
    {
        constexpr uint32_t kDefaultTestPasscode = 20202021;
        setupPasscode                           = kDefaultTestPasscode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override
    {
        (void) setupPasscode;
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

} // namespace DeviceLayer
} // namespace chip
