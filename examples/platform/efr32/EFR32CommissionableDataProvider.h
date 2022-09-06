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

#include <platform/CommissionableDataProvider.h>
#include <platform/internal/GenericDeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {
namespace EFR32 {

/**
 * @brief This class provides Commissionable data, Device Attestation Credentials,
 *        and Device Instance Info.
 */

class EFR32DeviceDataProvider : public CommissionableDataProvider
#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
    ,
                                public Internal::GenericDeviceInstanceInfoProvider<Internal::EFR32Config>
#endif // CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
{
public:
    EFR32DeviceDataProvider() :
        CommissionableDataProvider()
#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
        ,
        Internal::GenericDeviceInstanceInfoProvider<Internal::EFR32Config>(ConfigurationManagerImpl::GetDefaultInstance())
#endif // CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
    {}

    // ===== Members functions that implement the CommissionableDataProvider
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
    // ===== Members functions that implement the GenericDeviceInstanceInfoProvider
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;
#endif // CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
};

/**
 * @brief Get implementation of the EFR32DeviceDataProvider
 *
 * @returns a singleton CommissionableDataProvider of the EFR32 platform implementation
 */
CommissionableDataProvider * GetEFR32DeviceDataProvider();

} // namespace EFR32
} // namespace DeviceLayer
} // namespace chip
