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
namespace Silabs {

/**
 * @brief This class provides Commissionable data, Device Attestation Credentials,
 *        and Device Instance Info.
 */

class SilabsDeviceDataProvider : public CommissionableDataProvider,
                                 public Internal::GenericDeviceInstanceInfoProvider<Internal::SilabsConfig>
{
public:
    SilabsDeviceDataProvider() :
        CommissionableDataProvider(),
        Internal::GenericDeviceInstanceInfoProvider<Internal::SilabsConfig>(ConfigurationManagerImpl::GetDefaultInstance())
    {}

// TODO Remove once Commander supports (doesn't erase) NVM3 for 917
#ifdef SIWX917_USE_COMISSIONABLE_DATA
    void setupPayload(uint8_t * outBuf);
    CHIP_ERROR FlashFactoryData();
#endif /* SIWX917_USE_COMISSIONABLE_DATA */

    static SilabsDeviceDataProvider & GetDeviceDataProvider();
    CHIP_ERROR GetSetupPayload(MutableCharSpan & payloadBuf);
    // ===== Members functions that implement the CommissionableDataProvider
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR SetSetupDiscriminator(uint16_t setupDiscriminator) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen) override;
    // Per spec 5.1.7. Passcode cannot be stored on the device
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override { return CHIP_ERROR_NOT_IMPLEMENTED; };
    CHIP_ERROR SetSetupPasscode(uint32_t setupPasscode) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // ===== Members functions that implement the GenericDeviceInstanceInfoProvider
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override;
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override;
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductURL(char * buf, size_t bufSzie) override;
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
