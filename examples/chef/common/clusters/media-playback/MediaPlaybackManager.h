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

#include <app/clusters/media-playback-server/media-playback-server.h>
#include <vector>

class MediaPlaybackManager : public chip::app::Clusters::MediaPlayback::Delegate
{
    using PlaybackResponseType = chip::app::Clusters::MediaPlayback::Commands::PlaybackResponse::Type;
    using PlaybackPositionType = chip::app::Clusters::MediaPlayback::Structs::PlaybackPositionStruct::Type;
    using TrackType            = chip::app::Clusters::MediaPlayback::Structs::TrackStruct::Type;
    using TrackAttributesType  = chip::app::Clusters::MediaPlayback::Structs::TrackAttributesStruct::Type;
    using Feature              = chip::app::Clusters::MediaPlayback::Feature;

public:
    chip::app::Clusters::MediaPlayback::PlaybackStateEnum HandleGetCurrentState() override;
    uint64_t HandleGetStartTime() override;
    uint64_t HandleGetDuration() override;
    CHIP_ERROR HandleGetSampledPosition(chip::app::AttributeValueEncoder & aEncoder) override;
    float HandleGetPlaybackSpeed() override;
    uint64_t HandleGetSeekRangeStart() override;
    uint64_t HandleGetSeekRangeEnd() override;
    CHIP_ERROR HandleGetActiveAudioTrack(chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetAvailableAudioTracks(chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetActiveTextTrack(chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR HandleGetAvailableTextTracks(chip::app::AttributeValueEncoder & aEncoder) override;

    void HandlePlay(chip::app::CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandlePause(chip::app::CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleStop(chip::app::CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleFastForward(chip::app::CommandResponseHelper<PlaybackResponseType> & helper,
                           const chip::Optional<bool> & audioAdvanceUnmuted) override;
    void HandlePrevious(chip::app::CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleRewind(chip::app::CommandResponseHelper<PlaybackResponseType> & helper,
                      const chip::Optional<bool> & audioAdvanceUnmuted) override;
    void HandleSkipBackward(chip::app::CommandResponseHelper<PlaybackResponseType> & helper,
                            const uint64_t & deltaPositionMilliseconds) override;
    void HandleSkipForward(chip::app::CommandResponseHelper<PlaybackResponseType> & helper,
                           const uint64_t & deltaPositionMilliseconds) override;
    void HandleSeek(chip::app::CommandResponseHelper<PlaybackResponseType> & helper,
                    const uint64_t & positionMilliseconds) override;
    void HandleNext(chip::app::CommandResponseHelper<PlaybackResponseType> & helper) override;
    void HandleStartOver(chip::app::CommandResponseHelper<PlaybackResponseType> & helper) override;
    bool HandleActivateAudioTrack(const chip::CharSpan & trackId, const uint8_t & audioOutputIndex) override;
    bool HandleActivateTextTrack(const chip::CharSpan & trackId) override;
    bool HandleDeactivateTextTrack() override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

protected:
    // NOTE: it does not make sense to have default state of playing with a speed of 0, but
    // the CI test cases expect these values, and need to be fixed.
    chip::app::Clusters::MediaPlayback::PlaybackStateEnum mCurrentState =
        chip::app::Clusters::MediaPlayback::PlaybackStateEnum::kPlaying;
    PlaybackPositionType mPlaybackPosition       = { 0, chip::app::DataModel::Nullable<uint64_t>(0) };
    TrackType mActiveAudioTrack                  = { chip::CharSpan("activeAudioTrackId_0", 20),
                                                     chip::app::DataModel::Nullable<TrackAttributesType>(
                                        { chip::CharSpan("languageCode1", 13),
                                                           chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                                              { chip::app::DataModel::MakeNullable(chip::CharSpan("displayName1", 12)) }) }) };
    std::vector<TrackType> mAvailableAudioTracks = {
        { chip::CharSpan("activeAudioTrackId_0", 20),
          chip::app::DataModel::Nullable<TrackAttributesType>(
              { chip::CharSpan("languageCode1", 13),
                chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                    { chip::app::DataModel::MakeNullable(chip::CharSpan("displayName1", 12)) }) }) },
        { chip::CharSpan("activeAudioTrackId_1", 20),
          chip::app::DataModel::Nullable<TrackAttributesType>(
              { chip::CharSpan("languageCode2", 13),
                chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                    { chip::app::DataModel::MakeNullable(chip::CharSpan("displayName2", 12)) }) }) }
    };
    TrackType mActiveTextTrack                  = {};
    std::vector<TrackType> mAvailableTextTracks = {
        { chip::CharSpan("activeTextTrackId_0", 19),
          chip::app::DataModel::Nullable<TrackAttributesType>(
              { chip::CharSpan("languageCode1", 13),
                chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                    { chip::app::DataModel::MakeNullable(chip::CharSpan("displayName1", 12)) }) }) },
        { chip::CharSpan("activeTextTrackId_1", 19),
          chip::app::DataModel::Nullable<TrackAttributesType>(
              { chip::CharSpan("languageCode2", 13),
                chip::Optional<chip::app::DataModel::Nullable<chip::CharSpan>>(
                    { chip::app::DataModel::MakeNullable(chip::CharSpan("displayName2", 12)) }) }) }
    };
    float mPlaybackSpeed = 1.0;
    uint64_t mStartTime  = 0;
    // Magic number for testing.
    uint64_t mDuration      = 80000;
    bool mAudioAdvanceMuted = false;

    static const int kPlaybackMaxForwardSpeed = 10;
    static const int kPlaybackMaxRewindSpeed  = -10;

private:
    static constexpr uint32_t mDynamicEndpointFeatureMap =
        chip::BitMask<Feature, uint32_t>(Feature::kAdvancedSeek, Feature::kVariableSpeed).Raw();
    static constexpr uint16_t kClusterRevision = 2;
};
