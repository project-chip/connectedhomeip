/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "lib/core/CHIPError.h"
#include <platform/CHIPDeviceLayer.h>
#include <app/clusters/bindings/bindings.h>
#include <variant>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::LevelControl;

CHIP_ERROR InitBindingHandler();
void SwitchWorkerFunction(intptr_t context);
void BindingWorkerFunction(intptr_t context);


struct CommandBase
{
    chip::BitMask<OptionsBitmap> optionsMask;
    chip::BitMask<OptionsBitmap> optionsOverride;

    // Constructor to initialize the BitMask
    CommandBase()
        : optionsMask(0), optionsOverride(0) {}
};

struct BindingCommandData
{
    chip::EndpointId localEndpointId = 1;
    chip::CommandId commandId;
    chip::ClusterId clusterId;
    bool isGroup = false;

    struct MoveToLevel : public CommandBase
    {
        uint8_t level;
        DataModel::Nullable<uint16_t> transitionTime;
    };
    struct Move : public CommandBase
    {
        MoveModeEnum moveMode;
        DataModel::Nullable<uint8_t> rate;
    };
    struct Step : public CommandBase
    {
        StepModeEnum stepMode;
        uint8_t stepSize;
        DataModel::Nullable<uint16_t> transitionTime;
    };
    struct Stop : public CommandBase
    {
        // Inherits optionsMask and optionsOverride from CommandBase
    };
    // Use std::variant to hold different command types
    std::variant<MoveToLevel, Move, Step, Stop> commandData;
};