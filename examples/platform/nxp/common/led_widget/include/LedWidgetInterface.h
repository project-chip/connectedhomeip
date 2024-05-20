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

#include "EmbeddedTypes.h"

namespace chip::NXP::App {

/**
 * @brief
 *   An interface that defines LED operations.
 */
class LedWidgetInterface
{
public:
    virtual ~LedWidgetInterface() {}

    virtual void Init(uint8_t index = 0, bool inverted = false)         = 0;
    virtual void Set(uint8_t level = 0)                                 = 0;
    virtual void Animate(uint32_t onTimeMS = 0, uint32_t offTimeMS = 0) = 0;

    bool IsTurnedOff() { return !mState; }

protected:
    bool mState = false;
};

} // namespace chip::NXP::App
