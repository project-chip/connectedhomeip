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

#include "TargetNavigatorManager.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/target-navigator-server/target-navigator-server.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>

#include <map>
#include <string>

using namespace std;

CHIP_ERROR TargetNavigatorManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR TargetNavigatorManager::proxyGetTargetInfoList(chip::app::AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const chip::app::TagBoundEncoder & encoder) -> CHIP_ERROR {
        // TODO: Insert code here
        int maximumVectorSize = 2;
        char name[]           = "exampleName";

        for (int i = 0; i < maximumVectorSize; ++i)
        {
            chip::app::Clusters::TargetNavigator::Structs::NavigateTargetTargetInfo::Type targetInfo;
            targetInfo.name       = chip::CharSpan(name, sizeof(name) - 1);
            targetInfo.identifier = static_cast<uint8_t>(1 + i);
            ReturnErrorOnFailure(encoder.Encode(targetInfo));
        }
        return CHIP_NO_ERROR;
    });
}

TargetNavigatorResponse targetNavigatorClusterNavigateTarget(uint8_t target, std::string data)
{
    // TODO: Insert code here
    TargetNavigatorResponse response;
    const char * testData = "data response";
    response.data         = (uint8_t *) testData;
    response.status       = EMBER_ZCL_APPLICATION_LAUNCHER_STATUS_SUCCESS;
    return response;
}
