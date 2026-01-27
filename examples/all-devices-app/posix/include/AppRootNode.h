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
#include <devices/Types.h>
#include <devices/interface/DeviceInterface.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <devices/root-node/WifiRootNodeDevice.h>
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include <platform/Darwin/WiFi/NetworkCommissioningWiFiDriver.h>
#else
#include <platform/Linux/NetworkCommissioningDriver.h>
#endif
#else
#include <devices/root-node/RootNodeDevice.h>
#endif

namespace chip::app {

class AppRootNode : public DeviceInterface
{
public:
    AppRootNode(const RootNodeDevice::Context & context) :
        DeviceInterface(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRootNode, 1)),
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        mRootNodeDevice(context, { .wifiDriver = mWiFiDriver })
    {
        mWiFiDriver.Set5gSupport(true);
    }
#else
        mRootNodeDevice(context)
    {
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override
    {
        return mRootNodeDevice.Register(endpoint, provider, parentId);
    }

    /// Removes a device's clusters from the given provider. This
    /// must only be called when register has succeeded before. Expected
    /// usage of this function is for when the device is no longer needed
    /// (for example, on shutdown), to destroy the device's clusters.
    void UnRegister(CodeDrivenDataModelProvider & provider) override { return mRootNodeDevice.UnRegister(provider); }

    CHIP_ERROR DeviceTypes(ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out) const override
    {
        return mRootNodeDevice.DeviceTypes(out);
    }

    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override { return mRootNodeDevice.ClientClusters(out); }

private:
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    DeviceLayer::NetworkCommissioning::DarwinWiFiDriver mWiFiDriver;
#else
    DeviceLayer::NetworkCommissioning::LinuxWiFiDriver mWiFiDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

    WifiRootNodeDevice mRootNodeDevice;
#else
    RootNodeDevice mRootNodeDevice;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
};

} // namespace chip::app
