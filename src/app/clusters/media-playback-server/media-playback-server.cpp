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

/****************************************************************************
 * @file
 * @brief Routines for the Media Playback plugin, the
 *server implementation of the Media Playback cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/media-playback-server/media-playback-delegate.h>
#include <app/clusters/media-playback-server/media-playback-server.h>

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/app-platform/ContentAppPlatform.h>
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#include <app/data-model/Encode.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MediaPlayback;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
using namespace chip::AppPlatform;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::MediaPlayback::Delegate;

namespace {

Delegate * gDelegateTable[EMBER_AF_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = ContentAppPlatform::GetInstance().GetContentApp(endpoint);
    if (app != nullptr)
    {
        ChipLogError(Zcl, "MediaPlayback returning ContentApp delegate for endpoint:%" PRIu16, endpoint);
        return app->GetMediaPlaybackDelegate();
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ChipLogError(Zcl, "MediaPlayback NOT returning ContentApp delegate for endpoint:%" PRIu16, endpoint);

    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, MediaPlayback::Id);
    return ((ep == 0xFFFF || ep >= EMBER_AF_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT) ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Media Playback has no delegate set for endpoint:%" PRIu16, endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfFindClusterServerEndpointIndex(endpoint, MediaPlayback::Id);
    // if endpoint is found and is not a dynamic endpoint
    if (ep != 0xFFFF && ep < EMBER_AF_MEDIA_PLAYBACK_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip

// -----------------------------------------------------------------------------
// Attribute Accessor Implementation

namespace {

class MediaPlaybackAttrAccess : public app::AttributeAccessInterface
{
public:
    MediaPlaybackAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), MediaPlayback::Id) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadCurrentStateAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadStartTimeAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadDurationAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSampledPositionAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadPlaybackSpeedAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSeekRangeStartAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSeekRangeEndAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate);
};

MediaPlaybackAttrAccess gMediaPlaybackAttrAccess;

CHIP_ERROR MediaPlaybackAttrAccess::Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (isDelegateNull(delegate, endpoint))
    {
        return CHIP_NO_ERROR;
    }

    switch (aPath.mAttributeId)
    {
    case app::Clusters::MediaPlayback::Attributes::PlaybackState::Id: {
        return ReadCurrentStateAttribute(aEncoder, delegate);
    }
    case app::Clusters::MediaPlayback::Attributes::StartTime::Id: {
        return ReadStartTimeAttribute(aEncoder, delegate);
    }
    case app::Clusters::MediaPlayback::Attributes::Duration::Id: {
        return ReadDurationAttribute(aEncoder, delegate);
    }
    case app::Clusters::MediaPlayback::Attributes::Position::Id: {
        return ReadSampledPositionAttribute(aEncoder, delegate);
    }
    case app::Clusters::MediaPlayback::Attributes::PlaybackSpeed::Id: {
        return ReadPlaybackSpeedAttribute(aEncoder, delegate);
    }
    case app::Clusters::MediaPlayback::Attributes::SeekRangeStart::Id: {
        return ReadSeekRangeStartAttribute(aEncoder, delegate);
    }
    case app::Clusters::MediaPlayback::Attributes::SeekRangeEnd::Id: {
        return ReadSeekRangeEndAttribute(aEncoder, delegate);
    }
    default: {
        break;
    }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadCurrentStateAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    MediaPlayback::PlaybackStateEnum currentState = delegate->HandleGetCurrentState();
    return aEncoder.Encode(currentState);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadStartTimeAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t startTime = delegate->HandleGetStartTime();
    return aEncoder.Encode(startTime);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadDurationAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t duration = delegate->HandleGetDuration();
    return aEncoder.Encode(duration);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadSampledPositionAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetSampledPosition(aEncoder);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadPlaybackSpeedAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    float playbackSpeed = delegate->HandleGetPlaybackSpeed();
    return aEncoder.Encode(playbackSpeed);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadSeekRangeStartAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t seekRangeStart = delegate->HandleGetSeekRangeStart();
    return aEncoder.Encode(seekRangeStart);
}

CHIP_ERROR MediaPlaybackAttrAccess::ReadSeekRangeEndAttribute(app::AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    uint64_t seekRangeEnd = delegate->HandleGetSeekRangeEnd();
    return aEncoder.Encode(seekRangeEnd);
}

} // anonymous namespace

// -----------------------------------------------------------------------------
// Matter Framework Callbacks Implementation

bool emberAfMediaPlaybackClusterPlayRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::PlayRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandlePlay(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterPlayRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterPauseRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::PauseRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandlePause(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterPauseRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterStopRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::StopRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleStop(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterStopRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterFastForwardRequestCallback(app::CommandHandler * command,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::FastForwardRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleFastForward(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterFastForwardRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterPreviousRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                        const Commands::PreviousRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandlePrevious(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterPreviousRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterRewindRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                      const Commands::RewindRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleRewind(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterRewindRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterSkipBackwardRequestCallback(app::CommandHandler * command,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::SkipBackwardRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleSkipBackward(responder, deltaPositionMilliseconds);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterSkipBackwardRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterSkipForwardRequestCallback(app::CommandHandler * command,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::SkipForwardRequest::DecodableType & commandData)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    EndpointId endpoint              = commandPath.mEndpointId;
    auto & deltaPositionMilliseconds = commandData.deltaPositionMilliseconds;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleSkipForward(responder, deltaPositionMilliseconds);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterSkipForwardRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterSeekRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::SeekRequest::DecodableType & commandData)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    EndpointId endpoint         = commandPath.mEndpointId;
    auto & positionMilliseconds = commandData.position;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleSeekRequest(responder, positionMilliseconds);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterSeekRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterNextRequestCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::NextRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleNext(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterNextRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfMediaPlaybackClusterStartOverRequestCallback(app::CommandHandler * command,
                                                         const app::ConcreteCommandPath & commandPath,
                                                         const Commands::StartOverRequest::DecodableType & commandData)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    EndpointId endpoint = commandPath.mEndpointId;
    app::CommandResponseHelper<Commands::PlaybackResponse::Type> responder(command, commandPath);

    Delegate * delegate = GetDelegate(endpoint);
    VerifyOrExit(isDelegateNull(delegate, endpoint) != true, err = CHIP_ERROR_INCORRECT_STATE);

    {
        delegate->HandleStartOverRequest(responder);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "emberAfMediaPlaybackClusterStartOverRequestCallback error: %s", err.AsString());
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

void MatterMediaPlaybackPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gMediaPlaybackAttrAccess);
}
