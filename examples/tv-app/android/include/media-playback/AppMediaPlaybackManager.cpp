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

#include "AppMediaPlaybackManager.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/config.h>
#include <cstdint>
#include <jni.h>
#include <json/json.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::MediaPlayback;
using namespace chip::Uint8;
using chip::CharSpan;

AppMediaPlaybackManager::AppMediaPlaybackManager(ContentAppAttributeDelegate * attributeDelegate) :
    mAttributeDelegate(attributeDelegate){};

PlaybackStateEnum AppMediaPlaybackManager::HandleGetCurrentState()
{
    uint64_t ret = HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_PLAYBACK_STATE);
    return static_cast<PlaybackStateEnum>(ret);
}

uint64_t AppMediaPlaybackManager::HandleGetStartTime()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_START_TIME);
}

uint64_t AppMediaPlaybackManager::HandleGetDuration()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_DURATION);
}

float AppMediaPlaybackManager::HandleGetPlaybackSpeed()
{
    uint64_t ret = HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_SPEED);
    return static_cast<float>(ret) / 10000.0f;
}

uint64_t AppMediaPlaybackManager::HandleGetSeekRangeStart()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_START);
}

uint64_t AppMediaPlaybackManager::HandleGetSeekRangeEnd()
{
    return HandleMediaRequestGetAttribute(MEDIA_PLAYBACK_ATTRIBUTE_SEEK_RANGE_END);
}

CHIP_ERROR AppMediaPlaybackManager::HandleGetActiveAudioTrack(AttributeValueEncoder & aEncoder)
{
    TrackType mActiveAudioTrack;
    return aEncoder.Encode(mActiveAudioTrack);
}

CHIP_ERROR AppMediaPlaybackManager::HandleGetAvailableAudioTracks(AttributeValueEncoder & aEncoder)
{
    std::vector<TrackType> mAvailableAudioTracks;
    // TODO: Insert code here
    return aEncoder.EncodeList([mAvailableAudioTracks](const auto & encoder) -> CHIP_ERROR {
        for (auto const & audioTrack : mAvailableAudioTracks)
        {
            ReturnErrorOnFailure(encoder.Encode(audioTrack));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AppMediaPlaybackManager::HandleGetActiveTextTrack(AttributeValueEncoder & aEncoder)
{
    TrackType mActiveTextTrack;
    return aEncoder.Encode(mActiveTextTrack);
}

CHIP_ERROR AppMediaPlaybackManager::HandleGetAvailableTextTracks(AttributeValueEncoder & aEncoder)
{
    std::vector<TrackType> mAvailableTextTracks;
    // TODO: Insert code here
    return aEncoder.EncodeList([mAvailableTextTracks](const auto & encoder) -> CHIP_ERROR {
        for (auto const & textTrack : mAvailableTextTracks)
        {
            ReturnErrorOnFailure(encoder.Encode(textTrack));
        }
        return CHIP_NO_ERROR;
    });
}

void AppMediaPlaybackManager::HandlePlay(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PLAY, 0));
}

void AppMediaPlaybackManager::HandlePause(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PAUSE, 0));
}

void AppMediaPlaybackManager::HandleStop(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_STOP, 0));
}

void AppMediaPlaybackManager::HandleFastForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                                const chip::Optional<bool> & audioAdvanceUnmuted)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_FAST_FORWARD, 0));
}

void AppMediaPlaybackManager::HandlePrevious(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_PREVIOUS, 0));
}

void AppMediaPlaybackManager::HandleRewind(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                           const chip::Optional<bool> & audioAdvanceUnmuted)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_REWIND, 0));
}

void AppMediaPlaybackManager::HandleSkipBackward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                                 const uint64_t & deltaPositionMilliseconds)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SKIP_BACKWARD, deltaPositionMilliseconds));
}

void AppMediaPlaybackManager::HandleSkipForward(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                                const uint64_t & deltaPositionMilliseconds)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SKIP_FORWARD, deltaPositionMilliseconds));
}

void AppMediaPlaybackManager::HandleSeek(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper,
                                         const uint64_t & positionMilliseconds)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_SEEK, positionMilliseconds));
}

void AppMediaPlaybackManager::HandleNext(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_NEXT, 0));
}

void AppMediaPlaybackManager::HandleStartOver(CommandResponseHelper<Commands::PlaybackResponse::Type> & helper)
{
    helper.Success(HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_START_OVER, 0));
}

bool AppMediaPlaybackManager::HandleActivateAudioTrack(const chip::CharSpan & trackId, const uint8_t & audioOutputIndex)
{
    // Handle Activate Audio Track
    HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_ACTIVATE_AUDIO_TRACK, 0);
    return true;
}

bool AppMediaPlaybackManager::HandleActivateTextTrack(const chip::CharSpan & trackId)
{
    // Handle Activate Text Track
    HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_ACTIVATE_TEXT_TRACK, 0);
    return true;
}

