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
ExampleMicrowaveOvenControlDelegate::HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTime, uint8_t powerSetting)
{
    return (mSetCookingParametersInstance->*mSetCookingParametersCommandCallback)(cookMode, cookTime, powerSetting);
}

Protocols::InteractionModel::Status ExampleMicrowaveOvenControlDelegate::HandleAddMoreTimeCallback(uint32_t finalCookTime)
{
    return (mAddMoreTimeInstance->*mAddMoreTimeCommandCallback)(finalCookTime);
}

void ExampleMicrowaveOvenControlDelegate::SetMicrowaveOvenControlSetCookingParametersCallback(
    HandleSetCookingParametersCommand aCallback, MicrowaveOvenDevice * aInstance)
{
    mSetCookingParametersCommandCallback = aCallback;
    mSetCookingParametersInstance        = aInstance;
}

void ExampleMicrowaveOvenControlDelegate::SetMicrowaveOvenControlAddMoreTimeCallback(HandleAddMoreTimeCommand aCallback,
                                                                                     MicrowaveOvenDevice * aInstance)
{
    mAddMoreTimeCommandCallback = aCallback;
    mAddMoreTimeInstance        = aInstance;
}
