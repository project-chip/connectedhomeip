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

#include "LoggingCooktop.h"

namespace chip::app {

// LoggingCookSurfacePart

LoggingCookSurfacePart::LoggingCookSurfacePart(TimerDelegate & timerDelegate, const char * name) :
    CookSurfacePart(timerDelegate, *this, *this), mName(name)
{}

void LoggingCookSurfacePart::OnOnOffChanged(bool on)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnOnOffChanged -> %s", mName, on ? "ON" : "OFF");
}

void LoggingCookSurfacePart::OnOffStartup(bool on)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnOffStartup -> %s", mName, on ? "ON" : "OFF");
}

void LoggingCookSurfacePart::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnIdentifyStart", mName);
}

void LoggingCookSurfacePart::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnIdentifyStop", mName);
}

void LoggingCookSurfacePart::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "CookSurface (%s): OnTriggerEffect", mName);
}

// LoggingCooktop

LoggingCooktop::LoggingCooktop(TimerDelegate & timerDelegate) :
    Cooktop(timerDelegate, mLoggingSurface1, mLoggingSurface2, mLoggingSurface1, mLoggingSurface2),
    mLoggingSurface1(timerDelegate, "Left"), mLoggingSurface2(timerDelegate, "Right")
{}

} // namespace chip::app
