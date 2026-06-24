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

#include <devices/cooktop/CooktopDevice.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

class LoggingCookSurfacePart : public CookSurfacePart, public Clusters::OnOffDelegate, public Clusters::IdentifyDelegate
{
public:
    LoggingCookSurfacePart(TimerDelegate & timerDelegate, const char * name);
    ~LoggingCookSurfacePart() override = default;

    // OnOffDelegate
    void OnOnOffChanged(bool on) override;
    void OnOffStartup(bool on) override;

    // IdentifyDelegate
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }

private:
    const char * mName;
};

class LoggingCooktopDevice : public CooktopDevice
{
public:
    explicit LoggingCooktopDevice(TimerDelegate & timerDelegate);
    ~LoggingCooktopDevice() override = default;

private:
    LoggingCookSurfacePart mLoggingSurface1;
    LoggingCookSurfacePart mLoggingSurface2;
};

} // namespace chip::app
