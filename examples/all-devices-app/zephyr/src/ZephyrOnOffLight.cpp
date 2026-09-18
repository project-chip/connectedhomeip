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

#include "ZephyrOnOffLight.h"

#include <devices/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#if ALL_DEVICES_HAS_ONOFF_LED
#include <zephyr/drivers/gpio.h>
#endif

namespace chip::app::AllDevices {
namespace {

constexpr System::Clock::Milliseconds32 kBlinkInterval = System::Clock::Milliseconds32(500);

#if ALL_DEVICES_HAS_ONOFF_LED
const struct gpio_dt_spec sOnOffLed = GPIO_DT_SPEC_GET(ALL_DEVICES_ONOFF_LED_NODE, gpios);
#endif

} // namespace

ZephyrOnOffLight::ZephyrOnOffLight(const Context & context) :
    LoggingOnOffLoad(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOnOffLight, 1), context, nullptr, nullptr, this)
{
#if ALL_DEVICES_HAS_ONOFF_LED
    VerifyOrReturn(gpio_is_ready_dt(&sOnOffLed), ChipLogError(DeviceLayer, "On/Off LED GPIO not ready"));
    VerifyOrReturn(gpio_pin_configure_dt(&sOnOffLed, GPIO_OUTPUT_INACTIVE) == 0,
                   ChipLogError(DeviceLayer, "On/Off LED GPIO configure failed"));
    mLedReady = true;
#endif
}

ZephyrOnOffLight::~ZephyrOnOffLight()
{
    StopBlink();
}

void ZephyrOnOffLight::OnOffStartup(bool on)
{
    LoggingOnOffLoad::OnOffStartup(on);
    Apply(on);
}

void ZephyrOnOffLight::OnOnOffChanged(bool on)
{
    LoggingOnOffLoad::OnOnOffChanged(on);
    Apply(on);
}

void ZephyrOnOffLight::OnIdentifyStart(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ZephyrOnOffLight: Identify START");
    StartBlink();
}

void ZephyrOnOffLight::OnIdentifyStop(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ZephyrOnOffLight: Identify STOP");
    StopBlink();
}

void ZephyrOnOffLight::OnTriggerEffect(Clusters::IdentifyCluster & cluster)
{
    ChipLogProgress(DeviceLayer, "ZephyrOnOffLight: Identify TriggerEffect");
    StartBlink();
}

void ZephyrOnOffLight::StartBlink()
{
    VerifyOrReturn(!mBlinking);
    mBlinking   = true;
    mBlinkPhase = true;
    SetLed(mBlinkPhase);
    DeviceLayer::SystemLayer().StartTimer(kBlinkInterval, BlinkTimerHandler, this);
}

void ZephyrOnOffLight::StopBlink()
{
    VerifyOrReturn(mBlinking);
    mBlinking = false;
    DeviceLayer::SystemLayer().CancelTimer(BlinkTimerHandler, this);
    SetLed(mOn);
}

void ZephyrOnOffLight::BlinkTimerHandler(System::Layer * layer, void * context)
{
    auto * self = static_cast<ZephyrOnOffLight *>(context);
    VerifyOrReturn(self->mBlinking);
    self->mBlinkPhase = !self->mBlinkPhase;
    self->SetLed(self->mBlinkPhase);
    layer->StartTimer(kBlinkInterval, BlinkTimerHandler, self);
}

void ZephyrOnOffLight::Apply(bool on)
{
    mOn = on;
    VerifyOrReturn(!mBlinking);
    SetLed(on);
}

void ZephyrOnOffLight::SetLed([[maybe_unused]] bool on)
{
#if ALL_DEVICES_HAS_ONOFF_LED
    VerifyOrReturn(mLedReady);
    const int rv = gpio_pin_set_dt(&sOnOffLed, on ? 1 : 0);
    VerifyOrDo(rv == 0, ChipLogError(DeviceLayer, "On/Off LED GPIO set failed: %d", rv));
#endif
}

} // namespace chip::app::AllDevices
