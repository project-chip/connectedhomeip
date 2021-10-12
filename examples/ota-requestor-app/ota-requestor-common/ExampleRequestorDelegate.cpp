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
#include <lib/support/BufferReader.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

constexpr uint32_t kImmediateStartDelayMs = 1;

ExampleRequestorDelegate::ExampleRequestorDelegate()
{
    mOtaStartDelayMs = 0;
    mProviderFabricIndex.SetValue(0);
}

void ExampleRequestorDelegate::Init(chip::Controller::ControllerDeviceInitParams connectParams, uint32_t startDelayMs)
{
    mConnectParams   = connectParams;
    mOtaStartDelayMs = startDelayMs;
}

void ExampleRequestorDelegate::ConnectAndBeginOTA()
{
    VerifyOrReturn(mProviderId.HasValue(), ChipLogError(SoftwareUpdate, "%s: missing Provider ID", __FUNCTION__));
    VerifyOrReturn(mProviderFabricIndex.HasValue(), ChipLogError(SoftwareUpdate, "%s: missing Provider FabricIndex", __FUNCTION__));

    ChipLogProgress(SoftwareUpdate, "When #7976 is fixed, this will attempt to connect to 0x" ChipLogFormatX64 " on FabricId %u",
                    ChipLogValueX64(mProviderId.Value()), mProviderFabricIndex.Value());

    // TODO: uncomment and fill in after #7976 is fixed
    // mProviderDevice.Init(mConnectParams, mProviderId.Value(), address, mProviderFabricIndex.Value());
    // mProviderDevice.EstablishConnectivity();
}

EmberAfStatus ExampleRequestorDelegate::HandleAnnounceOTAProvider(chip::app::CommandHandler * commandObj,
                                                                  chip::NodeId providerLocation, uint16_t vendorId,
                                                                  uint8_t announcementReason, chip::ByteSpan metadataForNode)
{
    mProviderId.SetValue(providerLocation);

    ChipLogDetail(SoftwareUpdate, "notified of Provider at NodeID: 0x" ChipLogFormatX64, ChipLogValueX64(mProviderId.Value()));

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
        break;
    }

    chip::DeviceLayer::SystemLayer().StartTimer(msToStart, StartDelayTimerHandler, this);

    return EMBER_ZCL_STATUS_SUCCESS;
}

void ExampleRequestorDelegate::StartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    VerifyOrReturn(appState != nullptr);
    static_cast<ExampleRequestorDelegate *>(appState)->ConnectAndBeginOTA();
}
