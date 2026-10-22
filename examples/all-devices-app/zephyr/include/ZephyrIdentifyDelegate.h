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

#if DT_NODE_EXISTS(DT_ALIAS(led0))
#define ALL_DEVICES_IDENTIFY_LED_NODE DT_ALIAS(led0)
#endif

#if defined(ALL_DEVICES_IDENTIFY_LED_NODE)
#define ALL_DEVICES_HAS_IDENTIFY_LED 1
#else
#define ALL_DEVICES_HAS_IDENTIFY_LED 0
#endif

namespace chip::app::AllDevices {

class ZephyrIdentifyDelegate : public Clusters::IdentifyDelegate
{
public:
    ZephyrIdentifyDelegate();
    ~ZephyrIdentifyDelegate() override;

    void SetNormalState(bool on);
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }

private:
    void SetLed(bool on);
    void StartBlink();
    void StopBlink();
    static void BlinkTimerHandler(System::Layer * systemLayer, void * appState);

    bool mNormalState = false;
#if ALL_DEVICES_HAS_IDENTIFY_LED
    bool mLedReady   = false;
    bool mBlinking   = false;
    bool mBlinkPhase = false;
#endif
};

} // namespace chip::app::AllDevices
