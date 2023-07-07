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
#include <app/ConcreteAttributePath.h>
#include <app/clusters/level-control/level-control.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include "dbus/DBusLightApp.h"

using namespace chip;
using namespace chip::app;

namespace example {

CHIP_ERROR DBusInterface::Init()
{
    // During the initialization we are going to connect glib signals, so we need to be
    // on the GLib Matter context. Otherwise, signals will be emitted on the glib default
    // main context.
    return chip::DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(InitOnGLibMatterContext, this);
}

void DBusInterface::SetOnOff(bool on)
{
    VerifyOrReturn(mIfaceOnOff != nullptr);
    InternalSetGuard guard(this);
    light_app_on_off_set_on_off(mIfaceOnOff, on);
}

void DBusInterface::SetLevel(uint8_t value)
{
    VerifyOrReturn(mIfaceLevelControl != nullptr);
    InternalSetGuard guard(this);
    light_app_level_control_set_current_level(mIfaceLevelControl, value);
}

void DBusInterface::SetHue(uint8_t value)
{
    VerifyOrReturn(mIfaceColorControl != nullptr);
    InternalSetGuard guard(this);
    light_app_color_control_set_current_hue(mIfaceColorControl, value);
}

void DBusInterface::SetSaturation(uint8_t value)
{
    VerifyOrReturn(mIfaceColorControl != nullptr);
    InternalSetGuard guard(this);
    light_app_color_control_set_current_saturation(mIfaceColorControl, value);
}

CHIP_ERROR DBusInterface::InitOnGLibMatterContext(DBusInterface * self)
{
    g_autoptr(GDBusConnection) bus = nullptr;
    g_autoptr(GError) error        = nullptr;
    EmberAfStatus status;

    if ((bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error)) == nullptr)
    {
        ChipLogError(NotSpecified, "Couldn't get D-Bus bus: %s\n", error->message);
        return CHIP_ERROR_NOT_CONNECTED;
    }

    self->mManager = g_dbus_object_manager_server_new("/");

    LightAppObjectSkeleton * object = light_app_object_skeleton_new("/app");
    g_dbus_object_manager_server_export(self->mManager, G_DBUS_OBJECT_SKELETON(object));

    self->mIfaceOnOff = light_app_on_off_skeleton_new();
    light_app_object_skeleton_set_on_off(object, self->mIfaceOnOff);

    self->mIfaceLevelControl = light_app_level_control_skeleton_new();
    light_app_object_skeleton_set_level_control(object, self->mIfaceLevelControl);

    self->mIfaceColorControl = light_app_color_control_skeleton_new();
    light_app_object_skeleton_set_color_control(object, self->mIfaceColorControl);

    bool isOn = false;
    if ((status = Clusters::OnOff::Attributes::OnOff::Get(self->mEndpointId, &isOn)) != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "Error getting current OnOff: %x", status);
    }
    else
    {
        light_app_on_off_set_on_off(self->mIfaceOnOff, isOn);
    }

    chip::app::DataModel::Nullable<uint8_t> currentLevel;
    if ((status = Clusters::LevelControl::Attributes::CurrentLevel::Get(self->mEndpointId, currentLevel)) !=
        EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "Error getting current Level: %x", status);
    }
    else
    {
        light_app_level_control_set_current_level(self->mIfaceLevelControl, currentLevel.ValueOr(0));
    }

    g_dbus_object_manager_server_set_connection(self->mManager, bus);
    g_object_unref(object);

    g_signal_connect(self->mIfaceOnOff, "notify::on-off", G_CALLBACK(OnOnOffChanged), self);
    g_signal_connect(self->mIfaceLevelControl, "notify::current-level", G_CALLBACK(OnCurrentLevelChanged), self);
    g_signal_connect(self->mIfaceColorControl, "notify::current-hue", G_CALLBACK(OnCurrentHueChanged), self);
    g_signal_connect(self->mIfaceColorControl, "notify::current-saturation", G_CALLBACK(OnCurrentSaturationChanged), self);

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
    if (!self->mNameAcquired)
    {
        ChipLogError(NotSpecified, "Couldn't acquire D-Bus name. Please check D-Bus configuration. Requested name: %s", name);
    }
}

gboolean DBusInterface::OnOnOffChanged(LightAppOnOff * onOff, GDBusMethodInvocation * invocation, DBusInterface * self)
{
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
    VerifyOrReturnValue(!self->mInternalSet, G_DBUS_METHOD_INVOCATION_HANDLED);

    Clusters::LevelControl::Commands::MoveToLevel::DecodableType data;
    data.level = light_app_level_control_get_current_level(levelControl);
    data.optionsMask.Set(Clusters::LevelControl::LevelControlOptions::kExecuteIfOff);
    data.optionsOverride.Set(Clusters::LevelControl::LevelControlOptions::kExecuteIfOff);

    chip::DeviceLayer::StackLock lock;
    LevelControlServer::MoveToLevel(self->mEndpointId, data);

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

gboolean DBusInterface::OnCurrentHueChanged(LightAppColorControl * colorControl, GDBusMethodInvocation * invocation,
                                            DBusInterface * self)
{
    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

gboolean DBusInterface::OnCurrentSaturationChanged(LightAppColorControl * colorControl, GDBusMethodInvocation * invocation,
                                                   DBusInterface * self)
{
    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

}; // namespace example
