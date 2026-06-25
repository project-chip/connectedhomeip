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

#include "LoggingTemperatureControlledCabinetPart.h"

namespace chip::app {

LoggingTemperatureControlledCabinetPart::LoggingTemperatureControlledCabinetPart(TimerDelegate & timerDelegate, const char * name) :
    LoggingTemperatureControlledCabinetPart(timerDelegate, Config{}, name)
{}

LoggingTemperatureControlledCabinetPart::LoggingTemperatureControlledCabinetPart(TimerDelegate & timerDelegate, Config config,
                                                                                 const char * name) :
    TemperatureControlledCabinetPart(timerDelegate, config, *this),
    mName(name)
{}

void LoggingTemperatureControlledCabinetPart::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "TempCabinet (%s): OnIdentifyStart", mName);
}

void LoggingTemperatureControlledCabinetPart::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "TempCabinet (%s): OnIdentifyStop", mName);
}

void LoggingTemperatureControlledCabinetPart::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "TempCabinet (%s): OnTriggerEffect", mName);
}

} // namespace chip::app
