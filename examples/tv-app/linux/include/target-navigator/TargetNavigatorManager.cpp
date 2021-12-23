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
#include <app/util/ContentAppPlatform.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CodeUtils.h>

#include <map>
#include <string>

using namespace std;
using namespace chip::AppPlatform;

// index starts at 1 for
std::list<std::string> gTargets = { "exampleName", "exampleName" };
uint8_t gCurrentTarget          = 1;

CHIP_ERROR TargetNavigatorManager::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SuccessOrExit(err);
exit:
    return err;
}

CHIP_ERROR TargetNavigatorManager::proxyGetTargetInfoList(chip::EndpointId endpointId, chip::app::AttributeValueEncoder & aEncoder)
{
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = chip::AppPlatform::AppPlatform::GetInstance().GetContentAppByEndpointId(endpointId);
    if (app != NULL)
    {
        return app->GetTargetNavigator()->GetTargetInfoList(aEncoder);
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        int i = 1; // make sure TV_TargetNavigatorCluster.yaml test suite passes - assumes index starts at 1
        for (string entry : gTargets)
        {
            // ReturnErrorOnFailure(encoder.Encode(chip::CharSpan(entry.c_str(), entry.length())));

            chip::app::Clusters::TargetNavigator::Structs::TargetInfo::Type targetInfo;
            targetInfo.name       = chip::CharSpan(entry.c_str(), entry.length());
            targetInfo.identifier = static_cast<uint8_t>(i++);
            ReturnErrorOnFailure(encoder.Encode(targetInfo));
        }
        return CHIP_NO_ERROR;
    });
}

TargetNavigatorResponse targetNavigatorClusterNavigateTarget(chip::EndpointId endpointId, uint8_t target, std::string data)
{
    ChipLogProgress(Zcl, "targetNavigatorClusterNavigateTarget endpoint=%d", endpointId);

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentApp * app = chip::AppPlatform::AppPlatform::GetInstance().GetContentAppByEndpointId(endpointId);
    if (app != NULL)
    {
        return app->GetTargetNavigator()->NavigateTarget(target, data);
    }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    TargetNavigatorResponse response;
    const char * testData = "data response";
    response.data         = (uint8_t *) testData;
    // make sure TV_TargetNavigatorCluster.yaml test suite passes - assumes index starts at 1
    if (target == 0 || target > gTargets.size())
    {
        response.status = chip::to_underlying(chip::app::Clusters::TargetNavigator::StatusEnum::kAppNotAvailable);
    }
    else
    {
        response.status = chip::to_underlying(chip::app::Clusters::TargetNavigator::StatusEnum::kSuccess);
        gCurrentTarget  = target;
    }
    return response;
}
