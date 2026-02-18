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

using chip::CharSpan;
using chip::EndpointId;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ContentLauncherDelegate = chip::app::Clusters::ContentLauncher::Delegate;
using LaunchResponseType      = chip::app::Clusters::ContentLauncher::Commands::LauncherResponse::Type;
using ParameterType           = chip::app::Clusters::ContentLauncher::Structs::ParameterStruct::DecodableType;
using BrandingInformationType = chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type;
using PlaybackPreferencesType = chip::app::Clusters::ContentLauncher::Structs::PlaybackPreferencesStruct::DecodableType;

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
static const ParameterType kLaunchableContentData[]    = {
    // LaunchContent will return success if the requested content matches any of the below parameters.
    {
        .type  = ParameterEnum::kActor,
        .value = "Gaby Hoffman"_span,
    },
    {
        .type  = ParameterEnum::kChannel,
        .value = "PBS"_span,
    },
    {
        .type  = ParameterEnum::kCharacter,
        .value = "Snow White"_span,
    },
    {
        .type  = ParameterEnum::kDirector,
        .value = "Spike Lee"_span,
    },
    {
        .type  = ParameterEnum::kFranchise,
        .value = "Star Wars"_span,
    },
    {
        .type  = ParameterEnum::kGenre,
        .value = "Horror"_span,
    },
    {
        .type  = ParameterEnum::kPopularity,
        .value = "Popularity"_span,
    },
    {
        .type  = ParameterEnum::kProvider,
        .value = "Netflix"_span,
    },
    {
        .type  = ParameterEnum::kEvent,
        .value = "Football games"_span,
    },
    {
        .type  = ParameterEnum::kLeague,
        .value = "NCAA"_span,
    },
    {
        .type  = ParameterEnum::kSport,
        .value = "football"_span,
    },
    {
        .type  = ParameterEnum::kSportsTeam,
        .value = "Arsenel"_span,
    },
    {
        .type  = ParameterEnum::kType,
        .value = "TVSeries"_span,
    },
};

class ChefDelegate : public Delegate
{
public:
    ChefDelegate(const EndpointId endpointId, const uint32_t featureMap, const uint16_t clusterRevision,
                 const uint32_t supportedStreamingProtocols, const Span<const CharSpan> acceptHeaderList,
                 const Span<const ParameterType> launchableContentData) :
        mEndpointId(endpointId), mFeaturemap(featureMap), mClusterRevision(clusterRevision),
        mSupportedStreamingProtocols(supportedStreamingProtocols), mAcceptHeaderList(acceptHeaderList),
        mLaunchableContentData(launchableContentData) {};
    ChefDelegate(EndpointId endpointId) :
        ChefDelegate(endpointId, kEndpointFeatureMap, kClusterRevision, kSupportedStreamingProtocols,
                     Span<const CharSpan>(kAcceptHeaderList), Span<const ParameterType>(kLaunchableContentData)) {};

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

private:
    EndpointId mEndpointId;
    uint32_t mFeaturemap;
    uint16_t mClusterRevision;
    uint32_t mSupportedStreamingProtocols;
    Span<const CharSpan> mAcceptHeaderList;
    Span<const ParameterType> mLaunchableContentData;
};

} // namespace Chef
} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
