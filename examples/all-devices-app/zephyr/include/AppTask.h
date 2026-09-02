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

#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Server.h>
#include <credentials/GroupDataProviderImpl.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/Interface.h>
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
 * `Run()` is not virtual and fixes the order of initialization. Each step is
 * a virtual method with a working default that could be overriden.
 */
class AppTask
{
public:
    virtual ~AppTask() = default;

    /// Runs every initialization step in order, then starts the CHIP event
    /// loop. Returns on the first failure.
    CHIP_ERROR Run();

protected:
    virtual CHIP_ERROR InitPlatform();

    // SetDeviceAttestationCredentialsProvider and SetDeviceInstanceInfoProvider
    virtual CHIP_ERROR InitCredentials();

    // Dispatches to InitThreadNetworking() / InitWifiNetworking(); both are
    virtual CHIP_ERROR InitNetwork();

    virtual CHIP_ERROR InitThreadNetworking();

    // TODO: no Wi-Fi implementation yet
    virtual CHIP_ERROR InitWifiNetworking();

    /// Storage/group data provider setup.
    virtual CHIP_ERROR InitPersistence();

    virtual CHIP_ERROR InitRootNode();

    /// OTA Requestor cluster on the root endpoint, if enabled.
    virtual CHIP_ERROR RegisterOTACluster();

    /// Instantiates every type in DeviceFactory::SupportedDeviceTypes().
    virtual CHIP_ERROR RegisterAppDevices();

    virtual CHIP_ERROR InitServer();

    // Factory reset button
    virtual CHIP_ERROR InitBoardControls();

    virtual CHIP_ERROR PostServerInit();

    virtual CHIP_ERROR InitOTARequestor();

    chip::CommonCaseDeviceServerInitParams mInitParams;
    Credentials::GroupDataProviderImpl mGroupDataProvider;
    DefaultTimerDelegate mTimerDelegate;
    DefaultAttributePersistenceProvider mAttributePersistenceProvider;

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

/// Returns the single application task. Defined in AppTaskInstance.cpp, which may be replaced through APP_TASK_INSTANCE_SOURCE.
AppTask & GetAppTask();

} // namespace chip::app::AllDevices
