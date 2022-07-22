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

#pragma once

#include <stdint.h>
#include <vector>

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <platform/CommissionableDataProvider.h>

class LinuxCommissionableDataProvider : public chip::DeviceLayer::CommissionableDataProvider
{
public:
    LinuxCommissionableDataProvider() {}

    /**
     * @brief Initialize the commissionable Data provider for Linux samples
     *
     * This version attempts to properly make use of the different arguments
     * to allow direct override of PASE verifier, and correct computation of
     * the PAKE verifier based on configurable iteration counts.
     *
     * The setupPasscode is not required BUT if it is absent, it will read
     * back as 0 and log a warning. This means testing assumptions of being able
     * to generate a setup payload will be wrong, but it will allow testing
     * configurations where the passcode is maintained separately than the
     * verifier for security purposes.
     *
     * @param serializedSpake2pVerifier - Optional serialized verifier that will
     *                                    override computation from setupPasscode if provided
     * @param spake2pSalt               - Optional salt to use. A random one will be generated
     *                                    otherwise.
     * @param spake2pIterationCount     - Iteration count to use. If not in range of the
     *                                    spec bounds, CHIP_ERROR_INVALID_ARGUMENT will be returned.
     * @param setupPasscode             - Optional setup passcode to store, and to use to generate
     *                                    PASE verifier if `serializedSpake2pVerifier` argument empty.
     * @param discriminator             - Discriminator to use for advertising.
     * @return CHIP_ERROR_OK on success, CHIP_ERROR_INVALID_ARGUMENT on any invalid argument combinations,
     *         CHIP_ERROR_INCORRECT_STATE if already initialized, or other CHIP_ERROR values if inner
     *         implementation dependencies fail.
     */
    CHIP_ERROR Init(chip::Optional<std::vector<uint8_t>> serializedSpake2pVerifier,
                    chip::Optional<std::vector<uint8_t>> spake2pSalt, uint32_t spake2pIterationCount,
                    chip::Optional<uint32_t> setupPasscode, uint16_t discriminator);

    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override
    {
        // We don't support overriding the discriminator post-init (it is deprecated!)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(chip::MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(chip::MutableByteSpan & verifierBuf, size_t & outVerifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override
    {
        // We don't support overriding the passcode post-init (it is deprecated!)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    bool mIsInitialized = false;
    std::vector<uint8_t> mSerializedPaseVerifier;
    std::vector<uint8_t> mPaseSalt;
    uint32_t mPaseIterationCount = 0;
    chip::Optional<uint32_t> mSetupPasscode;
    uint16_t mDiscriminator = 0;
};
