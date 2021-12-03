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
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <map>
#include <string>
#include <vector>

using namespace chip;

CHIP_ERROR TvChannelManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // TODO: Store feature map once it is supported
    std::map<std::string, bool> featureMap;
    featureMap["CL"] = true;
    featureMap["LI"] = true;

    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR TvChannelManager::proxyGetTvChannelList(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        // TODO: Insert code here
        int maximumVectorSize    = 2;
        char affiliateCallSign[] = "exampleASign";
        char callSign[]          = "exampleCSign";
        char name[]              = "exampleName";

        for (int i = 0; i < maximumVectorSize; ++i)
        {
            chip::app::Clusters::TvChannel::Structs::TvChannelInfo::Type channelInfo;
            channelInfo.affiliateCallSign = CharSpan(affiliateCallSign, sizeof(affiliateCallSign) - 1);
            channelInfo.callSign          = CharSpan(callSign, sizeof(callSign) - 1);
            channelInfo.name              = CharSpan(name, sizeof(name) - 1);
            channelInfo.majorNumber       = static_cast<uint8_t>(1 + i);
            channelInfo.minorNumber       = static_cast<uint16_t>(2 + i);
            ReturnErrorOnFailure(encoder.Encode(channelInfo));
        }
        return CHIP_NO_ERROR;
    });
}

TvChannelInfo tvChannelClusterChangeChannel(std::string match)
{
    // TODO: Insert code here
    TvChannelInfo channel = {};
    return channel;
}
bool tvChannelClusterChangeChannelByNumber(uint16_t majorNumber, uint16_t minorNumber)
{
    // TODO: Insert code here
    return true;
}
bool tvChannelClusterSkipChannel(uint16_t count)
{
    // TODO: Insert code here
    return true;
}
