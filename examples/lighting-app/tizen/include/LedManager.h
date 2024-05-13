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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#include <peripheral_io.h>

namespace example {
class LedManager
{
public:
    explicit LedManager(chip::EndpointId endpointId);
    void SetOnOff(bool on);
    CHIP_ERROR Init();

private:
    void InitOnOff();

    // Numbers of GPIO used to control LED in order: RED, GREEN, BLUE
    static constexpr int number_of_pins       = 3;
    static constexpr int pins[number_of_pins] = { 20, 19, 18 };
    peripheral_gpio_h gpio[number_of_pins]    = {};

    const chip::EndpointId mEndpointId;
};

} // namespace example
