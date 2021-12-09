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

#include <app-common/zap-generated/af-structs.h>
#include <app/AttributeAccessInterface.h>
#include <app/clusters/content-launch-server/content-launch-server.h>

#include <lib/core/CHIPError.h>
#include <list>
#include <string>
#include <vector>

class ContentLauncherManager
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR proxyGetAcceptsHeader(chip::app::AttributeValueEncoder & aEncoder);
    ContentLaunchResponse proxyLaunchContentRequest(std::list<ContentLaunchParamater> parameterList, bool autoplay,
                                                    std::string data);
    ContentLaunchResponse proxyLaunchUrlRequest(std::string contentUrl, std::string displayString,
                                                ContentLaunchBrandingInformation brandingInformation);
};
