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

#include "LoggingOnOffSensor.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

LoggingOnOffSensor::LoggingOnOffSensor(const Context & context) : OnOffSensor(context, *this) {}

void LoggingOnOffSensor::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "On/Off Sensor endpoint %u: identification started", GetEndpointId());
}

void LoggingOnOffSensor::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "On/Off Sensor endpoint %u: identification stopped", GetEndpointId());
}

void LoggingOnOffSensor::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(AppServer, "On/Off Sensor endpoint %u: identify effect", GetEndpointId());
}

} // namespace chip::app
