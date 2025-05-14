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

#include <platform/internal/GenericDeviceInstanceInfoProvider.h>
#include <platform/webos/ConfigurationManagerImpl.h>

namespace chip {
namespace DeviceLayer {

class DeviceInstanceInfoProviderImpl : public Internal::GenericDeviceInstanceInfoProvider<Internal::PosixConfig>
{
public:
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;

    DeviceInstanceInfoProviderImpl(ConfigurationManagerImpl & configManager) :
        Internal::GenericDeviceInstanceInfoProvider<Internal::PosixConfig>(configManager)
    {}
};

inline DeviceInstanceInfoProviderImpl & DeviceInstanceInfoProviderMgrImpl()
{
    static DeviceInstanceInfoProviderImpl sInstance(ConfigurationManagerImpl::GetDefaultInstance());
    return sInstance;
}
} // namespace DeviceLayer
} // namespace chip
