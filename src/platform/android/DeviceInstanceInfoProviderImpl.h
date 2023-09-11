/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/android/ConfigurationManagerImpl.h>
#include <platform/internal/GenericDeviceInstanceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class DeviceInstanceInfoProviderImpl : public Internal::GenericDeviceInstanceInfoProvider<Internal::AndroidConfig>
{
public:
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductId(uint16_t & productId) override;
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override;
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override;
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override;

    DeviceInstanceInfoProviderImpl(ConfigurationManagerImpl & configManager) :
        Internal::GenericDeviceInstanceInfoProvider<Internal::AndroidConfig>(configManager)
    {}
};

inline DeviceInstanceInfoProviderImpl & DeviceInstanceInfoProviderMgrImpl()
{
    static DeviceInstanceInfoProviderImpl sInstance(ConfigurationManagerImpl::GetDefaultInstance());
    return sInstance;
}
} // namespace DeviceLayer
} // namespace chip
