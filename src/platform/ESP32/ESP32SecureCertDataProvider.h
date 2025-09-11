/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

/*
 * ESP32FactoryDataProvider reads the commissionable data and device instance information from the factory partition.
 * This implementation extends that behaviour to read the commissionable data from secure cert partition.
 * It also extends the behaviour to read the unique id for generating rotating device identifier from the secure cert
 * partition.
 *
 */

#pragma once

#include "ESP32FactoryDataProvider.h"
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#include <esp_secure_cert_tlv_config.h>

namespace chip {
namespace DeviceLayer {

class ESP32SecureCertDataProvider : public ESP32FactoryDataProvider
{
public:
    // CommissionableDataProvider implementation
    CHIP_ERROR GetSetupDiscriminator(uint16_t & setupDiscriminator) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSpake2pVerifier(MutableByteSpan & verifierBuf, size_t & verifierLen) override;

#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER
    // GetRotatingDeviceIdUniqueId from GenericDeviceInstanceInfoProvider
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;
#endif // CHIP_DEVICE_CONFIG_ENABLE_DEVICE_INSTANCE_INFO_PROVIDER

    // esp-secure-cert partition contains two 32-byte fixed random values that are set during manufacturing
    // and remain constant for the lifetime of the device. These are unique per device and can be used
    // for device identification, serial numbers, or any other purpose requiring a device-specific identifier.
    static constexpr uint32_t kFixedRandomValueLength = 32;
    static CHIP_ERROR GetFixedRandom1(MutableByteSpan & randomBuf);
    static CHIP_ERROR GetFixedRandom2(MutableByteSpan & randomBuf);
};

} // namespace DeviceLayer
} // namespace chip
