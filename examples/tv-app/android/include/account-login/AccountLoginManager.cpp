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
#include <app/util/config.h>
#include <json/json.h>
#include <lib/core/DataModelTypes.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AccountLogin;
using Status = chip::Protocols::InteractionModel::Status;

namespace {

const auto loginTempAccountIdentifierFieldId =
    std::to_string(chip::to_underlying(AccountLogin::Commands::Login::Fields::kTempAccountIdentifier));
const auto loginSetupPINFieldId = std::to_string(chip::to_underlying(AccountLogin::Commands::Login::Fields::kSetupPIN));
const auto loginNodeFieldId     = std::to_string(chip::to_underlying(AccountLogin::Commands::Login::Fields::kNode));
const auto logoutNodeFieldId    = std::to_string(chip::to_underlying(AccountLogin::Commands::Logout::Fields::kNode));

std::string charSpanToString(const CharSpan & charSpan)
{
    return { charSpan.data(), charSpan.size() };
}

std::string serializeLoginCommand(AccountLogin::Commands::Login::Type cmd)
{
    return R"({")" + loginTempAccountIdentifierFieldId + R"(":")" + charSpanToString(cmd.tempAccountIdentifier) + R"(",)" + R"(")" +
        loginSetupPINFieldId + R"(":")" + charSpanToString(cmd.setupPIN) + R"(",)" + R"(")" + loginNodeFieldId + R"(":")" +
        std::to_string(cmd.node.Value()) + R"("})";
}

std::string serializeLogoutCommand(AccountLogin::Commands::Logout::Type cmd)
{
    return R"({")" + logoutNodeFieldId + R"(":")" + std::to_string(cmd.node.Value()) + R"("})";
}

} // namespace

AccountLoginManager::AccountLoginManager(ContentAppCommandDelegate * commandDelegate, const char * setupPin) :
    mCommandDelegate(commandDelegate)
{
    CopyString(mSetupPin, sizeof(mSetupPin), setupPin);
}

bool AccountLoginManager::HandleLogin(const CharSpan & tempAccountIdentifier, const CharSpan & setupPIN,
                                      const chip::Optional<chip::NodeId> & nodeId)
{
    ChipLogProgress(DeviceLayer, "AccountLoginManager::HandleLogin called for endpoint %d", mEndpointId);

    if (mCommandDelegate == nullptr)
    {
        ChipLogError(Zcl, "CommandDelegate not found");
        return false;
    }

    if (tempAccountIdentifier.empty() || setupPIN.empty() || !nodeId.HasValue())
    {
        ChipLogError(Zcl, "Invalid parameters");
        return false;
    }

    Json::Value response;
    bool commandHandled                     = true;
    AccountLogin::Commands::Login::Type cmd = { tempAccountIdentifier, setupPIN, nodeId };

    auto status = mCommandDelegate->InvokeCommand(mEndpointId, AccountLogin::Id, AccountLogin::Commands::Login::Id,
                                                  serializeLoginCommand(cmd), commandHandled, response);
    if (status == Status::Success)
    {
        // Format status response to verify that response is non-failure.
        status = mCommandDelegate->FormatStatusResponse(response);
    }
    ChipLogProgress(Zcl, "AccountLoginManager::HandleLogin command returned with status: %d", chip::to_underlying(status));
    return status == chip::Protocols::InteractionModel::Status::Success;
}

bool AccountLoginManager::HandleLogout(const chip::Optional<chip::NodeId> & nodeId)
{
    ChipLogProgress(DeviceLayer, "AccountLoginManager::HandleLogout called for endpoint %d", mEndpointId);

    if (mCommandDelegate == nullptr)
    {
        ChipLogError(Zcl, "CommandDelegate not found");
        return false;
    }

    if (!nodeId.HasValue())
    {
        ChipLogError(Zcl, "Invalid parameters");
        return false;
    }

    Json::Value response;
    bool commandHandled                      = true;
    AccountLogin::Commands::Logout::Type cmd = { nodeId };

    auto status = mCommandDelegate->InvokeCommand(mEndpointId, AccountLogin::Id, AccountLogin::Commands::Logout::Id,
                                                  serializeLogoutCommand(cmd), commandHandled, response);

    if (status == Status::Success)
    {
        // Format status response to verify that response is non-failure.
        status = mCommandDelegate->FormatStatusResponse(response);
    }
    ChipLogProgress(Zcl, "AccountLoginManager::HandleLogout command returned with status: %d", chip::to_underlying(status));
    return status == chip::Protocols::InteractionModel::Status::Success;
}

void AccountLoginManager::HandleGetSetupPin(CommandResponseHelper<GetSetupPINResponse> & helper,
                                            const CharSpan & tempAccountIdentifier)
{
    std::string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());

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
    std::string tempAccountIdentifierString(tempAccountIdentifier.data(), tempAccountIdentifier.size());
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
