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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/channel-server/channel-delegate.h>
#include <app/clusters/channel-server/channel-server.h>
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Channel;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kChannelDelegateTableSize =
    MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kChannelDelegateTableSize <= kEmberInvalidEndpointIndex, "Channel Delegate table size error");

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

    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Channel::Id, MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kChannelDelegateTableSize ? nullptr : gDelegateTable[ep]);
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
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Channel::Id, MATTER_DM_CHANNEL_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kChannelDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

bool Delegate::HasFeature(chip::EndpointId endpoint, Feature feature)
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
    CHIP_ERROR ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

ChannelAttrAccess gChannelAttrAccess;

CHIP_ERROR ChannelAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    switch (aPath.mAttributeId)
    {
    case app::Clusters::Channel::Attributes::ChannelList::Id: {
        if (isDelegateNull(delegate, endpoint) || !delegate->HasFeature(endpoint, Feature::kChannelList))
        {
            return aEncoder.EncodeEmptyList();
        }

        return ReadChannelListAttribute(aEncoder, delegate);
    }
    case app::Clusters::Channel::Attributes::Lineup::Id: {
        if (isDelegateNull(delegate, endpoint) || !delegate->HasFeature(endpoint, Feature::kLineupInfo))
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
    case app::Clusters::Channel::Attributes::ClusterRevision::Id:
        return ReadRevisionAttribute(endpoint, aEncoder, delegate);
    default:
        break;
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

CHIP_ERROR ChannelAttrAccess::ReadRevisionAttribute(EndpointId endpoint, app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint16_t clusterRevision = delegate->GetClusterRevision(endpoint);
    return aEncoder.Encode(clusterRevision);
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
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfChannelClusterChangeChannelByNumberCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ChangeChannelByNumber::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;
    auto & majorNumber  = commandData.majorNumber;
    auto & minorNumber  = commandData.minorNumber;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleChangeChannelByNumber(majorNumber, minorNumber))
    {
        status = Status::Failure;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterChangeChannelByNumberCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

bool emberAfChannelClusterSkipChannelCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                              const Commands::SkipChannel::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    Status status       = Status::Success;
    auto & count        = commandData.count;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleSkipChannel(count))
    {
        status = Status::Failure;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterSkipChannelCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Channel Cluster GetProgramGuide Command callback (from client)
 */
bool emberAfChannelClusterGetProgramGuideCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Channel::Commands::GetProgramGuide::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;

    auto & startTime      = commandData.startTime;
    auto & endTime        = commandData.endTime;
    auto & channelList    = commandData.channelList;
    auto & pageToken      = commandData.pageToken;
    auto & recordingFlag  = commandData.recordingFlag;
    auto & externalIDList = commandData.externalIDList;
    auto & data           = commandData.data;

    app::CommandResponseHelper<Commands::ProgramGuideResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);
    {
        delegate->HandleGetProgramGuide(responder, startTime, endTime, channelList, pageToken, recordingFlag, externalIDList, data);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterGetProgramGuideCallback error: %s", err.AsString());
    }

    // If isDelegateNull, no one will call responder, so HasSentResponse will be false
    if (!responder.HasSentResponse())
    {
        command->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

/**
 * @brief Channel Cluster RecordProgram Command callback (from client)
 */
bool emberAfChannelClusterRecordProgramCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Channel::Commands::RecordProgram::DecodableType & commandData)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    EndpointId endpoint       = commandPath.mEndpointId;
    Status status             = Status::Success;
    auto & programIdentifier  = commandData.programIdentifier;
    auto & shouldRecordSeries = commandData.shouldRecordSeries;
    auto & externalIDList     = commandData.externalIDList;
    auto & data               = commandData.data;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleRecordProgram(programIdentifier, shouldRecordSeries, externalIDList, data))
    {
        status = Status::Failure;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterRecordProgramCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

/**
 * @brief Channel Cluster CancelRecordProgram Command callback (from client)
 */
bool emberAfChannelClusterCancelRecordProgramCallback(
    chip::app::CommandHandler * command, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Channel::Commands::CancelRecordProgram::DecodableType & commandData)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    EndpointId endpoint       = commandPath.mEndpointId;
    Status status             = Status::Success;
    auto & programIdentifier  = commandData.programIdentifier;
    auto & shouldRecordSeries = commandData.shouldRecordSeries;
    auto & externalIDList     = commandData.externalIDList;
    auto & data               = commandData.data;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    if (!delegate->HandleCancelRecordProgram(programIdentifier, shouldRecordSeries, externalIDList, data))
    {
        status = Status::Failure;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfChannelClusterCancelRecordProgramCallback error: %s", err.AsString());
        status = Status::Failure;
    }

    command->AddStatus(commandPath, status);
    return true;
}

void MatterChannelPluginServerInitCallback()
{
    app::AttributeAccessInterfaceRegistry::Instance().Register(&gChannelAttrAccess);
}
