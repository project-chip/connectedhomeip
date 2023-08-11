/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <platform/nxp/k32w/common/FactoryDataProvider.h>
#include <vector>

namespace chip {
namespace DeviceLayer {

/**
 * This class provides K32W0 specific factory data features.
 * CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER       - application defined factory data provider
 * CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR - enables factory data OTA
 */

class FactoryDataProviderImpl : public FactoryDataProvider
{
public:
    FactoryDataProviderImpl();

    CHIP_ERROR Init() override;
    CHIP_ERROR SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer) override;

#if CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER
    virtual ~FactoryDataProviderImpl() {}
    // Custom factory data providers must implement this method in order to define
    // their own custom IDs.
    virtual CHIP_ERROR SetCustomIds() = 0;
#endif

#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
    using RestoreMechanism = CHIP_ERROR (*)(void);

    static CHIP_ERROR UpdateData(uint8_t * pBuf);
    CHIP_ERROR ValidateWithRestore();
    void RegisterRestoreMechanism(RestoreMechanism mechanism);

private:
    std::vector<RestoreMechanism> mRestoreMechanisms;
#endif
};

} // namespace DeviceLayer
} // namespace chip
