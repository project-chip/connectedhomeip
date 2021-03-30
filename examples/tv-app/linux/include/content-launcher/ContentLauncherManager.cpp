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

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <app/util/af.h>
#include <app/util/basic-types.h>

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

    // TODO: Update once storing a list attribute is supported
    // list<string> acceptedHeaderList = ContentLauncherManager().proxyGetAcceptsHeader();
    // emberAfWriteServerAttribute(endpoint, ZCL_CONTENT_LAUNCH_CLUSTER_ID, ZCL_CONTENT_LAUNCHER_ACCEPTS_HEADER_ATTRIBUTE_ID,
    //                             (uint8_t *) &acceptedHeaderList, ZCL_STRUCT_ATTRIBUTE_TYPE);

    // TODO: Update once storing a list attribute is supported
    // list<string> supportedStreamingTypes = ContentLauncherManager().proxyGetAcceptsHeader();
    // emberAfWriteServerAttribute(endpoint, ZCL_CONTENT_LAUNCH_CLUSTER_ID,
    //                             ZCL_CONTENT_LAUNCHER_SUPPORTED_STREAMING_TYPES_ATTRIBUTE_ID, (uint8_t *)
    //                             &supportedStreamingTypes, ZCL_STRUCT_ATTRIBUTE_TYPE);

    SuccessOrExit(err);
exit:
    return err;
}

list<string> ContentLauncherManager::proxyGetAcceptsHeader()
{
    // TODO: Insert code here
    list<string> acceptedHeaderList;
    acceptedHeaderList.push_back("HeaderExample");
    return acceptedHeaderList;
}

list<EmberAfContentLaunchStreamingType> ContentLauncherManager::proxyGetSupportedStreamingTypes()
{
    // TODO: Insert code here
    list<EmberAfContentLaunchStreamingType> supportedStreamingTypes;
    supportedStreamingTypes.push_back(EMBER_ZCL_CONTENT_LAUNCH_STREAMING_TYPE_DASH);
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
        emberAfContentLauncherClusterPrintln("Failed to send %s: 0x%X", responseName, status);
    }
}

bool emberAfContentLaunchClusterLaunchContentCallback(unsigned char autoplay, unsigned char * data)
{

    string dataString(reinterpret_cast<char *>(data));
    list<EmberAfContentLaunchParamater> parameterList;
    ContentLaunchResponse response = ContentLauncherManager().proxyLaunchContentRequest(parameterList, autoplay, dataString);
    sendResponse("LaunchContent", response, ZCL_LAUNCH_CONTENT_RESPONSE_COMMAND_ID);
    return true;
}

bool emberAfContentLaunchClusterLaunchURLCallback(unsigned char * contentUrl, unsigned char * displayString)
{
    string contentUrlString(reinterpret_cast<char *>(contentUrl));
    string displayStringString(reinterpret_cast<char *>(displayString));
    EmberAfContentLaunchBrandingInformation brandingInformation;
    ContentLaunchResponse response =
        ContentLauncherManager().proxyLaunchUrlRequest(contentUrlString, displayStringString, brandingInformation);
    sendResponse("LaunchURL", response, ZCL_LAUNCH_URL_RESPONSE_COMMAND_ID);
    return true;
}
