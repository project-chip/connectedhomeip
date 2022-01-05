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

using namespace std;
using namespace chip::app::Clusters::TargetNavigator;

TargetNavigatorManager::TargetNavigatorManager(std::list<std::string> targets, uint8_t currentTarget)
{
    mTargets       = targets;
    mCurrentTarget = currentTarget;
}

std::list<Structs::TargetInfo::Type> TargetNavigatorManager::HandleGetTargetList()
{
    std::list<Structs::TargetInfo::Type> list;

    int i = 0;
    for (std::string entry : mTargets)
    {
        Structs::TargetInfo::Type outputInfo;
        outputInfo.identifier = static_cast<uint8_t>(i + 1);
        outputInfo.name       = chip::CharSpan(entry.c_str(), entry.length());
        list.push_back(outputInfo);
    }
    return list;
}

uint8_t TargetNavigatorManager::HandleGetCurrentTarget()
{
    return mCurrentTarget;
}

Commands::NavigateTargetResponse::Type TargetNavigatorManager::HandleNavigateTarget(const uint64_t & target,
                                                                                    const chip::CharSpan & data)
{
    Commands::NavigateTargetResponse::Type response;
    if (target <= 0 || target > mTargets.size())
    {
        response.data = chip::CharSpan("error", strlen("error"));
        // TODO: should be TARGET_NOT_FOUND
        response.status = chip::app::Clusters::TargetNavigator::StatusEnum::kAppNotAvailable;
        return response;
    }
    mCurrentTarget = static_cast<uint8_t>(target);

    response.data   = chip::CharSpan("data response", strlen("data response"));
    response.status = chip::app::Clusters::TargetNavigator::StatusEnum::kSuccess;
    return response;
}
