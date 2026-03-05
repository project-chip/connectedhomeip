/*
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

#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace app {
namespace Clusters {


class DeviceLayerBasicInformationPolicy
{
public:
    // Base class for the cluster to inherit from, to satisfy PlatformManagerDelegate requirements
    using DelegateBase = DeviceLayer::PlatformManagerDelegate;

    struct Context
    {
        DeviceLayer::DeviceInstanceInfoProvider & deviceInstanceInfoProvider;
        DeviceLayer::ConfigurationManager & configurationManager;
        DeviceLayer::PlatformManager & platformManager;
        uint16_t subscriptionsPerFabric;
    };

    DeviceLayerBasicInformationPolicy(Context ctx) : mContext(ctx) {}

    // Delegated methods
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize)
    {
        return mContext.deviceInstanceInfoProvider.GetVendorName(buf, bufSize);
    }

    CHIP_ERROR GetVendorId(uint16_t & vendorId) const
    {
        return mContext.deviceInstanceInfoProvider.GetVendorId(vendorId);
    }

    CHIP_ERROR GetProductName(char * buf, size_t bufSize)
    {
        return mContext.deviceInstanceInfoProvider.GetProductName(buf, bufSize);
    }

    CHIP_ERROR GetProductId(uint16_t & productId) const
    {
        return mContext.deviceInstanceInfoProvider.GetProductId(productId);
    }

    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize)
    {
        return mContext.deviceInstanceInfoProvider.GetPartNumber(buf, bufSize);
    }

    CHIP_ERROR GetProductURL(char * buf, size_t bufSize)
    {
        return mContext.deviceInstanceInfoProvider.GetProductURL(buf, bufSize);
    }

    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize)
    {
        return mContext.deviceInstanceInfoProvider.GetProductLabel(buf, bufSize);
    }

    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize)
    {
        return mContext.deviceInstanceInfoProvider.GetSerialNumber(buf, bufSize);
    }

    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
    {
        return mContext.deviceInstanceInfoProvider.GetManufacturingDate(year, month, day);
    }

    CHIP_ERROR GetManufacturingDateSuffix(MutableCharSpan & suffixBuffer)
    {
        return mContext.deviceInstanceInfoProvider.GetManufacturingDateSuffix(suffixBuffer);
    }

    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion)
    {
        return mContext.deviceInstanceInfoProvider.GetHardwareVersion(hardwareVersion);
    }

    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize)
    {
        return mContext.deviceInstanceInfoProvider.GetHardwareVersionString(buf, bufSize);
    }

    CHIP_ERROR GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
    {
        return mContext.deviceInstanceInfoProvider.GetProductFinish(finish);
    }

    CHIP_ERROR GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
    {
        return mContext.deviceInstanceInfoProvider.GetProductPrimaryColor(primaryColor);
    }

    CHIP_ERROR GetLocalConfigDisabled(bool & localConfigDisabled)
    {
        return mContext.deviceInstanceInfoProvider.GetLocalConfigDisabled(localConfigDisabled);
    }

    CHIP_ERROR SetLocalConfigDisabled(bool localConfigDisabled)
    {
        return mContext.deviceInstanceInfoProvider.SetLocalConfigDisabled(localConfigDisabled);
    }

    DeviceLayer::DeviceInstanceInfoProvider::DeviceInfoCapabilityMinimas GetSupportedCapabilityMinimaValues()
    {
        return mContext.deviceInstanceInfoProvider.GetSupportedCapabilityMinimaValues();
    }

    // ConfigurationManager delegated methods
    CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVersion)
    {
        return mContext.configurationManager.GetSoftwareVersion(softwareVersion);
    }

    CHIP_ERROR GetSoftwareVersionString(char * buf, size_t bufSize)
    {
        return mContext.configurationManager.GetSoftwareVersionString(buf, bufSize);
    }

    CHIP_ERROR GetUniqueId(char * buf, size_t bufSize)
    {
        return mContext.configurationManager.GetUniqueId(buf, bufSize);
    }

    CHIP_ERROR GetConfigurationVersion(uint32_t & configurationVersion)
    {
        return mContext.configurationManager.GetConfigurationVersion(configurationVersion);
    }
    
    CHIP_ERROR StoreConfigurationVersion(uint32_t configurationVersion)
    {
        return mContext.configurationManager.StoreConfigurationVersion(configurationVersion);
    }

    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)
    {
        return mContext.configurationManager.GetCountryCode(buf, bufSize, codeLen);
    }

    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen)
    {
        return mContext.configurationManager.StoreCountryCode(code, codeLen);
    }

    // PlatformManagerDelegate logic
    void RegisterPlatformDelegate(DeviceLayer::PlatformManagerDelegate * delegate)
    {
        if (mContext.platformManager.GetDelegate() == nullptr)
        {
            mContext.platformManager.SetDelegate(delegate);
        }
    }

    void UnregisterPlatformDelegate(DeviceLayer::PlatformManagerDelegate * delegate)
    {
        if (mContext.platformManager.GetDelegate() == delegate)
        {
            mContext.platformManager.SetDelegate(nullptr);
        }
    }

    uint16_t GetSubscriptionsPerFabric() const { return mContext.subscriptionsPerFabric; }

private:
   Context mContext;
};

} // namespace Clusters
} // namespace app
} // namespace chip
