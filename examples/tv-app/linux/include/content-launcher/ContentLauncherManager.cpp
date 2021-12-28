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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/command-id.h>

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/ContentAppPlatform.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>

#include <map>
#include <string>

using namespace std;
using namespace chip::AppPlatform;

LaunchResponse ContentLauncherManager::HandleLaunchContent(chip::EndpointId endpointId, const std::list<Parameter> & parameterList,
                                                           bool autoplay, const chip::CharSpan & data)
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleLaunchContent ");
    string dataString(data.data(), data.size());

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = chip::AppPlatform::AppPlatform::GetInstance().GetContentAppByEndpointId(endpointId);
    if (app != NULL)
    {
        return app->GetContentLauncher()->LaunchContent(parameterList, autoplay, dataString);
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    // TODO: Insert code here
    LaunchResponse response;
    response.err    = CHIP_NO_ERROR;
    response.data   = chip::CharSpan("exampleData", strlen("exampleData"));
    response.status = chip::app::Clusters::ContentLauncher::StatusEnum::kSuccess;
    return response;
}

LaunchResponse ContentLauncherManager::HandleLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                                       const std::list<BrandingInformation> & brandingInformation)
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleLaunchUrl");

    string contentUrlString(contentUrl.data(), contentUrl.size());
    string displayStringString(displayString.data(), displayString.size());

    // TODO: Insert code here
    LaunchResponse response;
    response.err    = CHIP_NO_ERROR;
    response.data   = chip::CharSpan("exampleData", strlen("exampleData"));
    response.status = chip::app::Clusters::ContentLauncher::StatusEnum::kSuccess;
    return response;
}

std::list<std::string> ContentLauncherManager::HandleGetAcceptHeaderList()
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleGetAcceptHeaderList");
    return { "example", "example" };
}

uint32_t ContentLauncherManager::HandleGetSupportedStreamingProtocols()
{
    ChipLogProgress(Zcl, "ContentLauncherManager::HandleGetSupportedStreamingProtocols");
    uint32_t streamingProtocols = 0;
    return streamingProtocols;
}
