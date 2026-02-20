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

using chip::CharSpan;
using chip::EndpointId;
using chip::Span;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ContentLauncherDelegate = chip::app::Clusters::ContentLauncher::Delegate;
using LaunchResponseType      = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::Type;
using ParameterType           = chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::DecodableType;
using BrandingInformationType = chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type;
using PlaybackPreferencesType = chip::app::Clusters::ContentLauncher::Structs::PlaybackPreferencesStruct::DecodableType;

typedef Span<const ParameterType> ContentEntry;

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

namespace Chef {

// Default application constants.
static constexpr uint32_t kEndpointFeatureMap = to_underlying(Feature::kContentSearch) | to_underlying(Feature::kURLPlayback);
static constexpr uint16_t kClusterRevision    = 1;
static constexpr uint32_t kSupportedStreamingProtocols = 0;
static constexpr CharSpan kAcceptHeaderList[]          = { "application/dash+xml"_span,
                                                           "application/vnd.apple.mpegurl"_span,
                                                           "application/x-mpegurl"_span,
                                                           "video/mp4"_span,
                                                           "video/webm"_span,
                                                           "audio/mp4"_span,
                                                           "audio/mpeg"_span };

// LaunchContent will succeed only when requested content matches with one of the below items.
static const ParameterType kInterstellar[] = {
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

static const ParameterType kManUnitedMatch[] = {
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
    ChefDelegate(const EndpointId endpointId, const uint32_t featureMap, const uint16_t clusterRevision,
                 const uint32_t supportedStreamingProtocols, const Span<const CharSpan> acceptHeaderList,
                 const Span<const ContentEntry> launchableContent) :
        mEndpointId(endpointId), mFeaturemap(featureMap), mClusterRevision(clusterRevision),
        mSupportedStreamingProtocols(supportedStreamingProtocols), mAcceptHeaderList(acceptHeaderList),
        mLaunchableContent(launchableContent) {};
    ChefDelegate(EndpointId endpointId) :
        ChefDelegate(endpointId, kEndpointFeatureMap, kClusterRevision, kSupportedStreamingProtocols,
                     Span<const CharSpan>(kAcceptHeaderList), Span<const ContentEntry>(kLaunchableContent)) {};

    void HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                             const chip::app::DataModel::DecodableList<ParameterType> & parameterList, bool autoplay,
                             const CharSpan & data, const chip::Optional<PlaybackPreferencesType> playbackPreferences,
                             bool useCurrentContext) override;
    void HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                         const CharSpan & displayString, const BrandingInformationType & brandingInformation) override;
    CHIP_ERROR HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder) override;
    uint32_t HandleGetSupportedStreamingProtocols() override;

    uint32_t GetFeatureMap(chip::EndpointId endpoint) override;
    uint16_t GetClusterRevision(chip::EndpointId endpoint) override;

    EndpointId GetEndpointId() { return mEndpointId; }

private:
    EndpointId mEndpointId;
    uint32_t mFeaturemap;
    uint16_t mClusterRevision;
    uint32_t mSupportedStreamingProtocols;
    Span<const CharSpan> mAcceptHeaderList;
    Span<const ContentEntry> mLaunchableContent;
};

void AddDefaultDelegateForEndpioint(EndpointId endpoint);

} // namespace Chef
} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT
