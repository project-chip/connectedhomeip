/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "TvChannelManager.h"

#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

#include <app/util/af.h>
#include <app/util/basic-types.h>

#include <map>
#include <string>

using namespace std;

CHIP_ERROR TvChannelManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // TODO: Store feature map once it is supported
    map<string, bool> featureMap;
    featureMap["CL"] = true;
    featureMap["LI"] = true;

    SuccessOrExit(err);
exit:
    return err;
}

EmberAfTvChannelInfo TvChannelManager::proxyChangeChannelRequest(std::string match)
{
    // TODO: Insert code here
    EmberAfTvChannelInfo channel = {};
    return channel;
}

bool TvChannelManager::proxyChangeChannelByNumberRequest(uint16_t majorNumer, uint16_t minorNumber)
{
    // TODO: Insert code here
    return true;
}
bool TvChannelManager::proxySkipChannelRequest(uint16_t count)
{
    // TODO: Insert code here
    return true;
}

static void sendResponse(const char * responseName, EmberAfTvChannelInfo channelInfo)
{
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_SERVER_TO_CLIENT), ZCL_TV_CHANNEL_CLUSTER_ID,
                              ZCL_CHANGE_CHANNEL_RESPONSE_COMMAND_ID, "uusss", channelInfo.majorNumber, channelInfo.minorNumber,
                              channelInfo.affiliateCallSign, channelInfo.callSign, channelInfo.name);

    EmberStatus status = emberAfSendResponse();
    if (status != EMBER_SUCCESS)
    {
        emberAfMediaPlaybackClusterPrintln("Failed to send %s: 0x%X", responseName, status);
    }
}

bool emberAfTvChannelClusterChangeChannelCallback(char * match)
{
    EmberAfTvChannelInfo channelInfo = TvChannelManager().proxyChangeChannelRequest(match);
    sendResponse("ChangeChannel", channelInfo);
    return true;
}

bool emberAfTvChannelClusterChangeChannelByNumberCallback(unsigned short majorNumber, unsigned short minorNumber)
{
    bool success         = TvChannelManager().proxyChangeChannelByNumberRequest(majorNumber, minorNumber);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfTvChannelClusterSkipChannelCallback(unsigned short count)
{
    bool success         = TvChannelManager().proxySkipChannelRequest(count);
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}