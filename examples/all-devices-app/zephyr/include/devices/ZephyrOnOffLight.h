/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/capabilities/on-off-load/OnOffLoad.h>
#include <device/capabilities/on-off-load/impl/LoggingOnOffDelegate.h>
#include <zephyr/devicetree.h>

namespace chip::app::AllDevices {

/// On/Off Light that drives the board's led0 alias for normal On/Off state.
class ZephyrOnOffLight : private LoggingOnOffDelegate, public OnOffLoad
{
public:
    explicit ZephyrOnOffLight(const OnOffLoad::Context & context);
    ~ZephyrOnOffLight() override;

protected:
    void OnOffStartup(bool on) override;
    void OnOnOffChanged(bool on) override;

private:
    void Apply(bool on);
    void SetLed(bool on);
    bool mLedReady = false;
};

} // namespace chip::app::AllDevices
