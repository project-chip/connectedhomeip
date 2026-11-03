/*
 *
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

#include <device/types/water-valve/WaterValve.h>

namespace chip {
namespace app {

class LoggingWaterValve : public Clusters::ValveConfigurationAndControl::Delegate, public WaterValve
{
public:
    explicit LoggingWaterValve(TimerDelegate & timerDelegate, const WaterValveConfig & config = {}) :
        WaterValve(timerDelegate, *this, config){};
    ~LoggingWaterValve() override = default;

    DataModel::Nullable<Percent> HandleOpenValve(DataModel::Nullable<Percent> level) override;
    CHIP_ERROR HandleCloseValve() override;
    void HandleRemainingDurationTick(uint32_t duration) override;
};

} // namespace app
} // namespace chip
