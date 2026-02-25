/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/content-launch-server/content-launch-delegate.h>
#include <app/util/config.h>

#if MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT > 0

typedef chip::Span<const chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::DecodableType> ContentEntry;

namespace chip::app::Clusters::ContentLauncher::Chef {

// Default application constants.
static constexpr uint32_t kEndpointFeatureMap = to_underlying(Feature::kContentSearch) | to_underlying(Feature::kURLPlayback);
static constexpr uint16_t kClusterRevision    = 1;
static constexpr uint32_t kSupportedStreamingProtocols = 0;
static constexpr chip::CharSpan kAcceptHeaderList[]    = { "application/dash+xml"_span,
                                                           "application/vnd.apple.mpegurl"_span,
                                                           "application/x-mpegurl"_span,
                                                           "video/mp4"_span,
                                                           "video/webm"_span,
                                                           "audio/mp4"_span,
                                                           "audio/mpeg"_span };

// LaunchContent will succeed only when requested content matches with one of the below items.
static const ContentLauncher::Structs::ParameterStruct::DecodableType kInterstellar[] = {
    {
        .type  = ParameterEnum::kType,
        .value = "Movie"_span,
    },
    {
        // Video represents the title of the content
        .type  = ParameterEnum::kVideo,
        .value = "Interstellar"_span,
    },
    {
        .type  = ParameterEnum::kDirector,
        .value = "Christopher Nolan"_span,
    },
    {
        .type  = ParameterEnum::kActor,
        .value = "Matthew McConaughey"_span,
    },
    {
        .type  = ParameterEnum::kActor,
        .value = "Anne Hathaway"_span,
    },
    {
        .type  = ParameterEnum::kGenre,
        .value = "Sci-Fi"_span,
    },
};

static const ContentLauncher::Structs::ParameterStruct::DecodableType kManUnitedMatch[] = {
    {
        .type  = ParameterEnum::kType,
        .value = "SportsEvent"_span,
    },
    {
        .type  = ParameterEnum::kLeague,
        .value = "Premier League"_span,
    },
    {
        .type  = ParameterEnum::kSportsTeam,
        .value = "Manchester United"_span,
    },
    {
        .type  = ParameterEnum::kSport,
        .value = "Football"_span,
    },
};

static const ContentEntry kLaunchableContent[] = {

    ContentEntry(kInterstellar),

    ContentEntry(kManUnitedMatch),
};

class ChefDelegate : public Delegate
{
public:
    ChefDelegate(const chip::EndpointId endpointId, const uint32_t featureMap, const uint16_t clusterRevision,
                 const uint32_t supportedStreamingProtocols, const chip::Span<const chip::CharSpan> acceptHeaderList,
                 const chip::Span<const ContentEntry> launchableContent) :
        mEndpointId(endpointId),
        mFeaturemap(featureMap), mClusterRevision(clusterRevision), mSupportedStreamingProtocols(supportedStreamingProtocols),
        mAcceptHeaderList(acceptHeaderList), mLaunchableContent(launchableContent){};
    ChefDelegate(chip::EndpointId endpointId) :
        ChefDelegate(endpointId, kEndpointFeatureMap, kClusterRevision, kSupportedStreamingProtocols,
                     chip::Span<const chip::CharSpan>(kAcceptHeaderList), chip::Span<const ContentEntry>(kLaunchableContent)){};
    ~ChefDelegate() = default;

    void HandleLaunchContent(
        chip::app::CommandResponseHelper<ContentLauncher::Commands::LauncherResponse::Type> & helper,
        const chip::app::DataModel::DecodableList<ContentLauncher::Structs::ParameterStruct::DecodableType> & parameterList,
        bool autoplay, const chip::CharSpan & data,
        const chip::Optional<ContentLauncher::Structs::PlaybackPreferencesStruct::DecodableType> playbackPreferences,
        bool useCurrentContext) override;
    void HandleLaunchUrl(chip::app::CommandResponseHelper<ContentLauncher::Commands::LauncherResponse::Type> & helper,
                         const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                         const ContentLauncher::Structs::BrandingInformationStruct::Type & brandingInformation) override;
    CHIP_ERROR HandleGetAcceptHeaderList(chip::app::AttributeValueEncoder & aEncoder) override;
    uint32_t HandleGetSupportedStreamingProtocols() override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

    chip::EndpointId GetEndpointId() { return mEndpointId; }

    void Register();

private:
    chip::EndpointId mEndpointId;
    uint32_t mFeaturemap;
    uint16_t mClusterRevision;
    uint32_t mSupportedStreamingProtocols;
    chip::Span<const chip::CharSpan> mAcceptHeaderList;
    chip::Span<const ContentEntry> mLaunchableContent;
};

} // namespace chip::app::Clusters::ContentLauncher::Chef

#endif // MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT
