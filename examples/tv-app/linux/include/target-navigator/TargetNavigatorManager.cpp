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

list<Structs::TargetInfo::Type> TargetNavigatorManager::HandleGetTargetList()
{
    list<Structs::TargetInfo::Type> list;

    // NOTE: the ids for each target start at 1 so that we can reserve 0 as "no current target"
    int i = 0;
    for (std::string & entry : mTargets)
    {
        Structs::TargetInfo::Type outputInfo;
        outputInfo.identifier = static_cast<uint8_t>(i + 1);
        outputInfo.name       = CharSpan(entry.c_str(), entry.length());
        list.push_back(outputInfo);
        i++;
    }
    return list;
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
        response.data = CharSpan("error", strlen("error"));
        // TODO: should be TARGET_NOT_FOUND
        response.status = StatusEnum::kAppNotAvailable;
        helper.Success(response);
        return;
    }
    mCurrentTarget = static_cast<uint8_t>(target);

    response.data   = CharSpan("data response", strlen("data response"));
    response.status = StatusEnum::kSuccess;
    helper.Success(response);
}
