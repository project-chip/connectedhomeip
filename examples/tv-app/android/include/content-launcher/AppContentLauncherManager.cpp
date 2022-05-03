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

#include "AppContentLauncherManager.h"

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters::ContentLauncher;

AppContentLauncherManager::AppContentLauncherManager(list<std::string> acceptHeaderList, uint32_t supportedStreamingProtocols)
{
    mAcceptHeaderList            = acceptHeaderList;
    mSupportedStreamingProtocols = supportedStreamingProtocols;
}

void AppContentLauncherManager::HandleLaunchContent(CommandResponseHelper<LaunchResponseType> & helper,
                                                    const DecodableList<ParameterType> & parameterList, bool autoplay,
                                                    const CharSpan & data)
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleLaunchContent for endpoint %d", mEndpointId);
    string dataString(data.data(), data.size());

    LaunchResponseType response;
    // TODO: Insert code here
    response.data   = chip::MakeOptional(CharSpan::fromCharString("exampleData"));
    response.status = ContentLauncher::ContentLaunchStatusEnum::kSuccess;
    helper.Success(response);
}

void AppContentLauncherManager::HandleLaunchUrl(CommandResponseHelper<LaunchResponseType> & helper, const CharSpan & contentUrl,
                                                const CharSpan & displayString, const BrandingInformationType & brandingInformation)
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleLaunchUrl");

    string contentUrlString(contentUrl.data(), contentUrl.size());
    string displayStringString(displayString.data(), displayString.size());

    // TODO: Insert code here
    LaunchResponseType response;
    response.data   = chip::MakeOptional(CharSpan::fromCharString("exampleData"));
    response.status = ContentLauncher::ContentLaunchStatusEnum::kSuccess;
    helper.Success(response);
}

CHIP_ERROR AppContentLauncherManager::HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetAcceptHeaderList");
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (std::string & entry : mAcceptHeaderList)
        {
            CharSpan data = CharSpan::fromCharString(entry.c_str());
            ReturnErrorOnFailure(encoder.Encode(data));
        }
        return CHIP_NO_ERROR;
    });
}

uint32_t AppContentLauncherManager::HandleGetSupportedStreamingProtocols()
{
    ChipLogProgress(Zcl, "AppContentLauncherManager::HandleGetSupportedStreamingProtocols");
    return mSupportedStreamingProtocols;
}
