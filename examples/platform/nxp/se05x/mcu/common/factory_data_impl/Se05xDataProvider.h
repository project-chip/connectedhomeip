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
#pragma once

#if CONFIG_CHIP_NXP_PLATFORM_RW61X
#include <platform/nxp/rt/rw61x/FactoryDataProviderImpl.h>
#elif CONFIG_CHIP_NXP_PLATFORM_RT1060
#include <platform/nxp/rt/rt1060/FactoryDataProviderImpl.h>
#else
#error "The selected platform is not supported for SE05x data provider"
#endif

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

#include <platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_utils.h>

#include <vector>

namespace chip {
namespace DeviceLayer {

/**
 * @brief This class provides spake salt data, iterations and passcode.
 */

class Se05xDataProviderImpl : public FactoryDataProviderImpl
{
public:
#if CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
    CHIP_ERROR GetSpake2pSalt(MutableByteSpan & saltBuf) override;
    CHIP_ERROR GetSetupPasscode(uint32_t & setupPasscode) override;
    CHIP_ERROR GetSpake2pIterationCount(uint32_t & iterationCount) override;
#endif

#if CONFIG_CHIP_SE05X_DEVICE_ATTESTATION
    // Device Attestation Credentials overrides for SE05X
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & messageToSign, MutableByteSpan & out_signature_buffer) override;
#endif

private:
#if CONFIG_CHIP_SE05X_SPAKE_VERIFIER_USE_TP_VALUES
    uint8_t cert[128] = {0};
    size_t certLen = 0;

    CHIP_ERROR GetSpake2pSaltBuffer(uint8_t * buf, uint16_t bufLen, uint16_t * outLen);
#endif
};

FactoryDataProvider & FactoryDataPrvdImpl();

} // namespace DeviceLayer
} // namespace chip
