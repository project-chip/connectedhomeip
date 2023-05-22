/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>

#include <LightingManager.h>
#include <TizenServiceAppMain.h>

#include <peripheral_io.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace {
DeviceLayer::NetworkCommissioning::TizenWiFiDriver sTizenWiFiDriver;
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0, &sTizenWiFiDriver);
} // namespace
#endif

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if (attributePath.mClusterId == OnOff::Id && attributePath.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
    }
}

constexpr int blue_pin  = 18;
constexpr int green_pin = 19;
constexpr int red_pin   = 20;

static peripheral_gpio_h blue_gpio;
static peripheral_gpio_h green_gpio;
static peripheral_gpio_h red_gpio;

void gpio_turn_on();

int gpio_init()
{
    peripheral_gpio_open(blue_pin, &blue_gpio);
    peripheral_gpio_set_direction(blue_gpio, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);

    peripheral_gpio_open(green_pin, &green_gpio);
    peripheral_gpio_set_direction(green_gpio, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);

    peripheral_gpio_open(red_pin, &red_gpio);
    peripheral_gpio_set_direction(red_gpio, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
    gpio_turn_on();
    return 0;
}

void gpio_turn_on()
{
    peripheral_gpio_write(green_gpio, 1);
    peripheral_gpio_write(blue_gpio, 1);
    peripheral_gpio_write(red_gpio, 1);
}

void gpio_turn_off()
{
    peripheral_gpio_write(green_gpio, 0);
    peripheral_gpio_write(blue_gpio, 0);
    peripheral_gpio_write(red_gpio, 0);
}

void ApplicationInit()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    sWiFiNetworkCommissioningInstance.Init();
#endif
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    TizenServiceAppMain app;
    VerifyOrDie(app.Init(argc, argv) == 0);

    VerifyOrDie(gpio_init() == 0);
    VerifyOrDie(LightingMgr().Init() == CHIP_NO_ERROR);
    LightingMgr().SetCallbacks(
        [](LightingManager::Action_t action) {
            if (action == LightingManager::ON_ACTION)
            {
                gpio_turn_on();
            }
            else if (action == LightingManager::OFF_ACTION)
            {
                gpio_turn_off();
            }
        },
        [](LightingManager::Action_t action) {

        });

    return app.RunMainLoop();
}
