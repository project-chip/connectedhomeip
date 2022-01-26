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

std::list<Structs::TargetInfo::Type> TargetNavigatorManager::HandleGetTargetList()
{
    std::list<Structs::TargetInfo::Type> list;
    // TODO: Insert code here
    int maximumVectorSize = 2;

    for (int i = 0; i < maximumVectorSize; ++i)
    {
        Structs::TargetInfo::Type outputInfo;
        outputInfo.identifier = static_cast<uint8_t>(i + 1);
        outputInfo.name       = chip::CharSpan::fromCharString("exampleName");
        list.push_back(outputInfo);
    }
    return list;
}

uint8_t TargetNavigatorManager::HandleGetCurrentTarget()
{
    return 0;
}

Commands::NavigateTargetResponse::Type TargetNavigatorManager::HandleNavigateTarget(const uint64_t & target,
                                                                                    const chip::CharSpan & data)
{
    // TODO: Insert code here
    Commands::NavigateTargetResponse::Type response;
    response.data   = chip::CharSpan::fromCharString("data response");
    response.status = chip::app::Clusters::TargetNavigator::StatusEnum::kSuccess;
    return response;
}