bool AppMediaPlaybackManager::HandleDeactivateTextTrack()
{
    // Handle Deactivate Text Track
    HandleMediaRequest(MEDIA_PLAYBACK_REQUEST_DEACTIVATE_TEXT_TRACK, 0);
    return true;
}

uint64_t AppMediaPlaybackManager::HandleMediaRequestGetAttribute(chip::AttributeId attributeId)
{
    ChipLogProgress(Zcl, "Received AppMediaPlaybackManager::HandleMediaRequestGetAttribute:%d", attributeId);
    chip::app::ConcreteReadAttributePath aPath(mEndpointId, chip::app::Clusters::MediaPlayback::Id, attributeId);
    std::string resStr = mAttributeDelegate->Read(aPath);
    ChipLogProgress(Zcl, "AppMediaPlaybackManager::HandleMediaRequestGetAttribute response %s", resStr.c_str());

    uint64_t ret = std::numeric_limits<uint64_t>::max();
    if (resStr.length() != 0)
    {
        Json::Reader reader;
        Json::Value value;
        if (reader.parse(resStr, value))
        {
            std::string attrId = to_string(attributeId);
            ChipLogProgress(Zcl, "AppMediaPlaybackManager::HandleMediaRequestGetAttribute response parsing done. reading attr %s",
                            attrId.c_str());
            if (!value[attrId].empty() && value[attrId].isUInt())
            {
                ret = static_cast<uint64_t>(value[attrId].asUInt());
                return ret;
            }
            ChipLogError(Zcl,
                         "AppMediaPlaybackManager::HandleMediaRequestGetAttribute error. Invalid response from the content app.");
            return ret;
        }
    }
    ChipLogError(
        Zcl, "AppMediaPlaybackManager::HandleMediaRequestGetAttribute error. Did not get a response back from the content app.");
    return ret;
}

Commands::PlaybackResponse::Type AppMediaPlaybackManager::HandleMediaRequest(MediaPlaybackRequest mediaPlaybackRequest,
                                                                             uint64_t deltaPositionMilliseconds)
{
    // Ideally should not come here
    ChipLogProgress(Zcl, "AppMediaPlaybackManager::HandleMediaRequest");
    Commands::PlaybackResponse::Type response;
    response.status = StatusEnum::kInvalidStateForCommand;
    return response;
}

CHIP_ERROR AppMediaPlaybackManager::HandleGetSampledPosition(AttributeValueEncoder & aEncoder)
{
    Structs::PlaybackPositionStruct::Type response;
    response.updatedAt = 0;
    response.position  = Nullable<uint64_t>(0);

    ChipLogProgress(Zcl, "AppMediaPlaybackManager::HandleGetSampledPosition");
    chip::app::ConcreteReadAttributePath aPath(mEndpointId, chip::app::Clusters::MediaPlayback::Id,
                                               chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::Id);
    std::string resStr = mAttributeDelegate->Read(aPath);
    ChipLogProgress(Zcl, "AppMediaPlaybackManager::HandleGetSampledPosition response %s", resStr.c_str());

    if (resStr.length() != 0)
    {
        Json::Reader reader;
        Json::Value value;
        if (reader.parse(resStr, value))
        {
            std::string attrId = to_string(chip::app::Clusters::MediaPlayback::Attributes::SampledPosition::Id);
            ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetSampledPosition response parsing done. reading attr %s",
                            attrId.c_str());
            if (!value[attrId].empty() && value[attrId].isObject())
            {
                std::string updatedAt = to_string(
                    static_cast<uint32_t>(chip::app::Clusters::MediaPlayback::Structs::PlaybackPositionStruct::Fields::kUpdatedAt));
                std::string position = to_string(
                    static_cast<uint32_t>(chip::app::Clusters::MediaPlayback::Structs::PlaybackPositionStruct::Fields::kPosition));
                if (!value[attrId][updatedAt].empty() && !value[attrId][position].empty() && value[attrId][updatedAt].isUInt() &&
                    value[attrId][position].isUInt())
                {
                    // valid response
                    response.updatedAt = value[attrId][updatedAt].asUInt();
                    response.position  = Nullable<uint64_t>(value[attrId][position].asUInt());
                    return aEncoder.Encode(response);
                }
            }
        }
    }
    ChipLogError(Zcl, "AppMediaPlaybackManager::GetAttribute error. Invalid response from the content app.");
    return aEncoder.Encode(response);
}

uint32_t AppMediaPlaybackManager::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kEndpointFeatureMap;
    }

    uint32_t featureMap = 0;
    Attributes::FeatureMap::Get(endpoint, &featureMap);
    return featureMap;
}

uint16_t AppMediaPlaybackManager::GetClusterRevision(chip::EndpointId endpoint)
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
        ChipLogError(Zcl, "AppMediaPlaybackManager::GetClusterRevision error reading cluster revision");
    }
    return clusterRevision;
}
