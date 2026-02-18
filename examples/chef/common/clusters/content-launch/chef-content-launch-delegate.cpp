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
#include <app/util/config.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ContentLauncher;

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {
namespace Chef {

void ChefDelegate::HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                                       const chip::app::DataModel::DecodableList<ParameterType> & parameterList, bool autoplay,
                                       const CharSpan & data, const chip::Optional<PlaybackPreferencesType> playbackPreferences,
                                       bool useCurrentContext)
{
    ChipLogProgress(Zcl, "ChefDelegate::HandleLaunchContent");

    bool foundMatch = false;
    auto iter       = parameterList.begin();
    while (iter.Next())
    {
        auto & parameterType = iter.GetValue();
        for (auto const & launchableContent : mLaunchableContentData)
        {
            if (launchableContent.type == parameterType.type && launchableContent.value.data_equal(parameterType.value))
            {
                foundMatch = true;
                break;
            }
        }
        if (foundMatch)
        {
            break;
        }
    }

    if (foundMatch)
    {
        ChipLogProgress(Zcl, "ChefDelegate::HandleLaunchContent match found");
        LaunchResponseType response;
        response.status = ContentLauncher::StatusEnum::kSuccess;
        response.data   = chip::MakeOptional(data);
        helper.Success(response);
    }
    else
    {
        ChipLogProgress(Zcl, "ChefDelegate::HandleLaunchContent match NOT found");
        helper.Failure(chip::Protocols::InteractionModel::Status::NotFound);
    }
}

void ChefDelegate::HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                                   const CharSpan & displayString, const BrandingInformationType & brandingInformation)
{
    ChipLogProgress(Zcl, "ChefDelegate::HandleLaunchUrl");
    LaunchResponseType response;
    response.status = ContentLauncher::StatusEnum::kSuccess;
    response.data   = chip::MakeOptional(contentUrl);
    helper.Success(response);
}

CHIP_ERROR ChefDelegate::HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder)
{
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
    return mSupportedStreamingProtocols;
}

uint32_t ChefDelegate::GetFeatureMap(chip::EndpointId endpoint)
{
    if (endpoint != mEndpointId)
    {
        return kEndpointFeatureMap;
    }
    return mFeaturemap;
}

uint16_t ChefDelegate::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint != mEndpointId)
    {
        return kClusterRevision;
    }
    return mClusterRevision;
}

} // namespace Chef
} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
