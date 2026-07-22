/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <credentials/DeviceAttestationCredsProvider.h>
#include <lib/core/CHIPError.h>
#include <memory>
#include <optional>
#include <string>

namespace chip {
namespace DeviceLayer {

/**
 * @brief DeviceAttestationCredentialsProvider implementation for all-devices-app.
 *
 * In simulation / test harness environments (such as POSIX CLI executions), this provider can load
 * test DAC credentials dynamically from a JSON file path. If no path is provided, it falls back to
 * the default example credentials.
 *
 * For commercial products, developers should replace this with a provider that binds directly to
 * secure hardware storage (e.g. Hardware Secure Element, TPM, TrustZone, or platform factory partition).
 */
class AllDevicesExampleDACProvider : public Credentials::DeviceAttestationCredentialsProvider
{
public:
    // Defined in .cpp to allow std::unique_ptr<FileProviderContext> with forward-declared FileProviderContext.
    AllDevicesExampleDACProvider();
    ~AllDevicesExampleDACProvider() override;

    /**
     * @brief Initializes the DAC credentials provider.
     *
     * @param[in] filePath Optional file path to a JSON DAC test vector. If not provided or empty,
     *                     the default SDK example DAC provider is used.
     */
    CHIP_ERROR Init(const std::optional<std::string> & filePath = std::nullopt);

    CHIP_ERROR GetCertificationDeclaration(MutableByteSpan & out_cd_buffer) override;
    CHIP_ERROR GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer) override;
    CHIP_ERROR GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer) override;

private:
    Credentials::DeviceAttestationCredentialsProvider * mDelegate = nullptr;

    // Use PIMPL idiom to avoid leaking TestHarnessDACProvider test headers into the public provider interface.
    struct FileProviderContext;
    std::unique_ptr<FileProviderContext> mFileContext;
};

} // namespace DeviceLayer
} // namespace chip
