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

#pragma once

#include <app/clusters/target-navigator-server/target-navigator-server.h>

class TargetNavigatorManager : public chip::app::Clusters::TargetNavigator::Delegate
{
public:
    TargetNavigatorManager() : TargetNavigatorManager({ "exampleName", "exampleName" }, 0){};
    TargetNavigatorManager(std::list<std::string> targets, uint8_t currentTarget);

    std::list<chip::app::Clusters::TargetNavigator::Structs::TargetInfo::Type> HandleGetTargetList() override;
    uint8_t HandleGetCurrentTarget() override;
    chip::app::Clusters::TargetNavigator::Commands::NavigateTargetResponse::Type
    HandleNavigateTarget(const uint64_t & target, const chip::CharSpan & data) override;

protected:
    std::list<std::string> mTargets;
    uint8_t mCurrentTarget;
};
