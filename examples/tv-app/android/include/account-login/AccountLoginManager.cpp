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

#include "AccountLoginManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/util/af.h>
#include <json/json.h>
#include <lib/core/DataModelTypes.h>

using namespace std;
using namespace chip::app::Clusters::AccountLogin;
using Status = chip::Protocols::InteractionModel::Status;

AccountLoginManager::AccountLoginManager(ContentAppCommandDelegate * commandDelegate, const char * setupPin) :
    mCommandDelegate(commandDelegate)
{
    CopyString(mSetupPin, sizeof(mSetupPin), setupPin);
}

bool AccountLoginManager::HandleLogin(const CharSpan & tempAccountIdentifier, const CharSpan & setupPin,
                                      const chip::Optional<chip::NodeId> & nodeId)
{
    ChipLogProgress(DeviceLayer, "AccountLoginManager::HandleLogin called for endpoint %d", mEndpointId);
    string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
    string setupPinString(setupPin.data(), setupPin.size());

    if (strcmp(mSetupPin, setupPinString.c_str()) == 0)
    {
        ChipLogProgress(Zcl, "AccountLoginManager::HandleLogin success");
        return true;
    }
    else
    {
        ChipLogProgress(Zcl, "AccountLoginManager::HandleLogin failed expected pin %s", mSetupPin);
        return false;
    }
}

bool AccountLoginManager::HandleLogout(const chip::Optional<chip::NodeId> & nodeId)
{
    // TODO: Insert your code here to send logout request
    return true;
}

void AccountLoginManager::HandleGetSetupPin(CommandResponseHelper<GetSetupPINResponse> & helper,
                                            const CharSpan & tempAccountIdentifier)
{
    string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());

    GetSetupPINResponse response;
    ChipLogProgress(Zcl, "temporary account id: %s returning pin: %s", tempAccountIdentifierString.c_str(), mSetupPin);

    response.setupPIN = CharSpan::fromCharString(mSetupPin);
    helper.Success(response);
}

void AccountLoginManager::GetSetupPin(char * setupPin, size_t setupPinSize, const CharSpan & tempAccountIdentifier)
{
    // Only this method is called outside of the normal Matter communication with endpoints and clusters.
    // Hence we have to introduce call to the java layer(thorough the command delegate) here.
    // Other methods in this class do not need to be changed beecause those will get routed to java layer
    // upstream.
    ChipLogProgress(DeviceLayer, "AccountLoginManager::GetSetupPin called for endpoint %d", mEndpointId);
    string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
    if (mCommandDelegate == nullptr)
    {
        // For the dummy content apps to work.
        CopyString(setupPin, setupPinSize, mSetupPin);
        ChipLogProgress(Zcl, "Returning pin for dummy content app");
        return;
    }

    Json::Value response;
    bool commandHandled = true;
    mCommandDelegate->InvokeCommand(mEndpointId, chip::app::Clusters::AccountLogin::Id,
                                    chip::app::Clusters::AccountLogin::Commands::GetSetupPIN::Id,
                                    "{\"0\": \"" + tempAccountIdentifierString + "\"}", commandHandled, response);
    Status status;
    GetSetupPINResponse pinResponse = mCommandDelegate->FormatGetSetupPINResponse(response, status);
    if (status == chip::Protocols::InteractionModel::Status::Success)
    {
        CopyString(setupPin, setupPinSize, pinResponse.setupPIN);
    }
    else
    {
        CopyString(setupPin, setupPinSize, "");
    }
    ChipLogProgress(Zcl, "Returning pin for content app for endpoint %d", mEndpointId);
};

uint16_t AccountLoginManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "AccountLoginManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}
