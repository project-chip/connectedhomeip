/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <ExampleRequestorDelegate.h>

#include <app-common/zap-generated/enums.h>
#include <app/util/af-enums.h>
#include <controller/CHIPDevice.h>
#include <lib/core/NodeId.h>
#include <lib/support/BufferReader.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/FabricTable.h>

using chip::FabricInfo;

constexpr uint32_t kImmediateStartDelayMs = 1;

ExampleRequestorDelegate::ExampleRequestorDelegate()
{
    mOtaStartDelayMs     = 0;
    mProviderId          = chip::kUndefinedNodeId;
    mProviderFabricIndex = chip::kUndefinedFabricIndex;
}

void ExampleRequestorDelegate::Init(chip::Controller::ControllerDeviceInitParams connectParams, uint32_t startDelayMs)
{
    mConnectParams   = connectParams;
    mOtaStartDelayMs = startDelayMs;
}

void ExampleRequestorDelegate::ConnectToProvider()
{
    FabricInfo * providerFabric = GetProviderFabricInfo();
    VerifyOrReturn(providerFabric != nullptr,
                   ChipLogError(SoftwareUpdate, "No Fabric found for index %" PRIu8, mProviderFabricIndex));

    ChipLogProgress(SoftwareUpdate,
                    "Once #7976 is fixed, this would attempt to connect to 0x" ChipLogFormatX64 " on FabricIndex 0x%" PRIu8
                    " (" ChipLogFormatX64 ")",
                    ChipLogValueX64(mProviderId), mProviderFabricIndex, ChipLogValueX64(providerFabric->GetFabricId()));

    // TODO: uncomment and fill in after #7976 is fixed
    // mProviderDevice.Init(mConnectParams, mProviderId, address, mProviderFabricIndex);
    // mProviderDevice.EstablishConnectivity();
}

EmberAfStatus ExampleRequestorDelegate::HandleAnnounceOTAProvider(chip::app::CommandHandler * commandObj,
                                                                  chip::NodeId providerLocation, uint16_t vendorId,
                                                                  uint8_t announcementReason, chip::ByteSpan metadataForNode)
{
    if (commandObj == nullptr || commandObj->GetExchangeContext() == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access ExchangeContext for FabricIndex");
        return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
    }

    mProviderId          = providerLocation;
    mProviderFabricIndex = commandObj->GetExchangeContext()->GetSecureSession().GetFabricIndex();

    FabricInfo * providerFabric = GetProviderFabricInfo();
    if (providerFabric == nullptr)
    {
        ChipLogError(SoftwareUpdate, "No Fabric found for index %" PRIu8, mProviderFabricIndex);
        return EMBER_ZCL_STATUS_SUCCESS;
    }

    ChipLogProgress(SoftwareUpdate,
                    "Notified of Provider at NodeID: 0x" ChipLogFormatX64 "on FabricIndex 0x%" PRIu8 " (" ChipLogFormatX64 ")",
                    ChipLogValueX64(mProviderId), mProviderFabricIndex, ChipLogValueX64(providerFabric->GetFabricId()));

    // If reason is URGENT_UPDATE_AVAILABLE, we start OTA immediately. Otherwise, respect the timer value set in mOtaStartDelayMs.
    // This is done to exemplify what a real-world OTA Requestor might do while also being configurable enough to use as a test app.
    uint32_t msToStart = 0;
    switch (announcementReason)
    {
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_SIMPLE_ANNOUNCEMENT):
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_UPDATE_AVAILABLE):
        msToStart = mOtaStartDelayMs;
        break;
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_URGENT_UPDATE_AVAILABLE):
        msToStart = kImmediateStartDelayMs;
        break;
    default:
        ChipLogError(SoftwareUpdate, "Unexpected announcementReason: %" PRIu8, static_cast<uint8_t>(announcementReason));
        return EMBER_ZCL_STATUS_INVALID_ARGUMENT;
    }

    chip::DeviceLayer::SystemLayer().StartTimer(msToStart, StartDelayTimerHandler, this);

    return EMBER_ZCL_STATUS_SUCCESS;
}

void ExampleRequestorDelegate::StartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    VerifyOrReturn(appState != nullptr);
    static_cast<ExampleRequestorDelegate *>(appState)->ConnectToProvider();
}

chip::FabricInfo * ExampleRequestorDelegate::GetProviderFabricInfo()
{
    if (mConnectParams.fabricsTable == nullptr)
    {
        ChipLogError(SoftwareUpdate, "FabricTable is null!");
        return nullptr;
    }

    return mConnectParams.fabricsTable->FindFabricWithIndex(mProviderFabricIndex);
}
