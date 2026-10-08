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

#include <AppRootNode.h>
#include <app/DeviceLoadStatusProvider.h>
#include <app/EventManagement.h>
#include <app/FailSafeContext.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Dnssd.h>
#include <app_options/DeviceTypeParser.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/Interface.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TimerDelegate.h>
#include <platform/ConfigurationManager.h>
#include <platform/DeviceControlServer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <transport/SessionManager.h>

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
#include <app/server/TermsAndConditionsProvider.h> // nogncheck
#endif                                             // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED

#include <cstdint>
#include <memory>
#include <vector>

namespace chip::app {

/**
 * Manages the lifecycle of the code-driven data model provider and all device
 * endpoint instances for the application.
 *
 * This class owns:
 * - The backing `CodeDrivenDataModelProvider` and its attribute persistence provider.
 * - The root node device (`AppRootNode`) registered on endpoint 0 (`kRootEndpointId`).
 * - The collection of application device instances (`DeviceInterface`) constructed
 *   dynamically via `PosixDeviceFactory` from command-line `DeviceTypeParser::Entry`
 *   specifications.
 *
 * Usage:
 * 1. Instantiate with a `Context` containing the required server/platform dependencies.
 * 2. Call `Startup(deviceEntries)` before `Server::Init()` to initialize persistence,
 *    register the root node, and construct/register all requested device endpoints.
 * 3. Pass `&DataModelProvider()` to `ServerInitParams::dataModelProvider`.
 * 4. Call `Shutdown()` during application teardown to unregister and destroy all
 *    constructed devices and the root node.
 */
class DeviceInstances
{
public:
    struct Context
    {
        chip::PersistentStorageDelegate & storageDelegate;
        CommissioningWindowManager & commissioningWindowManager;
        DeviceLayer::ConfigurationManager & configurationManager;
        DeviceLayer::DeviceControlServer & deviceControlServer;
        FabricTable & fabricTable;
        Access::AccessControl & accessControl;
        FailSafeContext & failSafeContext;
        DeviceLayer::DeviceInstanceInfoProvider & deviceInstanceInfoProvider;
        DeviceLayer::PlatformManager & platformManager;
        Credentials::GroupDataProvider & groupDataProvider;
        SessionManager & sessionManager;
        DnssdServer & dnssdServer;
        DeviceLoadStatusProvider & deviceLoadStatusProvider;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        TestEventTriggerDelegate & testEventTriggerDelegate;
        Clusters::Binding::Table & bindingTable;
        Clusters::Binding::Manager & bindingManager;
        Clusters::IdentifyDelegate & identifyDelegate;
        Credentials::DeviceAttestationCredentialsProvider & dacProvider;
        EventManagement & eventManagement;
        TimerDelegate & timerDelegate;
        uint16_t minGuaranteedSubscriptionsPerFabric;
        bool enableWiFi = false;
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        TermsAndConditionsProvider & termsAndConditionsProvider;
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    };

    explicit DeviceInstances(const Context & context);

    CHIP_ERROR Startup(const std::vector<DeviceTypeParser::Entry> & deviceEntries);

    void Shutdown();

    chip::app::CodeDrivenDataModelProvider & DataModelProvider() { return mDataModelProvider; }

    AppRootNode & RootNode() { return mRootNode; }

    const std::vector<std::unique_ptr<DeviceInterface>> & GetConstructedDevices() const { return mConstructedDevices; }

private:
    Context mContext;
    chip::app::DefaultAttributePersistenceProvider mAttributePersistence;
    chip::app::CodeDrivenDataModelProvider mDataModelProvider;

    AppRootNode mRootNode;
    std::vector<std::unique_ptr<DeviceInterface>> mConstructedDevices;
};

} // namespace chip::app
