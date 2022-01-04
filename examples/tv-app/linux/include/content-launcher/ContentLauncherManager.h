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

class ContentLauncherManager : public chip::app::Clusters::ContentLauncher::Delegate
{
public:
    chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type
    HandleLaunchContent(chip::EndpointId endpointId, const std::list<Parameter> & parameterList, bool autoplay,
                        const chip::CharSpan & data) override;
    chip::app::Clusters::ContentLauncher::Commands::LaunchResponse::Type
    HandleLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                    const std::list<BrandingInformation> & brandingInformation) override;
    std::list<std::string> HandleGetAcceptHeaderList() override;
    uint32_t HandleGetSupportedStreamingProtocols() override;
};
