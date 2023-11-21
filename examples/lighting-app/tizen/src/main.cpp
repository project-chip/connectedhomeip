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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/color-control-server/color-control-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>

#include <LedManager.h>
#include <LightingManager.h>
#include <TizenServiceAppMain.h>

#if ENABLE_DBUS_UI
#include "DBusInterface.h"
#endif

using namespace chip;
using namespace chip::app;

namespace {

static constexpr chip::EndpointId chipEndpoint(1);

#if ENABLE_DBUS_UI
example::DBusInterface sDBusInterface(chipEndpoint);
#endif

static example::LedManager ledManager(chipEndpoint);

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
            ledManager.SetOnOff(*value);
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
            sDBusInterface.SetCurrentLevel(*value);
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

void emberAfColorControlClusterInitCallback(EndpointId endpoint)
{
    // Set the color mode to color temperature.
    Clusters::ColorControl::Attributes::ColorMode::Set(endpoint, ColorControlServer::EnhancedColorMode::kColorTemperature);
    // Preserve the state of the color temperature attribute across reboots.
    Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::SetNull(endpoint);
}

void ApplicationInit()
{
#if ENABLE_DBUS_UI
    sDBusInterface.Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    sWiFiNetworkCommissioningInstance.Init();
#endif
    ledManager.Init();
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    TizenServiceAppMain app;
    VerifyOrDie(app.Init(argc, argv) == 0);
    VerifyOrDie(LightingMgr().Init() == CHIP_NO_ERROR);

    return app.RunMainLoop();
}
