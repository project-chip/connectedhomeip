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

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Server.h>
#include <credentials/GroupDataProviderImpl.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/Interface.h>
#include <device/capabilities/identify/LoggingIdentifyDelegate.h>
#include <lib/core/CHIPError.h>
#include <platform/DefaultTimerDelegate.h>
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/Zephyr/wifi/ZephyrWifiDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/OTARequestorAttributes.h>
#include <app/clusters/ota-requestor/OTARequestorCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR

#include <memory>

namespace chip::app::AllDevices {

/**
 * Generic Zephyr Matter application task.
 *
 * This is a CRTP base. `Derived` is the final application type. `Run()` sets the
 * order of the initialization steps and calls each step through `Self()`. To
 * replace a step, declare a step with the same name in a derived class.
 *
 * Make your custom layer a template. Then you can stack the layers:
 *
 *     template <class D> class CustomAppTask : public AppTaskBase<D> { ... };
 *     class AppTask final : public CustomAppTask<AppTask> {};
 *
 * To also run the generic behaviour, call `AppTaskBase<D>::Step()` from your step.
 */
template <class Derived>
class AppTaskBase
{
public:
    /// Runs every initialization step in order, then starts the CHIP event
    /// loop. Returns on the first failure.
    CHIP_ERROR Run();

    CHIP_ERROR InitPlatform();

    // SetDeviceAttestationCredentialsProvider and SetDeviceInstanceInfoProvider
    CHIP_ERROR InitCredentials();

    // Dispatches to InitThreadNetworking() / InitWifiNetworking().
    CHIP_ERROR InitNetwork();

    CHIP_ERROR InitThreadNetworking();

    // TODO: no Wi-Fi implementation yet
    CHIP_ERROR InitWifiNetworking();

    /// Storage/group data provider setup.
    CHIP_ERROR InitPersistence();

    TestEventTriggerDelegate & GetTestEventTriggerDelegate();

    /// Identify delegate handed to every device created by the DeviceFactory.
    Clusters::IdentifyDelegate & GetIdentifyDelegate();

    CHIP_ERROR InitRootNode();

    /// OTA Requestor cluster on the root endpoint, if enabled.
    CHIP_ERROR RegisterOTACluster();

    /// Instantiates every type in DeviceFactory::SupportedDeviceTypes().
    CHIP_ERROR RegisterAppDevices();

    CHIP_ERROR InitServer();

    // Factory reset button
    CHIP_ERROR InitBoardControls();

    CHIP_ERROR PostServerInit();

    CHIP_ERROR InitOTARequestor();

protected:
    static constexpr EndpointId kRootEndpointId        = 0;
    static constexpr EndpointId kFirstDeviceEndpointId = 1;

    Derived & Self() { return static_cast<Derived &>(*this); }

    chip::CommonCaseDeviceServerInitParams mInitParams;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    SimpleTestEventTriggerDelegate mDefaultTestEventTriggerDelegate;
    LoggingIdentifyDelegate mDefaultIdentifyDelegate;
    DefaultTimerDelegate mTimerDelegate;
    DefaultAttributePersistenceProvider mAttributePersistenceProvider;
    DefaultSafeAttributePersistenceProvider mSafeAttributePersistenceProvider;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    DeviceLayer::NetworkCommissioning::GenericThreadDriver mThreadDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    DeviceLayer::NetworkCommissioning::ZephyrWifiDriver mWifiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    std::unique_ptr<CodeDrivenDataModelProvider> mDataModelProvider;
    std::unique_ptr<DeviceInterface> mRootNode;

    std::unique_ptr<DeviceInterface> mDevices[ALL_DEVICES_ENABLED_DEVICE_COUNT];
    size_t mDeviceCount = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    chip::DefaultOTARequestor mOTARequestorCore;
    chip::DefaultOTARequestorStorage mOTARequestorStorage;
    DeviceLayer::DefaultOTARequestorDriver mOTARequestorDriver;
    chip::BDXDownloader mOTADownloader;
    chip::OTARequestorAttributes mOTARequestorAttributes;
    LazyRegisteredServerCluster<Clusters::OTARequestorCluster> mOTARequestorCluster;
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
};

} // namespace chip::app::AllDevices

#include "AppTaskBase.ipp"
