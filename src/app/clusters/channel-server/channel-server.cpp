/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *
 *    Copyright (c) 2021 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief Routines for the Channel plugin, the
 *server implementation of the Channel cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/channel-server/channel-delegate.h>
#include <app/clusters/channel-server/channel-server.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters::Channel;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::Channel::Delegate;

namespace {

Delegate * gDelegateTable[EMBER_AF_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::Channel::Id);
    return (ep == 0xFFFF ? NULL : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Channel has no delegate set for endpoint:%" PRIu16, endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace Channel {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, chip::app::Clusters::Channel::Id);
    if (ep != 0xFFFF)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace Channel
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class ChannelAttrAccess : public app::AttributeAccessInterface
{
public:
    ChannelAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), chip::app::Clusters::Channel::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadChannelListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadLineupAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadCurrentChannelAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

ChannelAttrAccess gChannelAttrAccess;

CHIP_ERROR ChannelAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (isDelegateNull(delegate, endpoint))
    {
        return CHIP_NO_ERROR;
    }

    switch (aPath.mAttributeId)
    {
    case app::Clusters::Channel::Attributes::ChannelList::Id: {
        return ReadChannelListAttribute(aEncoder, delegate);
    }
    case app::Clusters::Channel::Attributes::ChannelLineup::Id: {
        return ReadLineupAttribute(aEncoder, delegate);
    }
    case app::Clusters::Channel::Attributes::CurrentChannel::Id: {
        return ReadCurrentChannelAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChannelAttrAccess::ReadChannelListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetChannelList(aEncoder);
}

CHIP_ERROR ChannelAttrAccess::ReadLineupAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetLineup(aEncoder);
}

CHIP_ERROR ChannelAttrAccess::ReadCurrentChannelAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetCurrentChannel(aEncoder);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfChannelClusterChangeChannelRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                       const Commands::ChangeChannelRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    auto & match = commandData.match;

    app::CommandResponseHelper<Commands::ChangeChannelResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleChangeChannel(match, responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterChangeChannelRequestCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfChannelClusterChangeChannelByNumberRequestCallback(
    app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
    const Commands::ChangeChannelByNumberRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    auto & majorNumber = commandData.majorNumber;
    auto & minorNumber = commandData.minorNumber;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterChangeChannelByNumberRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    bool success         = delegate->HandleChangeChannelByNumber(majorNumber, minorNumber);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfChannelClusterSkipChannelRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::SkipChannelRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    auto & count        = commandData.count;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterSkipChannelRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    bool success         = delegate->HandleSkipChannel(count);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterChannelPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gChannelAttrAccess);
}
