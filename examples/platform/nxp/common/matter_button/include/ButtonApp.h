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

#include "Button.h"

namespace chip::NXP::App {
/**
 * @brief This class implements a custom app button behavior.
 *
 * | Action       | Effect                            |
 * | ------------ | --------------------------------- |
 * | Short press  | Switch a cluster attribute state  |
 * | Long press   | Schedule a soft reset taking into |
 * |              | account Matter shutdown mechanism |
 * | Double click | Do nothing                        |
 *
 */
class ButtonApp : public Button
{
public:
    virtual CHIP_ERROR Init() override;
    virtual void HandleShortPress() override;
    virtual void HandleLongPress() override;
    virtual void HandleDoubleClick() override;
};

} // namespace chip::NXP::App
