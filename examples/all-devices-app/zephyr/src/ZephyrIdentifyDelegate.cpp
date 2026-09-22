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

#include "ZephyrIdentifyDelegate.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#if ALL_DEVICES_HAS_IDENTIFY_LED
#include <zephyr/drivers/gpio.h>
#endif

namespace chip::app::AllDevices {
namespace {

#if ALL_DEVICES_HAS_IDENTIFY_LED
const struct gpio_dt_spec sIdentifyLed = GPIO_DT_SPEC_GET(ALL_DEVICES_IDENTIFY_LED_NODE, gpios);
#endif

} // namespace

ZephyrIdentifyDelegate::ZephyrIdentifyDelegate()
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    VerifyOrReturn(gpio_is_ready_dt(&sIdentifyLed), ChipLogError(DeviceLayer, "Identify LED GPIO not ready"));
    VerifyOrReturn(gpio_pin_configure_dt(&sIdentifyLed, GPIO_OUTPUT_INACTIVE) == 0,
                   ChipLogError(DeviceLayer, "Identify LED GPIO configure failed"));
    mLedReady = true;
#endif
}

ZephyrIdentifyDelegate::~ZephyrIdentifyDelegate()
{
    StopBlink();
}

void ZephyrIdentifyDelegate::SetNormalState(bool on)
{
    mNormalState = on;
    if (!mBlinking)
    {
        SetLed(on);
    }
}

void ZephyrIdentifyDelegate::SetLed(bool on)
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    VerifyOrReturn(mLedReady);
    const int rv = gpio_pin_set_dt(&sIdentifyLed, on ? 1 : 0);
    VerifyOrDo(rv == 0, ChipLogError(DeviceLayer, "Identify LED GPIO set failed: %d", rv));
#else
    (void) on;
#endif
}

void ZephyrIdentifyDelegate::OnIdentifyStart(Clusters::IdentifyCluster & /*cluster*/)
{
    ChipLogProgress(DeviceLayer, "ZephyrIdentifyDelegate: Identify START");
    StartBlink();
}

void ZephyrIdentifyDelegate::OnIdentifyStop(Clusters::IdentifyCluster & /*cluster*/)
{
    ChipLogProgress(DeviceLayer, "ZephyrIdentifyDelegate: Identify STOP");
    StopBlink();
}

void ZephyrIdentifyDelegate::OnTriggerEffect(Clusters::IdentifyCluster & /*cluster*/)
{
    ChipLogProgress(DeviceLayer, "ZephyrIdentifyDelegate: Identify TriggerEffect");
    StartBlink();
}

void ZephyrIdentifyDelegate::StartBlink()
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    if (mBlinking)
    {
        return;
    }

    mBlinking   = true;
    mBlinkPhase = true;
    SetLed(mBlinkPhase);
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(500), BlinkTimerHandler, this);
#endif
}

void ZephyrIdentifyDelegate::StopBlink()
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    if (!mBlinking)
    {
        return;
    }

    mBlinking = false;
    (void) DeviceLayer::SystemLayer().CancelTimer(BlinkTimerHandler, this);
    SetLed(mNormalState);
#endif
}

void ZephyrIdentifyDelegate::BlinkTimerHandler(System::Layer *, void * appState)
{
#if ALL_DEVICES_HAS_IDENTIFY_LED
    auto * delegate = static_cast<ZephyrIdentifyDelegate *>(appState);
    if (!delegate->mBlinking)
    {
        return;
    }

    delegate->mBlinkPhase = !delegate->mBlinkPhase;
    delegate->SetLed(delegate->mBlinkPhase);
    (void) DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(500), BlinkTimerHandler, delegate);
#else
    (void) appState;
#endif
}

} // namespace chip::app::AllDevices
