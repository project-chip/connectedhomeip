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

#include <app/clusters/identify-server/IdentifyCluster.h>
#include <system/SystemLayer.h>
#include <zephyr/devicetree.h>

namespace chip::app::AllDevices {

class ZephyrIdentifyDelegate : public Clusters::IdentifyDelegate
{
public:
    ZephyrIdentifyDelegate();
    ~ZephyrIdentifyDelegate() override = default;

    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }

private:
    CHIP_ERROR StartBlink();
    CHIP_ERROR StopBlink();
    CHIP_ERROR ToggleLed();
    CHIP_ERROR ScheduleBlinkTimer();
    static void BlinkTimerHandler(System::Layer * systemLayer, void * appState);

    [[maybe_unused]] bool mLedReady       = false;
    [[maybe_unused]] bool mBlinking       = false;
    [[maybe_unused]] uint8_t mToggleCount = 0;
};

} // namespace chip::app::AllDevices
