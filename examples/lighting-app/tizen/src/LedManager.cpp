/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <LedManager.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace example {

using namespace chip;
using namespace chip::app;

LedManager::LedManager(chip::EndpointId endpointId) : mEndpointId(endpointId){};

CHIP_ERROR LedManager::Init()
{
    InitOnOff();
    int ret = 0;
    for (auto i = 0; i < number_of_pins; ++i)
    {
        ret = peripheral_gpio_open(pins[i], &gpio[i]);
        VerifyOrReturnError(ret == PERIPHERAL_ERROR_NONE, CHIP_ERROR_INTERNAL,
                            ChipLogError(DeviceLayer, "Error openning gpio, pin number: %d", pins[i]));
        ret = peripheral_gpio_set_direction(gpio[i], PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
        VerifyOrReturnError(ret == PERIPHERAL_ERROR_NONE, CHIP_ERROR_INTERNAL,
                            ChipLogError(DeviceLayer, "Error setting direction of gpio, pin number: %d", pins[i]));
    }
    return CHIP_NO_ERROR;
}

void LedManager::SetOnOff(const bool isOn)
{
    for (auto i = 0; i < number_of_pins; ++i)
    {
        int ret = 0;
        ret     = peripheral_gpio_write(gpio[i], isOn ? 1 : 0);
        VerifyOrReturn(ret == PERIPHERAL_ERROR_NONE,
                       ChipLogError(DeviceLayer, "Error setting value to gpio, pin number: %d", pins[i]));
    }
}

void LedManager::InitOnOff()
{
    bool isOn   = false;
    auto status = Clusters::OnOff::Attributes::OnOff::Get(mEndpointId, &isOn);
    VerifyOrReturn(status == Protocols::InteractionModel::Status::Success,
                   ChipLogError(NotSpecified, "Error getting OnOff: 0x%x", to_underlying(status)));
    SetOnOff(isOn);
}

} // namespace example
