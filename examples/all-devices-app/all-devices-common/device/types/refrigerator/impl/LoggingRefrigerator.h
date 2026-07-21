/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/refrigerator/Refrigerator.h>
#include <device/types/temperature-controlled-cabinet/impl/LoggingTemperatureControlledCabinetPart.h>

namespace chip::app {

class LoggingRefrigerator : public Refrigerator
{
public:
    explicit LoggingRefrigerator(TimerDelegate & timerDelegate);
    LoggingRefrigerator(TimerDelegate & timerDelegate, Config config);
    ~LoggingRefrigerator() override = default;

private:
    LoggingTemperatureControlledCabinetPart mLoggingCabinet;
};

} // namespace chip::app
