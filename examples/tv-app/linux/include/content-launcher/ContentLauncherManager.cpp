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

#include "ContentLauncherManager.h"

using namespace std;
using namespace chip::app::Clusters::ContentLauncher;

ContentLauncherManager::ContentLauncherManager(std::list<std::string> acceptHeaderList, uint32_t supportedStreamingProtocols)
{
    mAcceptHeaderList            = acceptHeaderList;
    mSupportedStreamingProtocols = supportedStreamingProtocols;
}

void ContentLauncherManager::HandleLaunchContent(
    const std::list<Parameter> & parameterList, bool autoplay, const chip::CharSpan & data,
    chip::app::CommandResponseHelper<chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type> & responser)
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleLaunchContent for endpoint %d", mEndpointId);
    string dataString(data.data(), data.size());

    Commands::LaunchResponse::Type response;
    // TODO: Insert code here
    response.data   = chip::CharSpan("exampleData", strlen("exampleData"));
    response.status = chip::app::Clusters::ContentLauncher::StatusEnum::kSuccess;
    responser.Success(response);
}

void ContentLauncherManager::HandleLaunchUrl(
    const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
    const std::list<BrandingInformation> & brandingInformation,
    chip::app::CommandResponseHelper<chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type> & responser)
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleLaunchUrl");

    string contentUrlString(contentUrl.data(), contentUrl.size());
    string displayStringString(displayString.data(), displayString.size());

    // TODO: Insert code here
    Commands::LaunchResponse::Type response;
    response.data   = chip::CharSpan("exampleData", strlen("exampleData"));
    response.status = chip::app::Clusters::ContentLauncher::StatusEnum::kSuccess;
    responser.Success(response);
}

CHIP_ERROR ContentLauncherManager::HandleGetAcceptHeaderList(chip::app::AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleGetAcceptHeaderList");
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (std::string & entry : mAcceptHeaderList)
        {
            chip::CharSpan data = chip::CharSpan(entry.c_str(), entry.length());
            ReturnErrorOnFailure(encoder.Encode(data));
        }
        return CHIP_NO_ERROR;
    });
}

uint32_t ContentLauncherManager::HandleGetSupportedStreamingProtocols()
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleGetSupportedStreamingProtocols");
    return mSupportedStreamingProtocols;
}
