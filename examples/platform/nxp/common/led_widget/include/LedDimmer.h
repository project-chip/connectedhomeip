/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "LedWidgetInterface.h"

namespace chip::NXP::App {

/**
 * @brief
 *   A class that manages dimmer LED operations.
 */
class LedDimmer : public LedWidgetInterface
{
public:
    void Init(uint8_t index = 0, bool inverted = false) override;
    void Set(uint8_t level = 0) override;
    void Animate(uint32_t onTimeMS = 0, uint32_t offTimeMS = 0) override {}
};

} // namespace chip::NXP::App
