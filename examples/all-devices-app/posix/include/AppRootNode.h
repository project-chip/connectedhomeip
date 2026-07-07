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
#include <platform/CHIPDeviceConfig.h>

#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterfaceRegistry.h>
#include <device/api/Interface.h>
#include <devices/Types.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <device/types/root-node/WifiRootNode.h>
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include <platform/Darwin/WiFi/NetworkCommissioningWiFiDriver.h>
#else
#include <platform/Linux/NetworkCommissioningDriver.h>
#endif
#else
#include <device/types/root-node/RootNode.h>
#endif

namespace chip::app {

/// Provides an application root node for darwin and linux.
///
/// In particular if the SDK supports WIFI, it will also provide the ability
/// to enable wifi in the root node.
class AppRootNode
{
public:
    enum class EnabledFeatures : uint32_t
    {
        kNone = 0,
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        kWiFi = 0x01, // enable wifi support
#endif
    };

    AppRootNode(const RootNode::Context & context, BitFlags<EnabledFeatures> features = {}) :
        mEnabledFeatures(features),
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        mWiFiRootNode(context, { .wifiDriver = mWiFiDriver }),
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
        mRootNode(context)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI && !CHIP_DEVICE_LAYER_TARGET_DARWIN
        mWiFiDriver.Set5gSupport(true);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI && !CHIP_DEVICE_LAYER_TARGET_DARWIN
    }

    SingleEndpoint & RootDevice()
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        if (mEnabledFeatures.Has(EnabledFeatures::kWiFi))
        {
            return mWiFiRootNode;
        }
#endif
        return mRootNode;
    }

    RootNode & GetRootNode() { return mRootNode; }

private:
    BitFlags<EnabledFeatures> mEnabledFeatures;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    DeviceLayer::NetworkCommissioning::DarwinWiFiDriver mWiFiDriver;
#else
    DeviceLayer::NetworkCommissioning::LinuxWiFiDriver mWiFiDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

    WifiRootNode mWiFiRootNode;
#else
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    RootNode mRootNode;
};

} // namespace chip::app
