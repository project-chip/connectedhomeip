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

#include "delegates/ZephyrIdentifyDelegate.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#if DT_NODE_EXISTS(DT_ALIAS(led0))
#include <zephyr/drivers/gpio.h>
#define ALL_DEVICES_HAS_IDENTIFY_LED 1
namespace {
const struct gpio_dt_spec sIdentifyLed = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
constexpr uint8_t sTargetBlinkToogleCount = 6;
} // namespace
#else
#define ALL_DEVICES_HAS_IDENTIFY_LED 0
#endif

namespace chip::app::AllDevices {

ZephyrIdentifyDelegate::ZephyrIdentifyDelegate()
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    VerifyOrReturn(gpio_is_ready_dt(&sIdentifyLed), ChipLogError(DeviceLayer, "Identify LED GPIO not ready; Identify effects disabled"));
    VerifyOrReturn(gpio_pin_configure_dt(&sIdentifyLed, GPIO_OUTPUT_INACTIVE) == 0, ChipLogError(DeviceLayer, "Identify LED GPIO configure failed; Identify effects disabled"));
    mLedReady = true;
#else
    ChipLogProgress(DeviceLayer, "No led0 GPIO available; Identify effects disabled");
#endif
}

void ZephyrIdentifyDelegate::OnIdentifyStart(Clusters::IdentifyCluster & /*cluster*/)
{
    ChipLogProgress(DeviceLayer, "ZephyrIdentifyDelegate: Identify START");
    LogErrorOnFailure(StartBlink());
}

void ZephyrIdentifyDelegate::OnIdentifyStop(Clusters::IdentifyCluster & /*cluster*/)
{
    ChipLogProgress(DeviceLayer, "ZephyrIdentifyDelegate: Identify STOP");
    LogErrorOnFailure(StopBlink());
}

void ZephyrIdentifyDelegate::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    const auto effect = cluster.GetEffectIdentifier();
    ChipLogProgress(DeviceLayer, "ZephyrIdentifyDelegate: TriggerEffect (%u)", static_cast<unsigned>(effect));
    switch (effect){
        case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
            LogErrorOnFailure(StopBlink());
            break;
        default:
            LogErrorOnFailure(StartBlink());

    }
}

CHIP_ERROR ZephyrIdentifyDelegate::StartBlink()
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    VerifyOrReturnError(mLedReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrDo(!mBlinking, DeviceLayer::SystemLayer().CancelTimer(BlinkTimerHandler, this));

    mBlinking    = true;
    mToggleCount = 0;
    const CHIP_ERROR err = ScheduleBlinkTimer();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, mBlinking = false);
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrIdentifyDelegate::StopBlink()
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    VerifyOrReturnError(mBlinking, CHIP_NO_ERROR);

    mBlinking = false;
    DeviceLayer::SystemLayer().CancelTimer(BlinkTimerHandler, this);
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrIdentifyDelegate::ToggleLed()
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    VerifyOrReturnError(mLedReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gpio_pin_toggle_dt(&sIdentifyLed) == 0, CHIP_ERROR_INTERNAL);
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZephyrIdentifyDelegate::ScheduleBlinkTimer()
{
    return DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(500), BlinkTimerHandler, this);
}

void ZephyrIdentifyDelegate::BlinkTimerHandler(System::Layer *, void * appState)
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    auto * delegate = static_cast<ZephyrIdentifyDelegate *>(appState);
    VerifyOrReturn(delegate->mBlinking);

    VerifyOrReturn(delegate->ToggleLed() == CHIP_NO_ERROR, LogErrorOnFailure(delegate->StopBlink()));
    
    if (++delegate->mToggleCount >= sTargetBlinkToogleCount)
    {
        LogErrorOnFailure(delegate->StopBlink());
        return;
    }

    VerifyOrDo(delegate->ScheduleBlinkTimer() == CHIP_NO_ERROR, LogErrorOnFailure(delegate->StopBlink()));

#else
    (void) appState;
#endif
}

} // namespace chip::app::AllDevices
