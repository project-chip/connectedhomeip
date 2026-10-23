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

#include <app/clusters/basic-information/BasicInformationOptionalAttributes.h>
#include <cstddef>
#include <cstdint>
#include <lib/support/Span.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>

namespace chip::app::Clusters {

class DeviceLayerBasicInformationPolicy
{
public:
    // Base class for the cluster to inherit from, to satisfy PlatformManagerDelegate requirements
    using LifetimeDelegate = DeviceLayer::PlatformManagerDelegate;

    DeviceLayerBasicInformationPolicy(BasicInformationOptionalAttributesSet optionalAttributes,
                                      DeviceLayer::DeviceInstanceInfoProvider & deviceInstanceInfoProvider,
                                      DeviceLayer::ConfigurationManager & configurationManager,
                                      DeviceLayer::PlatformManager & platformManager, uint16_t subscriptionsPerFabric) :
        mOptionalAttributes(optionalAttributes),
        mDeviceInstanceInfoProvider(deviceInstanceInfoProvider), mConfigurationManager(configurationManager),
        mPlatformManager(platformManager), mSubscriptionsPerFabric(subscriptionsPerFabric)
    {
        // UniqueID is mandatory as of spec revision 4. We force it on here regardless
        // of what optionalAttributeSet says, to prevent accidental non-certifiable configs.
        mOptionalAttributes.template Set<BasicInformation::Attributes::UniqueID::Id>();
    }

    const BasicInformationOptionalAttributesSet & GetOptionalAttributes() const { return mOptionalAttributes; }
    BasicInformationOptionalAttributesSet & GetOptionalAttributes() { return mOptionalAttributes; }

    // Delegated methods
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) { return mDeviceInstanceInfoProvider.GetVendorName(buf, bufSize); }

    CHIP_ERROR GetVendorId(uint16_t & vendorId) const { return mDeviceInstanceInfoProvider.GetVendorId(vendorId); }

    CHIP_ERROR GetProductName(char * buf, size_t bufSize) { return mDeviceInstanceInfoProvider.GetProductName(buf, bufSize); }

    CHIP_ERROR GetProductId(uint16_t & productId) const { return mDeviceInstanceInfoProvider.GetProductId(productId); }

    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetPartNumber(buf, bufSize), buf, bufSize);
    }

    CHIP_ERROR GetProductURL(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetProductURL(buf, bufSize), buf, bufSize);
    }

    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetProductLabel(buf, bufSize), buf, bufSize);
    }

    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mDeviceInstanceInfoProvider.GetSerialNumber(buf, bufSize), buf, bufSize);
    }

    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day)
    {
        return mDeviceInstanceInfoProvider.GetManufacturingDate(year, month, day);
    }

    CHIP_ERROR GetManufacturingDateSuffix(MutableCharSpan & suffixBuffer)
    {
        return mDeviceInstanceInfoProvider.GetManufacturingDateSuffix(suffixBuffer);
    }

    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion)
    {
        return mDeviceInstanceInfoProvider.GetHardwareVersion(hardwareVersion);
    }

    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize)
    {
        return mDeviceInstanceInfoProvider.GetHardwareVersionString(buf, bufSize);
    }

    CHIP_ERROR GetProductFinish(app::Clusters::BasicInformation::ProductFinishEnum * finish)
    {
        return mDeviceInstanceInfoProvider.GetProductFinish(finish);
    }

    CHIP_ERROR GetProductPrimaryColor(app::Clusters::BasicInformation::ColorEnum * primaryColor)
    {
        return mDeviceInstanceInfoProvider.GetProductPrimaryColor(primaryColor);
    }

    CHIP_ERROR GetLocalConfigDisabled(bool & localConfigDisabled)
    {
        return mDeviceInstanceInfoProvider.GetLocalConfigDisabled(localConfigDisabled);
    }

    CHIP_ERROR SetLocalConfigDisabled(bool localConfigDisabled)
    {
        return mDeviceInstanceInfoProvider.SetLocalConfigDisabled(localConfigDisabled);
    }

    DeviceLayer::DeviceInstanceInfoProvider::DeviceInfoCapabilityMinimas GetSupportedCapabilityMinimaValues()
    {
        return mDeviceInstanceInfoProvider.GetSupportedCapabilityMinimaValues();
    }

    // ConfigurationManager delegated methods
    CHIP_ERROR GetSoftwareVersion(uint32_t & softwareVersion) { return mConfigurationManager.GetSoftwareVersion(softwareVersion); }

    CHIP_ERROR GetSoftwareVersionString(char * buf, size_t bufSize)
    {
        return mConfigurationManager.GetSoftwareVersionString(buf, bufSize);
    }

    CHIP_ERROR GetUniqueId(char * buf, size_t bufSize)
    {
        return IgnoreUnimplemented(mConfigurationManager.GetUniqueId(buf, bufSize), buf, bufSize);
    }

    CHIP_ERROR GetConfigurationVersion(uint32_t & configurationVersion)
    {
        return mConfigurationManager.GetConfigurationVersion(configurationVersion);
    }

    CHIP_ERROR StoreConfigurationVersion(uint32_t configurationVersion)
    {
        return mConfigurationManager.StoreConfigurationVersion(configurationVersion);
    }

    CHIP_ERROR GetCountryCode(char * buf, size_t bufSize, size_t & codeLen)
    {
        return mConfigurationManager.GetCountryCode(buf, bufSize, codeLen);
    }

    CHIP_ERROR StoreCountryCode(const char * code, size_t codeLen) { return mConfigurationManager.StoreCountryCode(code, codeLen); }

    // PlatformManagerDelegate logic
    void RegisterPlatformDelegate(DeviceLayer::PlatformManagerDelegate * delegate)
    {
        if (mPlatformManager.GetDelegate() == nullptr)
        {
            mPlatformManager.SetDelegate(delegate);
        }
    }

    void UnregisterPlatformDelegate(DeviceLayer::PlatformManagerDelegate * delegate)
    {
        if (mPlatformManager.GetDelegate() == delegate)
        {
            mPlatformManager.SetDelegate(nullptr);
        }
    }

    uint16_t GetSubscriptionsPerFabric() const { return mSubscriptionsPerFabric; }

private:
    CHIP_ERROR IgnoreUnimplemented(CHIP_ERROR status, char * buf, size_t bufSize)
    {
        if (status == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || status == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
        {
            if (bufSize > 0)
            {
                buf[0] = 0;
            }
            return CHIP_NO_ERROR;
        }
        return status;
    }

    BasicInformationOptionalAttributesSet mOptionalAttributes;
    DeviceLayer::DeviceInstanceInfoProvider & mDeviceInstanceInfoProvider;
    DeviceLayer::ConfigurationManager & mConfigurationManager;
    DeviceLayer::PlatformManager & mPlatformManager;
    uint16_t mSubscriptionsPerFabric;
};

} // namespace chip::app::Clusters
