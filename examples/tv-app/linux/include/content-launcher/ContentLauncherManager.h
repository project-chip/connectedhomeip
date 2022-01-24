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

#include <app/clusters/content-launch-server/content-launch-server.h>

using chip::CharSpan;
using chip::EndpointId;
using chip::app::AttributeValueEncoder;
using chip::app::CommandResponseHelper;
using ContentLauncherDelegate = chip::app::Clusters::ContentLauncher::Delegate;
using LaunchResponseType      = chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type;

class ContentLauncherManager : public ContentLauncherDelegate
{
public:
    ContentLauncherManager() : ContentLauncherManager({ "example", "example" }, 0){};
    ContentLauncherManager(std::list<std::string> acceptHeaderList, uint32_t supportedStreamingProtocols);

    void HandleLaunchContent(const std::list<Parameter> & parameterList, bool autoplay, const CharSpan & data,
                             CommandResponseHelper<LaunchResponseType> & responser) override;
    void HandleLaunchUrl(const CharSpan & contentUrl, const CharSpan & displayString,
                         const std::list<BrandingInformation> & brandingInformation,
                         CommandResponseHelper<LaunchResponseType> & responser) override;
    CHIP_ERROR HandleGetAcceptHeaderList(AttributeValueEncoder & aEncoder) override;
    uint32_t HandleGetSupportedStreamingProtocols() override;

protected:
    std::list<std::string> mAcceptHeaderList;
    uint32_t mSupportedStreamingProtocols;

private:
    EndpointId mEndpointId;
};
