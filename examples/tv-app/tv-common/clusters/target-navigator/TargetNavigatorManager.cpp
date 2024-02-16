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
#include <app-common/zap-generated/attributes/Accessors.h>

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::TargetNavigator;

TargetNavigatorManager::TargetNavigatorManager(std::list<std::string> targets, uint8_t currentTarget)
{
    mTargets       = targets;
    mCurrentTarget = currentTarget;
}

CHIP_ERROR TargetNavigatorManager::HandleGetTargetList(AttributeValueEncoder & aEncoder)
{
    // NOTE: the ids for each target start at 1 so that we can reserve 0 as "no current target"
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        int i = 0;
        for (std::string & entry : mTargets)
        {
            Structs::TargetInfoStruct::Type outputInfo;
            outputInfo.identifier = static_cast<uint8_t>(i + 1);
            outputInfo.name       = CharSpan::fromCharString(entry.c_str());
            ReturnErrorOnFailure(encoder.Encode(outputInfo));
            i++;
        }
        return CHIP_NO_ERROR;
    });
}

uint8_t TargetNavigatorManager::HandleGetCurrentTarget()
{
    return mCurrentTarget;
}

void TargetNavigatorManager::HandleNavigateTarget(CommandResponseHelper<NavigateTargetResponseType> & helper,
                                                  const uint64_t & target, const CharSpan & data)
{
    NavigateTargetResponseType response;
    if (target == kNoCurrentTarget || target > mTargets.size())
    {
        response.data   = chip::MakeOptional(CharSpan::fromCharString("error"));
        response.status = StatusEnum::kTargetNotFound;
        helper.Success(response);
        return;
    }
    mCurrentTarget = static_cast<uint8_t>(target);

    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}

uint16_t TargetNavigatorManager::GetClusterRevision(chip::EndpointId endpoint)
{
    if (endpoint >= MATTER_DM_CONTENT_LAUNCHER_CLUSTER_SERVER_ENDPOINT_COUNT)
    {
        return kClusterRevision;
    }

    uint16_t clusterRevision = 0;
    bool success =
        (Attributes::ClusterRevision::Get(endpoint, &clusterRevision) == chip::Protocols::InteractionModel::Status::Success);
    if (!success)
    {
        ChipLogError(Zcl, "TargetNavigatorManager::GetClusterRevision error reading cluster revision");
    }

    return clusterRevision;
}
