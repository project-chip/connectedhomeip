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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/clusters/channel-server/channel-delegate.h>
#include <app/clusters/channel-server/channel-server.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceConfig.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Channel;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

static constexpr size_t kChannelDelegateTableSize =
    EMBER_AF_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::Channel::Delegate;

namespace {

Delegate * gDelegateTable[kChannelDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogProgress(Zcl, "Channel returning ContentApp delegate for endpoint:%u", endpoint);
        return app->GetChannelDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogProgress(Zcl, "Channel NOT returning ContentApp delegate for endpoint:%u", endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, Channel::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Channel has no delegate set for endpoint:%u", endpoint);
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
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, Channel::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

bool Delegate::HasFeature(chip::EndpointId endpoint, ChannelFeature feature)
{
    uint32_t featureMap = GetFeatureMap(endpoint);
    return (featureMap & chip::to_underlying(feature));
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
    ChannelAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), Channel::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadChannelListAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadLineupAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadCurrentChannelAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

ChannelAttrAccess gChannelAttrAccess;

CHIP_ERROR ChannelAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::Channel::Attributes::ChannelList::Id: {
        if (isDelegateNull(delegate, endpoint) || !delegate->HasFeature(endpoint, ChannelFeature::kChannelList))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadChannelListAttribute(aEncoder, delegate);
    }
    case app::Clusters::Channel::Attributes::Lineup::Id: {
        if (isDelegateNull(delegate, endpoint) || !delegate->HasFeature(endpoint, ChannelFeature::kLineupInfo))
        {
            return CHIP_NO_ERROR;
        }

        return ReadLineupAttribute(aEncoder, delegate);
    }
    case app::Clusters::Channel::Attributes::CurrentChannel::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadCurrentChannelAttribute(aEncoder, delegate);
    }
    case app::Clusters::Channel::Attributes::FeatureMap::Id: {
        if (isDelegateNull(delegate, endpoint))
        {
            return CHIP_NO_ERROR;
        }

        return ReadFeatureFlagAttribute(endpoint, aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChannelAttrAccess::ReadFeatureFlagAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder,
                                                       Delegate * delegate)
{
    uint32_t featureFlag = delegate->GetFeatureMap(endpoint);
    return aEncoder.Encode(featureFlag);
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

bool emberAfChannelClusterChangeChannelCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                const Commands::ChangeChannel::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    auto & match = commandData.match;

    app::CommandResponseHelper<Commands::ChangeChannelResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleChangeChannel(responder, match);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterChangeChannelCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfChannelClusterChangeChannelByNumberCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ChangeChannelByNumber::DecodableType & commandData)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    EndpointId endpoint  = commandPath.mEndpointId;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    auto & majorNumber   = commandData.majorNumber;
    auto & minorNumber   = commandData.minorNumber;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleChangeChannelByNumber(majorNumber, minorNumber))
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterChangeChannelByNumberCallback error: %s", err.AsString());
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfChannelClusterSkipChannelCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                              const Commands::SkipChannel::DecodableType & commandData)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    EndpointId endpoint  = commandPath.mEndpointId;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    auto & count         = commandData.count;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleSkipChannel(count))
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterSkipChannelCallback error: %s", err.AsString());
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void MatterChannelPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gChannelAttrAccess);
}
