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

constexpr uint32_t kMinSetupPasscode = 1;
constexpr uint32_t kMaxSetupPasscode = 0x5F5E0FE;

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
 * Instance getter for the global CommissionableDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global device attestation credentials provider. Assume never null.
 */
CommissionableDataProvider * GetCommissionableDataProvider();

/**
 * Instance setter for the global CommissionableDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the CommissionableDataProvider to start returning with the getter
 */
void SetCommissionableDataProvider(CommissionableDataProvider * provider);

} // namespace DeviceLayer
} // namespace chip
