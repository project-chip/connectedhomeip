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

#include <peripheral_io.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>

#include <LightingManager.h>
#include <TizenServiceAppMain.h>

#if ENABLE_DBUS_UI
#include "DBusInterface.h"
#endif

using namespace chip;
using namespace chip::app;

namespace {

#if ENABLE_DBUS_UI
example::DBusInterface sDBusInterface(chip::EndpointId(1));
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
DeviceLayer::NetworkCommissioning::TizenWiFiDriver sTizenWiFiDriver;
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(0, &sTizenWiFiDriver);
#endif

} // namespace

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ChipLogDetail(
        NotSpecified, "MatterPostAttributeChangeCallback: EndPoint=0x%x, Cluster=" ChipLogFormatMEI ", Attribute=" ChipLogFormatMEI,
        attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId), ChipLogValueMEI(attributePath.mAttributeId));
    switch (attributePath.mClusterId)
    {
    case Clusters::OnOff::Id:
        switch (attributePath.mAttributeId)
        {
        case Clusters::OnOff::Attributes::OnOff::Id:
            LightingMgr().InitiateAction(*value ? LightingManager::ON_ACTION : LightingManager::OFF_ACTION);
#if ENABLE_DBUS_UI
            sDBusInterface.SetOnOff(*value);
#endif
            break;
        default:
            ChipLogDetail(NotSpecified, "Not handled OnOff cluster attribute ID: " ChipLogFormatMEI,
                          ChipLogValueMEI(attributePath.mAttributeId));
        }
        break;
#if ENABLE_DBUS_UI
    case Clusters::Identify::Id:
        switch (attributePath.mAttributeId)
        {
        case Clusters::Identify::Attributes::IdentifyTime::Id:
            VerifyOrDie(size == sizeof(uint16_t));
            sDBusInterface.Identify(*reinterpret_cast<uint16_t *>(value));
            break;
        default:
            ChipLogDetail(NotSpecified, "Not handled Identify cluster attribute ID: " ChipLogFormatMEI,
                          ChipLogValueMEI(attributePath.mAttributeId));
        }
        break;
    case Clusters::LevelControl::Id:
        switch (attributePath.mAttributeId)
        {
        case Clusters::LevelControl::Attributes::CurrentLevel::Id:
            sDBusInterface.SetLevel(*value);
            break;
        default:
            ChipLogDetail(NotSpecified, "Not handled LevelControl cluster attribute ID: " ChipLogFormatMEI,
                          ChipLogValueMEI(attributePath.mAttributeId));
        }
        break;
    case Clusters::ColorControl::Id:
        switch (attributePath.mAttributeId)
        {
        case Clusters::ColorControl::Attributes::ColorMode::Id:
            sDBusInterface.SetColorMode(static_cast<Clusters::ColorControl::ColorMode>(*value));
            break;
        case Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id:
            VerifyOrDie(size == sizeof(uint16_t));
            sDBusInterface.SetColorTemperature(*reinterpret_cast<uint16_t *>(value));
            break;
        default:
            ChipLogDetail(NotSpecified, "Not handled ColorControl cluster attribute ID: " ChipLogFormatMEI,
                          ChipLogValueMEI(attributePath.mAttributeId));
        }
        break;
#endif // ENABLE_DBUS_UI
    default:
        ChipLogDetail(NotSpecified, "Not handled cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(attributePath.mClusterId));
    }
}

constexpr int blue_pin  = 18;
constexpr int green_pin = 19;
constexpr int red_pin   = 20;

static peripheral_gpio_h blue_gpio;
static peripheral_gpio_h green_gpio;
static peripheral_gpio_h red_gpio;

static int gpio_init()
{
    peripheral_gpio_open(blue_pin, &blue_gpio);
    peripheral_gpio_set_direction(blue_gpio, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);

    peripheral_gpio_open(green_pin, &green_gpio);
    peripheral_gpio_set_direction(green_gpio, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);

    peripheral_gpio_open(red_pin, &red_gpio);
    peripheral_gpio_set_direction(red_gpio, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
    return 0;
}

static void gpio_on_off(bool on)
{
    peripheral_gpio_write(green_gpio, on ? 1 : 0);
    peripheral_gpio_write(blue_gpio, on ? 1 : 0);
    peripheral_gpio_write(red_gpio, on ? 1 : 0);
}

void ApplicationInit()
{
#if ENABLE_DBUS_UI
    sDBusInterface.Init();
#endif
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

    LightingMgr().SetCallbacks([](LightingManager::Action_t action) { gpio_on_off(action == LightingManager::ON_ACTION); },
                               [](LightingManager::Action_t action) {});

    app.SetInitializedCb([]() { gpio_on_off(LightingMgr().IsTurnedOn()); });

    return app.RunMainLoop();
}
