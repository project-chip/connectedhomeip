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

/* This file contains the declaration for the OTA Requestor interface.
 * Any implementation of the OTA Requestor (e.g. the OTARequestor class) must implement
 * this interface.
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/util/af-enums.h>

#pragma once

namespace chip {

// Interface class to connect the OTA Software Update Requestor cluster command processing
// with the core OTA Requestor logic. The OTARequestor class implements this interface
class OTARequestorInterface
{
public:
    // Return value for various trigger-type APIs
    enum OTATriggerResult
    {
        kTriggerSuccessful = 0,
        kNoProviderKnown   = 1
    };

    // Handler for the AnnounceOTAProvider command
    virtual EmberAfStatus HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData) = 0;

    // TBD: This probably doesn't need to be a method OTARequestorInterface as the response handler is
    // explicitly supplied at command invocation
    // Handler for the QueryImageResponse command
    // virtual bool
    // HandleQueryImageResponse(chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType) = 0;

    // Destructor
    virtual ~OTARequestorInterface() = default;

    // Send QueryImage command
    virtual OTATriggerResult TriggerImmediateQuery() = 0;

    // Download image
    virtual void DownloadUpdate() = 0;

    // Send ApplyImage command
    virtual void ApplyUpdate() = 0;

    // Send NotifyUpdateApplied command
    virtual void NotifyUpdateApplied(uint32_t version) = 0;

    // Get image update progress in percents unit
    virtual CHIP_ERROR GetUpdateProgress(EndpointId endpointId, chip::app::DataModel::Nullable<uint8_t> & progress) = 0;

    // Get requestor state
    virtual CHIP_ERROR GetState(EndpointId endpointId,
                                chip::app::Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum & state) = 0;

    // Manually set OTA Provider parameters
    virtual void TestModeSetProviderParameters(NodeId nodeId, FabricIndex fabIndex, EndpointId endpointId) = 0;

    // Application directs the Requestor to cancel image update in progress. All the Requestor state is
    // cleared, UpdateState is reset to Idle
    virtual void CancelImageUpdate() = 0;
};

// The instance of the class implementing OTARequestorInterface must be managed through
// the following global getter and setter functions.

// Set the object implementing OTARequestorInterface
void SetRequestorInstance(OTARequestorInterface * instance);

// Get the object implementing OTARequestorInterface
OTARequestorInterface * GetRequestorInstance();

} // namespace chip
