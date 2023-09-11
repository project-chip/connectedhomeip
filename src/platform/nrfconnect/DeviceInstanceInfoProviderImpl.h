/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/Zephyr/ConfigurationManagerImpl.h>
#include <platform/internal/GenericDeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class DeviceInstanceInfoProviderImpl : public Internal::GenericDeviceInstanceInfoProvider<Internal::ZephyrConfig>
{
public:
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override;

    DeviceInstanceInfoProviderImpl(ConfigurationManagerImpl & configManager) :
        Internal::GenericDeviceInstanceInfoProvider<Internal::ZephyrConfig>(configManager)
    {}
};

inline DeviceInstanceInfoProviderImpl & DeviceInstanceInfoProviderMgrImpl()
{
    static DeviceInstanceInfoProviderImpl sInstance(ConfigurationManagerImpl::GetDefaultInstance());
    return sInstance;
}
} // namespace DeviceLayer
} // namespace chip
