/*
 *
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

#include "DataProvider.h"
#include "core/Types.h"

#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <platform/CommissionableDataProvider.h>

namespace matter {
namespace casting {
namespace support {

/**
 * @brief Parameters required to Initialize() the CastingApp
 */
class AppParameters
{
public:
    AppParameters() {}
    CHIP_ERROR Create(MutableByteSpanDataProvider * rotatingDeviceIdUniqueIdProvider,
                      chip::DeviceLayer::CommissionableDataProvider * commissionableDataProvider,
                      chip::Credentials::DeviceAttestationCredentialsProvider * deviceAttestationCredentialsProvider,
                      chip::Credentials::DeviceAttestationVerifier * deviceAttestationVerifier,
                      ServerInitParamsProvider * serverInitParamsProvider)
    {
        VerifyOrReturnError(commissionableDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(deviceAttestationCredentialsProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(deviceAttestationVerifier != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(serverInitParamsProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        mRotatingDeviceIdUniqueIdProvider     = rotatingDeviceIdUniqueIdProvider;
        mCommissionableDataProvider           = commissionableDataProvider;
        mDeviceAttestationCredentialsProvider = deviceAttestationCredentialsProvider;
        mDeviceAttestationVerifier            = deviceAttestationVerifier;
        mServerInitParamsProvider             = serverInitParamsProvider;

        return CHIP_NO_ERROR;
    }

    MutableByteSpanDataProvider * GetRotatingDeviceIdUniqueIdProvider() const { return mRotatingDeviceIdUniqueIdProvider; }

    chip::DeviceLayer::CommissionableDataProvider * GetCommissionableDataProvider() const { return mCommissionableDataProvider; }

    chip::Credentials::DeviceAttestationCredentialsProvider * GetDeviceAttestationCredentialsProvider() const
    {
        return mDeviceAttestationCredentialsProvider;
    }

    chip::Credentials::DeviceAttestationVerifier * GetDeviceAttestationVerifier() const { return mDeviceAttestationVerifier; }

    ServerInitParamsProvider * GetServerInitParamsProvider() const { return mServerInitParamsProvider; }

private:
    /**
     * @brief Provides UniqueId used to generate the RotatingDeviceId advertised by the CastingApp
     *
     */
    MutableByteSpanDataProvider * mRotatingDeviceIdUniqueIdProvider;

    /**
     * @brief Provides CommissionableData (such as setupPasscode, discriminator, etc) used to get the CastingApp commissioned
     *
     */
    chip::DeviceLayer::CommissionableDataProvider * mCommissionableDataProvider;

    /**
     * @brief Provides DeviceAttestationCredentials of the CastingApp during commissioning
     *
     */
    chip::Credentials::DeviceAttestationCredentialsProvider * mDeviceAttestationCredentialsProvider;

    /**
     * @brief DeviceAttestationVerifier used by the CastingApp during commissioning
     *
     */
    chip::Credentials::DeviceAttestationVerifier * mDeviceAttestationVerifier;

    /**
     * @brief Provides params used to initialize the Matter server run by the CastingApp
     *
     */
    ServerInitParamsProvider * mServerInitParamsProvider;
};

}; // namespace support
}; // namespace casting
}; // namespace matter
