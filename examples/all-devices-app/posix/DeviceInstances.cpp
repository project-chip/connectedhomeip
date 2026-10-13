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

#include <DeviceInstances.h>

#include <PosixDeviceFactory.h>
#include <device/api/SingleEndpoint.h>
#include <device/api/allocator/DynamicEndpointIdAllocator.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <oob-accessors/OOBAccessorRegistry.h>

#include <set>
#include <utility>

namespace chip::app {

namespace {

std::set<EndpointId> GetReservedEndpointIds(const std::vector<DeviceTypeParser::Entry> & deviceEntries)
{
    std::set<EndpointId> usedIds;
    usedIds.insert(kRootEndpointId);

    for (const auto & entry : deviceEntries)
    {
        if (entry.endpoint != kInvalidEndpointId)
        {
            usedIds.insert(entry.endpoint);
        }
    }
    return usedIds;
}

} // namespace

DeviceInstances::DeviceInstances(const Context & context) :
    mContext(context), mDataModelProvider(mContext.storageDelegate, mAttributePersistence),
    mRootNode(
        {
            .commissioningWindowManager              = mContext.commissioningWindowManager, //
                .configurationManager                = mContext.configurationManager,       //
                .deviceControlServer                 = mContext.deviceControlServer,        //
                .fabricTable                         = mContext.fabricTable,                //
                .accessControl                       = mContext.accessControl,              //
                .persistentStorage                   = mContext.storageDelegate,            //
                .failSafeContext                     = mContext.failSafeContext,            //
                .deviceInstanceInfoProvider          = mContext.deviceInstanceInfoProvider, //
                .platformManager                     = mContext.platformManager,            //
                .groupDataProvider                   = mContext.groupDataProvider,          //
                .sessionManager                      = mContext.sessionManager,             //
                .dnssdServer                         = mContext.dnssdServer,                //
                .deviceLoadStatusProvider            = mContext.deviceLoadStatusProvider,   //
                .diagnosticDataProvider              = mContext.diagnosticDataProvider,     //
                .testEventTriggerDelegate            = &mContext.testEventTriggerDelegate,  //
                .dacProvider                         = mContext.dacProvider,                //
                .eventManagement                     = mContext.eventManagement,            //
                .timerDelegate                       = mContext.timerDelegate,              //
                .minGuaranteedSubscriptionsPerFabric = mContext.minGuaranteedSubscriptionsPerFabric,
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            .termsAndConditionsProvider = mContext.termsAndConditionsProvider,
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        },
        []([[maybe_unused]] bool enableWiFi) {
            BitFlags<AppRootNode::EnabledFeatures> features;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
            features.Set(AppRootNode::EnabledFeatures::kWiFi, enableWiFi);
#endif
            return features;
        }(mContext.enableWiFi))
{}

CHIP_ERROR DeviceInstances::Startup(const std::vector<DeviceTypeParser::Entry> & deviceEntries)
{
    ReturnErrorOnFailure(mAttributePersistence.Init(&mContext.storageDelegate));

    DynamicEndpointIdAllocator endpointIdAllocator(GetReservedEndpointIds(deviceEntries));
    endpointIdAllocator.ForceNext(kRootEndpointId);
    ReturnErrorOnFailure(mRootNode.RootDevice().Register(endpointIdAllocator, mDataModelProvider));

    PosixDeviceFactory::GetInstance().Init(PosixDeviceFactory::Context{
        .groupDataProvider        = mContext.groupDataProvider,
        .fabricTable              = mContext.fabricTable,
        .timerDelegate            = mContext.timerDelegate,
        .storageDelegate          = mContext.storageDelegate,
        .diagnosticDataProvider   = mContext.diagnosticDataProvider,
        .platformManager          = mContext.platformManager,
        .failSafeContext          = mContext.failSafeContext,
        .breadcrumbTracker        = mRootNode.RootDevice().GeneralCommissioning(),
        .bindingTable             = mContext.bindingTable,
        .bindingManager           = mContext.bindingManager,
        .testEventTriggerDelegate = mContext.testEventTriggerDelegate,
        .identifyDelegate         = mContext.identifyDelegate,
    });
    PosixDeviceFactory::ExecuteHooks(mRootNode.RootDevice());

    for (const auto & entry : deviceEntries)
    {
        auto created = PosixDeviceFactory::GetInstance().Create(entry.type, entry.label);

        VerifyOrReturnError(created.device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        ChipLogProgress(AppServer, "Registering device %s on endpoint %u with parent 0x%04X", entry.type.c_str(), entry.endpoint,
                        entry.parentId);
        if (entry.endpoint != kInvalidEndpointId)
        {
            endpointIdAllocator.ForceNext(entry.endpoint);
        }
        ReturnErrorOnFailure(
            created.device->Register(endpointIdAllocator, mDataModelProvider, EndpointComposition::WithParent(entry.parentId)));
        if (created.onDeviceRegistered)
        {
            created.onDeviceRegistered();
        }
        mConstructedDevices.push_back(std::move(created.device));
    }

    return CHIP_NO_ERROR;
}

void DeviceInstances::Shutdown()
{
    OOBAccessorRegistry::Instance().Clear();
    for (auto & device : mConstructedDevices)
    {
        device->Unregister(mDataModelProvider);
    }
    mConstructedDevices.clear();
    mRootNode.RootDevice().Unregister(mDataModelProvider);
}

} // namespace chip::app
