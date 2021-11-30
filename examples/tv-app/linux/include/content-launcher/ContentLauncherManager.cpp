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

#include <app/Command.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>

#include <map>

using namespace std;

CHIP_ERROR ContentLauncherManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // TODO: Store feature map once it is supported
    map<string, bool> featureMap;
    featureMap["CS"] = true;
    featureMap["UP"] = true;
    featureMap["WA"] = true;

    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR ContentLauncherManager::proxyGetAcceptsHeader(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const chip::app::TagBoundEncoder & encoder) -> CHIP_ERROR {
        // TODO: Insert code here
        char headerExample[]  = "exampleHeader";
        int maximumVectorSize = 1;

        for (uint16_t i = 0; i < maximumVectorSize; ++i)
        {
            ReturnErrorOnFailure(encoder.Encode(chip::ByteSpan(chip::Uint8::from_char(headerExample), sizeof(headerExample) - 1)));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ContentLauncherManager::proxyGetSupportedStreamingTypes(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const chip::app::TagBoundEncoder & encoder) -> CHIP_ERROR {
        // TODO: Insert code here
        ReturnErrorOnFailure(encoder.Encode(EMBER_ZCL_CONTENT_LAUNCH_STREAMING_TYPE_DASH));
        ReturnErrorOnFailure(encoder.Encode(EMBER_ZCL_CONTENT_LAUNCH_STREAMING_TYPE_HLS));
        return CHIP_NO_ERROR;
    });
}

ContentLaunchResponse ContentLauncherManager::proxyLaunchContentRequest(list<ContentLaunchParamater> parameterList, bool autoplay,
                                                                        string data)
{
    // TODO: Insert code here
    ContentLaunchResponse response;
    response.err    = CHIP_NO_ERROR;
    response.data   = "Example data";
    response.status = EMBER_ZCL_CONTENT_LAUNCH_STATUS_SUCCESS;
    return response;
}
ContentLaunchResponse ContentLauncherManager::proxyLaunchUrlRequest(string contentUrl, string displayString,
                                                                    ContentLaunchBrandingInformation brandingInformation)
{
    // TODO: Insert code here
    ContentLaunchResponse response;
    response.err    = CHIP_NO_ERROR;
    response.data   = "Example data";
    response.status = EMBER_ZCL_CONTENT_LAUNCH_STATUS_SUCCESS;
    return response;
}

ContentLaunchResponse contentLauncherClusterLaunchContent(std::list<ContentLaunchParamater> parameterList, bool autoplay,
                                                          const chip::CharSpan & data)
{
    string dataString(data.data(), data.size());
    return ContentLauncherManager().proxyLaunchContentRequest(parameterList, autoplay, dataString);
}

ContentLaunchResponse contentLauncherClusterLaunchUrl(const chip::CharSpan & contentUrl, const chip::CharSpan & displayString,
                                                      ContentLaunchBrandingInformation & brandingInformation)
{
    string contentUrlString(contentUrl.data(), contentUrl.size());
    string displayStringString(displayString.data(), displayString.size());
    return ContentLauncherManager().proxyLaunchUrlRequest(contentUrlString, displayStringString, brandingInformation);
}
