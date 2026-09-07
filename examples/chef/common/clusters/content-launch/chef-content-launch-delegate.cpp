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

#include "chef-content-launch-delegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/content-launch-server/content-launch-server.h>
#include <app/util/config.h>
#include <lib/support/CHIPMem.h> // For chip::Platform
#include <lib/support/logging/CHIPLogging.h>

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

typedef chip::app::DataModel::DecodableList<ParameterType> SearchParameters;

namespace {

/**
 * Checks if given content has **all** search parameters.
 */
bool checkContentMatch(const ContentEntry & content, const SearchParameters & params)
{
    ChipLogProgress(Zcl, "In checkContentMatch");
    ChipLogProgress(Zcl, "    Available params:");
    for (auto it = content.begin(); it != content.end(); ++it)
    {
        ChipLogProgress(Zcl, "        type=%d value=%s", chip::to_underlying(it->type), chip::NullTerminated(it->value).c_str());
    }
    auto required_it = params.begin();
    ChipLogProgress(Zcl, "    Looking for params:");
    while (required_it.Next())
    {
        auto & requiredParam = required_it.GetValue();
        ChipLogProgress(Zcl, "        Looking for param: type=%d value=%s", chip::to_underlying(requiredParam.type),
                        chip::NullTerminated(requiredParam.value).c_str());
        bool found = std::any_of(content.begin(), content.end(), [requiredParam](const ParameterType & availableParam) {
            return requiredParam.type == availableParam.type && requiredParam.value.data_equal(availableParam.value);
        });
        if (!found)
        {
            ChipLogProgress(Zcl, "        Param not found");
            return false;
        }
        ChipLogProgress(Zcl, "        Param found");
    }
    ChipLogProgress(Zcl, "        All params found");
    return true;
}

} // namespace

namespace chip::app::Clusters::ContentLauncher::Chef {

void ChefDelegate::HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                                       const chip::app::DataModel::DecodableList<ParameterType> & parameterList, bool autoplay,
                                       const CharSpan & data, const chip::Optional<PlaybackPreferencesType> playbackPreferences,
                                       bool useCurrentContext)
{
    ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::HandleLaunchContent");

    bool foundMatch =
        std::any_of(mLaunchableContent.begin(), mLaunchableContent.end(),
                    [parameterList](const ContentEntry & content) { return checkContentMatch(content, parameterList); });

    LaunchResponseType response;

    if (foundMatch)
    {
        ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::HandleLaunchContent match found");
        response.status = ContentLauncher::StatusEnum::kSuccess;
    }
    else
    {
        ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::HandleLaunchContent match NOT found");
        response.status = ContentLauncher::StatusEnum::kURLNotAvailable; // Only available enum closest to a "not found"
    }

    response.data = chip::MakeOptional(data);
    LogErrorOnFailure(helper.Success(response));
}

void ChefDelegate::HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                                   const CharSpan & displayString, const BrandingInformationType & brandingInformation)
{
    ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::HandleLaunchUrl");
    LaunchResponseType response;
    response.status = ContentLauncher::StatusEnum::kSuccess;
    response.data   = chip::MakeOptional(contentUrl);
    LogErrorOnFailure(helper.Success(response));
}

CHIP_ERROR ChefDelegate::HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::HandleGetAcceptHeaderList");
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & header : mAcceptHeaderList)
        {
            ReturnErrorOnFailure(encoder.Encode(header));
        }
        return CHIP_NO_ERROR;
    });
}

uint32_t ChefDelegate::HandleGetSupportedStreamingProtocols()
{
    ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::HandleGetSupportedStreamingProtocols");
    return mSupportedStreamingProtocols;
}

uint32_t ChefDelegate::GetFeatureMap(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::GetFeatureMap");
    if (endpoint != mEndpointId)
    {
        ChipLogError(
            Zcl,
            "ContentLauncher::ChefDelegate::GetFeatureMap I am delegate for endpoint %d but got GetFeatureMap with endpoint %d",
            mEndpointId, endpoint);
    }
    return mFeaturemap;
}

uint16_t ChefDelegate::GetClusterRevision(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "ContentLauncher::ChefDelegate::GetClusterRevision");
    if (endpoint != mEndpointId)
    {
        ChipLogError(Zcl,
                     "ContentLauncher::ChefDelegate::GetClusterRevision I am delegate for endpoint %d but got GetClusterRevision "
                     "with endpoint %d",
                     mEndpointId, endpoint);
    }
    return mClusterRevision;
}

void ChefDelegate::Register()
{
    SetDefaultDelegate(mEndpointId, this);
}

} // namespace chip::app::Clusters::ContentLauncher::Chef

#endif // MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT
