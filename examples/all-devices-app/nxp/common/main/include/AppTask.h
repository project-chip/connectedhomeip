/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021-2023 Google LLC.
 *    Copyright 2026 NXP
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

#include "AppTaskFreeRTOS.h"

#include <app/TestEventTriggerDelegate.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <binding-handler/BindingHandler.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device-factory/DeviceRegistrationEntry.h>
#include <device/api/allocator/EndpointIdAllocator.h>
#include <device/capabilities/identify/LoggingIdentifyDelegate.h>
#include <device/types/root-node/RootNode.h>
#include <device/types/root-node/RootNodeWith.h>
#include <platform/DefaultTimerDelegate.h>
#include <providers/AllDevicesExampleDeviceInfoProviderImpl.h>

#if CONFIG_NET_L2_OPENTHREAD
#include <device/types/root-node/features/ThreadFeature.h>
#elif CONFIG_CHIP_WIFI
#include <device/types/root-node/features/WifiFeature.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/OTARequestorAttributes.h>
#include <device/types/root-node/features/OtaFeature.h>
#endif

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

class AppTask : public chip::NXP::App::AppTaskFreeRTOS
{
public:
    static AppTask & GetDefaultInstance();

    ~AppTask() override{};
    void PreInitMatterStack(void) override;
    void PostInitMatterStack(void) override;
    void AppMatter_RegisterCustomCliCommands(void) override;

    /**
     * Prepares EP0 and the runtime device factory on the Matter thread before Server::Init.
     * The application singleton owns the model for the server lifetime. Preparation is one-shot,
     * including on failure; a reboot is required before another attempt.
     */
    CHIP_ERROR PrepareCodeDrivenDataModel(chip::CommonCaseDeviceServerInitParams & params) override;
    void PostInitMatterServerInstance() override;

    /** Appends an instance on the Matter thread after successful Server::Init. */
    CHIP_ERROR RegisterAppDevice(const std::string & type);

private:
    CHIP_ERROR InitPersistence();
    CHIP_ERROR InitRootNode();
    CHIP_ERROR PostServerInit();

#if CONFIG_NET_L2_OPENTHREAD
    using NetworkFeature = chip::app::ThreadFeature;
#elif CONFIG_CHIP_WIFI
    using NetworkFeature = chip::app::WifiFeature;
#endif

#if CONFIG_NET_L2_OPENTHREAD || CONFIG_CHIP_WIFI
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    using RootNodeType = chip::app::RootNodeWith<NetworkFeature, chip::app::OtaFeature>;
#else
    using RootNodeType = chip::app::RootNodeWith<NetworkFeature>;
#endif
#elif CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    using RootNodeType   = chip::app::RootNodeWith<chip::app::OtaFeature>;
#else
    using RootNodeType = chip::app::RootNode;
#endif

    static constexpr chip::EndpointId kDeviceEndpointId = 1;
    static constexpr size_t kMaxRuntimeDevices          = 32;

    // A wider counter prevents allocation from wrapping into EP0.
    class RuntimeEndpointIdAllocator : public chip::app::EndpointIdAllocator
    {
    public:
        chip::EndpointId Allocate() override;
        uint32_t NextEndpointId() const { return mNext; }

    private:
        uint32_t mNext = kDeviceEndpointId;
    };

    chip::CommonCaseDeviceServerInitParams & mInitParams = GetServerInitParams();
    chip::app::DefaultAttributePersistenceProvider mAttributePersistenceProvider;
    chip::DeviceLayer::AllDevicesExampleDeviceInfoProviderImpl mDeviceInfoProvider;
    chip::app::BindingHandler mBindingHandler;
    chip::SimpleTestEventTriggerDelegate mDefaultTestEventTriggerDelegate;
    chip::app::LoggingIdentifyDelegate mDefaultIdentifyDelegate;
    chip::app::DefaultTimerDelegate mTimerDelegate;
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    chip::OTARequestorAttributes mOTARequestorAttributes;
#endif
    std::unique_ptr<RootNodeType> mRootNode;
    std::array<chip::app::DeviceRegistrationEntry, kMaxRuntimeDevices> mDevices;
    size_t mDeviceCount = 0;
    RuntimeEndpointIdAllocator mAllocator;
    bool mPreparationStarted = false;
    bool mRuntimeReady       = false;
    // The provider borrows persistence and registrations, so it is destroyed first.
    std::unique_ptr<chip::app::CodeDrivenDataModelProvider> mDataModelProvider;
};
