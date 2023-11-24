/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "microwave-oven-control-delegate.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MicrowaveOvenControl;

// Microwave Oven Control command callbacks
Protocols::InteractionModel::Status
ExampleMicrowaveOvenControlDelegate::HandleSetCookingParametersCommandCallback(uint8_t cookMode, uint32_t cookTime, uint8_t powerSetting)
{
    return (*mHandleSetCookingParametersCallback.get())(cookMode, cookTime, powerSetting);
}

Protocols::InteractionModel::Status ExampleMicrowaveOvenControlDelegate::HandleAddMoreTimeCommandCallback(uint32_t finalCookTime)
{
    return (*mHandleAddMoreTimeCallback.get())(finalCookTime);
}

void ExampleMicrowaveOvenControlDelegate::SetMicrowaveOvenControlSetCookingParametersCallback(
    std::function<Protocols::InteractionModel::Status(uint8_t, uint32_t, uint8_t)> aCallback)
{
    mHandleSetCookingParametersCallback = std::make_unique<std::function<Protocols::InteractionModel::Status(uint8_t, uint32_t, uint8_t)>>(aCallback);
}

void ExampleMicrowaveOvenControlDelegate::SetMicrowaveOvenControlAddMoreTimeCallback(
    std::function<Protocols::InteractionModel::Status(uint32_t)> aCallback)
{
    mHandleAddMoreTimeCallback = std::make_unique<std::function<Protocols::InteractionModel::Status(uint32_t)>>(aCallback);
}
