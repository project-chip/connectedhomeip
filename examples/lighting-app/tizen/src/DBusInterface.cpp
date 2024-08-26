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

#include "DBusInterface.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandlerImpl.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/color-control-server/color-control-server.h>
#include <app/clusters/level-control/level-control.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include "dbus/DBusLightApp.h"

using namespace chip;
using namespace chip::app;

namespace example {

// Dummy class to satisfy the CommandHandlerImpl::Callback interface.
class CommandHandlerImplCallback : public CommandHandlerImpl::Callback
{
public:
    using Status = Protocols::InteractionModel::Status;
    void OnDone(CommandHandlerImpl & apCommandObj) {}
    void DispatchCommand(CommandHandlerImpl & apCommandObj, const ConcreteCommandPath & aCommandPath, TLV::TLVReader & apPayload) {}
    Status CommandExists(const ConcreteCommandPath & aCommandPath) { return Status::Success; }
};

DBusInterface::DBusInterface(chip::EndpointId endpointId) : mEndpointId(endpointId)
{
    mManager           = g_dbus_object_manager_server_new("/");
    mIfaceIdentify     = light_app_identify_skeleton_new();
    mIfaceOnOff        = light_app_on_off_skeleton_new();
    mIfaceLevelControl = light_app_level_control_skeleton_new();
    mIfaceColorControl = light_app_color_control_skeleton_new();
}

DBusInterface::~DBusInterface()
{
    g_object_unref(mIfaceIdentify);
    g_object_unref(mIfaceOnOff);
    g_object_unref(mIfaceLevelControl);
    g_object_unref(mIfaceColorControl);
    g_object_unref(mManager);
}

CHIP_ERROR DBusInterface::Init()
{
    // During the initialization we are going to connect glib signals, so we need to be
    // on the GLib Matter context. Otherwise, signals will be emitted on the glib default
    // main context.
    return chip::DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(InitOnGLibMatterContext, this);
}

void DBusInterface::Identify(uint16_t time)
{
    light_app_identify_emit_identify(mIfaceIdentify, time);
}

void DBusInterface::SetOnOff(bool on)
{
    InternalSetGuard guard(this);
    if (light_app_on_off_get_on_off(mIfaceOnOff) != on)
        light_app_on_off_set_on_off(mIfaceOnOff, on);
}

void DBusInterface::SetCurrentLevel(uint8_t value)
{
    InternalSetGuard guard(this);
    if (light_app_level_control_get_current_level(mIfaceLevelControl) != value)
        light_app_level_control_set_current_level(mIfaceLevelControl, value);
}

void DBusInterface::SetColorMode(chip::app::Clusters::ColorControl::ColorMode colorMode)
{
    InternalSetGuard guard(this);
    if (light_app_color_control_get_color_mode(mIfaceColorControl) != chip::to_underlying(colorMode))
        light_app_color_control_set_color_mode(mIfaceColorControl, chip::to_underlying(colorMode));
}

void DBusInterface::SetColorTemperature(uint16_t value)
{
    InternalSetGuard guard(this);
    if (light_app_color_control_get_color_temperature_mireds(mIfaceColorControl) != value)
        light_app_color_control_set_color_temperature_mireds(mIfaceColorControl, value);
}

CHIP_ERROR DBusInterface::InitOnGLibMatterContext(DBusInterface * self)
{
    g_autoptr(GDBusConnection) bus = nullptr;
    g_autoptr(GError) error        = nullptr;

    if ((bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error)) == nullptr)
    {
        ChipLogError(NotSpecified, "Couldn't get D-Bus bus: %s", error->message);
        return CHIP_ERROR_NOT_CONNECTED;
    }

    LightAppObjectSkeleton * object = light_app_object_skeleton_new("/app");
    g_dbus_object_manager_server_export(self->mManager, G_DBUS_OBJECT_SKELETON(object));

    light_app_object_skeleton_set_identify(object, self->mIfaceIdentify);
    light_app_object_skeleton_set_on_off(object, self->mIfaceOnOff);
    light_app_object_skeleton_set_level_control(object, self->mIfaceLevelControl);
    light_app_object_skeleton_set_color_control(object, self->mIfaceColorControl);

    self->InitOnOff();
    self->InitColor();

    g_dbus_object_manager_server_set_connection(self->mManager, bus);
    g_object_unref(object);

    g_signal_connect(self->mIfaceOnOff, "notify::on-off", G_CALLBACK(OnOnOffChanged), self);
    g_signal_connect(self->mIfaceLevelControl, "notify::current-level", G_CALLBACK(OnCurrentLevelChanged), self);
    g_signal_connect(self->mIfaceColorControl, "notify::color-temperature-mireds", G_CALLBACK(OnColorTemperatureChanged), self);

