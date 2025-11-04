/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "chip_porting.h"
#include <credentials/DeviceAttestationCredsProvider.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/realtek/freertos/FactoryDataProvider.h>
#if FEATURE_TRUSTZONE_ENABLE && CONFIG_DAC_KEY_ENC
#include "rtk/include/nsc_veneer_customize.h"
#endif
namespace chip {
namespace DeviceLayer {

class RTKDACVendorProvider : public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    RTKDACVendorProvider() : pFactoryData(FactoryDataProvider::GetFactoryData()) {}
    ~RTKDACVendorProvider() = default;

    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & outBuffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

private:
    const FactoryData * pFactoryData;
    bool mDACKeyImported = false;
#if FEATURE_TRUSTZONE_ENABLE && CONFIG_DAC_KEY_ENC
    CHIP_ERROR ImportDACKey();
#endif
};

} // namespace DeviceLayer
} // namespace chip
