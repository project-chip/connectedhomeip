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

#pragma once

#include <app/Command.h>
#include <app/clusters/ota-requestor/ota-requestor-delegate.h>
#include <app/util/basic-types.h>
#include <controller/CHIPDevice.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>

using chip::app::clusters::OTARequestorDelegate;

// An example implementation for how an application might handle receiving an AnnounceOTAProvider command. In this case, the
// AnnounceOTAProvider command will be used as a trigger to send a QueryImage command and begin the OTA process. This class also
// contains other application-specific logic related to OTA Software Update.
class ExampleRequestorDelegate : public OTARequestorDelegate
{
public:
    ExampleRequestorDelegate();

    void Init(chip::Controller::ControllerDeviceInitParams connectParams, uint32_t startDelayMs);

    // Inherited from OTAProviderDelegate
    EmberAfStatus HandleAnnounceOTAProvider(chip::app::CommandHandler * commandObj, chip::NodeId providerLocation,
                                            uint16_t vendorId, uint8_t announcementReason, chip::ByteSpan metadataForNode) override;

private:
    void ConnectAndBeginOTA();
    static void StartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState);

    chip::Controller::Device mProviderDevice;
    chip::Controller::ControllerDeviceInitParams mConnectParams;
    chip::Optional<chip::NodeId> mProviderId;
    chip::Optional<chip::FabricIndex> mProviderFabricIndex;
    uint32_t mOtaStartDelayMs;
};
