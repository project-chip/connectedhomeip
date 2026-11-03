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

#include "devices/ZephyrOnOffLight.h"

#include <devices/Types.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#if DT_NODE_EXISTS(DT_ALIAS(led0))
#include <zephyr/drivers/gpio.h>
#define ALL_DEVICES_HAS_ONOFF_LED 1
namespace {
const struct gpio_dt_spec sOnOffLed = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
} // namespace
#else
#define ALL_DEVICES_HAS_ONOFF_LED 0
#endif

namespace chip::app::AllDevices {

ZephyrOnOffLight::ZephyrOnOffLight(const OnOffLoad::Context & context) :
    LoggingOnOffDelegate(), OnOffLoad(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kOnOffLight, 1), *this, *this,
                                      context.identifyDelegate, context)
{
#if ALL_DEVICES_HAS_ONOFF_LED
    VerifyOrReturn(gpio_is_ready_dt(&sOnOffLed), ChipLogError(DeviceLayer, "On/Off LED GPIO not ready"));
    VerifyOrReturn(gpio_pin_configure_dt(&sOnOffLed, GPIO_OUTPUT_INACTIVE) == 0,
                   ChipLogError(DeviceLayer, "On/Off LED GPIO configure failed"));
    mLedReady = true;
#endif
}

ZephyrOnOffLight::~ZephyrOnOffLight() = default;

void ZephyrOnOffLight::OnOffStartup(bool on)
{
    LoggingOnOffDelegate::OnOffStartup(on);
    Apply(on);
}

void ZephyrOnOffLight::OnOnOffChanged(bool on)
{
    LoggingOnOffDelegate::OnOnOffChanged(on);
    Apply(on);
}

void ZephyrOnOffLight::Apply(bool on)
{
#if ALL_DEVICES_HAS_ONOFF_LED
    SetLed(on);
#else
    (void) on;
#endif
}

void ZephyrOnOffLight::SetLed(bool on)
{
#if ALL_DEVICES_HAS_ONOFF_LED
    VerifyOrReturn(mLedReady);
    const int rv = gpio_pin_set_dt(&sOnOffLed, on ? 1 : 0);
    VerifyOrDo(rv == 0, ChipLogError(DeviceLayer, "On/Off LED GPIO set failed: %d", rv));
#else
    (void) on;
#endif
}

} // namespace chip::app::AllDevices
