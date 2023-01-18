/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include <app-common/zap-generated/cluster-enums.h>
#include <lib/support/Span.h>

#include <stdbool.h>
#include <stdint.h>

class Action
{
public:
    Action(uint16_t actionId, std::string name, chip::app::Clusters::Actions::ActionTypeEnum type, uint16_t endpointListId,
           uint16_t supportedCommands, chip::app::Clusters::Actions::ActionStateEnum status, bool isVisible);
    inline void setName(std::string name) { mName = name; };
    inline std::string getName() { return mName; };
    inline chip::app::Clusters::Actions::ActionTypeEnum getType() { return mType; };
    inline chip::app::Clusters::Actions::ActionStateEnum getStatus() { return mStatus; };
    inline uint16_t getActionId() { return mActionId; };
    inline uint16_t getEndpointListId() { return mEndpointListId; };
    inline uint16_t getSupportedCommands() { return mSupportedCommands; };
    inline void setIsVisible(bool isVisible) { mIsVisible = isVisible; };
    inline bool getIsVisible() { return mIsVisible; };

private:
    std::string mName;
    chip::app::Clusters::Actions::ActionTypeEnum mType;
    chip::app::Clusters::Actions::ActionStateEnum mStatus;
    uint16_t mActionId;
    uint16_t mEndpointListId;
    uint16_t mSupportedCommands;
    bool mIsVisible;
};
