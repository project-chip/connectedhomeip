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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/util/af-enums.h>
#include <credentials/FabricTable.h>

// An example implementation for how an application might handle receiving an AnnounceOTAProvider command. In this case, the
// AnnounceOTAProvider command will be used as a trigger to send a QueryImage command and begin the OTA process. This class also
// contains other application-specific logic related to OTA Software Update.
class ExampleOTARequestor
{
public:
    static ExampleOTARequestor & GetInstance() { return sInstance; }

    void Init(uint32_t startDelayMs);

    EmberAfStatus HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData);

    // Setter for mConnectToProviderCallback
    void SetConnectToProviderCallback(void (*f)(chip::NodeId, chip::FabricIndex)) { mConnectToProviderCallback = f; }

private:
    ExampleOTARequestor();

    static void StartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState);
    void ConnectToProvider();

    static ExampleOTARequestor sInstance;

    chip::NodeId mProviderNodeId;
    chip::FabricIndex mProviderFabricIndex;
    uint32_t mOtaStartDelayMs;

    // TODO: This will be redone once the full Requestor app design is in place
    // Pointer to the function that establishes a session with the Provider and initiates
    // the BDX download
    void (*mConnectToProviderCallback)(chip::NodeId, chip::FabricIndex);
};
