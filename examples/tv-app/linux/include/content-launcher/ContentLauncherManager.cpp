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
#include <app-common/zap-generated/command-id.h>

#include <app/Command.h>
#include <app/CommandHandler.h>
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

vector<chip::ByteSpan> ContentLauncherManager::proxyGetAcceptsHeader()
{
    // TODO: Insert code here
    vector<chip::ByteSpan> acceptedHeader;
    char headerExample[]  = "exampleHeader";
    int maximumVectorSize = 1;

    for (uint16_t i = 0; i < maximumVectorSize; ++i)
    {
        acceptedHeader.push_back(chip::ByteSpan(chip::Uint8::from_char(headerExample), sizeof(headerExample)));
    }
    return acceptedHeader;
}

vector<EmberAfContentLaunchStreamingType> ContentLauncherManager::proxyGetSupportedStreamingTypes()
{
    // TODO: Insert code here
    vector<EmberAfContentLaunchStreamingType> supportedStreamingTypes;
    supportedStreamingTypes.push_back(EMBER_ZCL_CONTENT_LAUNCH_STREAMING_TYPE_DASH);
    supportedStreamingTypes.push_back(EMBER_ZCL_CONTENT_LAUNCH_STREAMING_TYPE_HLS);
    return supportedStreamingTypes;
}

ContentLaunchResponse ContentLauncherManager::proxyLaunchContentRequest(list<EmberAfContentLaunchParamater> parameterList,
                                                                        bool autoplay, string data)
{
    // TODO: Insert code here
    ContentLaunchResponse response;
    response.data   = "Example data";
    response.status = EMBER_ZCL_CONTENT_LAUNCH_STATUS_SUCCESS;
    return response;
}
ContentLaunchResponse ContentLauncherManager::proxyLaunchUrlRequest(string contentUrl, string displayString,
                                                                    EmberAfContentLaunchBrandingInformation brandingInformation)
{
    // TODO: Insert code here
    ContentLaunchResponse response;
    response.data   = "Example data";
    response.status = EMBER_ZCL_CONTENT_LAUNCH_STATUS_SUCCESS;
    return response;
}

static void sendResponse(const char * responseName, ContentLaunchResponse launchResponse, chip::CommandId commandId)
{
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_CONTENT_LAUNCH_CLUSTER_ID,
                              commandId, "us", launchResponse.status, &launchResponse.data);

    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        ChipLogError(Zcl, "Failed to send %s. Error:%d", responseName, static_cast<int>(status));
    }
}

bool emberAfContentLauncherClusterLaunchContentCallback(chip::EndpointId endpoint, chip::app::CommandHandler * command,
                                                        bool autoplay, unsigned char * data)
{

    string dataString(reinterpret_cast<char *>(data));
    list<EmberAfContentLaunchParamater> parameterList;
    ContentLaunchResponse response = ContentLauncherManager().proxyLaunchContentRequest(parameterList, autoplay, dataString);
    sendResponse("LaunchContent", response, ZCL_LAUNCH_CONTENT_RESPONSE_COMMAND_ID);
    return true;
}

bool emberAfContentLauncherClusterLaunchURLCallback(chip::EndpointId endpoint, chip::app::CommandHandler * command,
                                                    unsigned char * contentUrl, unsigned char * displayString)
{
    string contentUrlString(reinterpret_cast<char *>(contentUrl));
    string displayStringString(reinterpret_cast<char *>(displayString));
    EmberAfContentLaunchBrandingInformation brandingInformation;
    ContentLaunchResponse response =
        ContentLauncherManager().proxyLaunchUrlRequest(contentUrlString, displayStringString, brandingInformation);
    sendResponse("LaunchURL", response, ZCL_LAUNCH_URL_RESPONSE_COMMAND_ID);
    return true;
}