    g_bus_own_name_on_connection(bus, "org.tizen.matter.example.lighting", G_BUS_NAME_OWNER_FLAGS_NONE,
                                 reinterpret_cast<GBusAcquiredCallback>(OnBusAcquired),
                                 reinterpret_cast<GBusNameLostCallback>(OnBusLost), self, nullptr);

    return CHIP_NO_ERROR;
}

void DBusInterface::OnBusAcquired(GDBusConnection *, const char *, DBusInterface * self)
{
    self->mNameAcquired = true;
}

void DBusInterface::OnBusLost(GDBusConnection *, const char * name, DBusInterface * self)
{
    VerifyOrReturn(self->mNameAcquired, /* connection was lost after name was acquired, so it's not an error */);
    ChipLogError(NotSpecified, "Couldn't acquire D-Bus name. Please check D-Bus configuration. Requested name: %s", name);
}

gboolean DBusInterface::OnOnOffChanged(LightAppOnOff * onOff, GDBusMethodInvocation * invocation, DBusInterface * self)
{
    // Do not handle on-change event if it was triggered by internal set
    VerifyOrReturnValue(!self->mInternalSet, G_DBUS_METHOD_INVOCATION_HANDLED);

    chip::DeviceLayer::StackLock lock;
    OnOffServer::Instance().setOnOffValue(self->mEndpointId,
                                          light_app_on_off_get_on_off(onOff) ? Clusters::OnOff::Commands::On::Id
                                                                             : Clusters::OnOff::Commands::Off::Id,
                                          false /* initiatedByLevelChange */);
    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

gboolean DBusInterface::OnCurrentLevelChanged(LightAppLevelControl * levelControl, GDBusMethodInvocation * invocation,
                                              DBusInterface * self)
{
    // Do not handle on-change event if it was triggered by internal set
    VerifyOrReturnValue(!self->mInternalSet, G_DBUS_METHOD_INVOCATION_HANDLED);

    Clusters::LevelControl::Commands::MoveToLevel::DecodableType data;
    data.level = light_app_level_control_get_current_level(levelControl);
    data.optionsMask.Set(Clusters::LevelControl::OptionsBitmap::kExecuteIfOff);
    data.optionsOverride.Set(Clusters::LevelControl::OptionsBitmap::kExecuteIfOff);

    chip::DeviceLayer::StackLock lock;
    LevelControlServer::MoveToLevel(self->mEndpointId, data);

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

gboolean DBusInterface::OnColorTemperatureChanged(LightAppColorControl * colorControl, GDBusMethodInvocation * invocation,
                                                  DBusInterface * self)
{
    // Do not handle on-change event if it was triggered by internal set
    VerifyOrReturnValue(!self->mInternalSet, G_DBUS_METHOD_INVOCATION_HANDLED);

    // TODO: creating such a complex object seems odd here
    //       as handler seems not used to send back any response back anywhere.
    CommandHandlerImplCallback callback;
    CommandHandlerImpl handler(&callback);

    ConcreteCommandPath path{ self->mEndpointId, Clusters::ColorControl::Id, 0 };

    Clusters::ColorControl::Commands::MoveToColorTemperature::DecodableType data;
    data.colorTemperatureMireds = light_app_color_control_get_color_temperature_mireds(colorControl);

    chip::DeviceLayer::StackLock lock;
    ColorControlServer::Instance().moveToColorTempCommand(&handler, path, data);

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

void DBusInterface::InitOnOff()
{
    bool isOn   = false;
    auto status = Clusters::OnOff::Attributes::OnOff::Get(mEndpointId, &isOn);
    VerifyOrReturn(status == Protocols::InteractionModel::Status::Success,
                   ChipLogError(NotSpecified, "Error getting OnOff: 0x%x", to_underlying(status)));
    light_app_on_off_set_on_off(mIfaceOnOff, isOn);
}

void DBusInterface::InitColor()
{
    {
        auto value  = Clusters::ColorControl::ColorModeEnum::kCurrentHueAndCurrentSaturation;
        auto status = Clusters::ColorControl::Attributes::ColorMode::Get(mEndpointId, &value);
        VerifyOrReturn(status == Protocols::InteractionModel::Status::Success,
                       ChipLogError(NotSpecified, "Error getting ColorMode: 0x%x", to_underlying(status)));
        light_app_color_control_set_color_mode(mIfaceColorControl, to_underlying(value));
    }
    {
        uint16_t value = 0;
        auto status    = Clusters::ColorControl::Attributes::ColorTemperatureMireds::Get(mEndpointId, &value);
        VerifyOrReturn(status == Protocols::InteractionModel::Status::Success,
                       ChipLogError(NotSpecified, "Error getting ColorTemperatureMireds: 0x%x", to_underlying(status)));
        light_app_color_control_set_color_temperature_mireds(mIfaceColorControl, value);
    }
}

}; // namespace example
