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

#include <gio/gio.h>
#include <sys/types.h>

#include <lightapp-iface.h>

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

static GDBusObjectManagerServer * dbus_manager = NULL;
static gboolean dbus_name_acquired             = FALSE;

static void on_bus_acquired(G_GNUC_UNUSED GDBusConnection * conn, G_GNUC_UNUSED const char * name, G_GNUC_UNUSED void * userdata)
{
    dbus_name_acquired = true;
}

static void on_bus_lost(G_GNUC_UNUSED GDBusConnection * conn, const char * name, G_GNUC_UNUSED void * userdata)
{
    if (!dbus_name_acquired)
    {
        g_printerr("Couldn't acquire D-Bus name. Please check D-Bus configuration."
                   " Requested name: %s\n",
                   name);
    }
}

static gboolean on_turn_on(LightAppManager1 * lightAppManager, GDBusMethodInvocation * invocation, G_GNUC_UNUSED void * userdata)
{

    unsigned int available_rpm = 0;
    unsigned int available_tpk = 0;
    g_autoptr(GError) error    = NULL;

    g_print("Check for update result: RPM=%u TPK=%u\n", available_rpm, available_tpk);

    LightingMgr().InitiateAction(LightingManager::ON_ACTION);

    light_app_manager1_set_on(lightAppManager, TRUE);
    light_app_manager1_complete_turn_on(lightAppManager, invocation);
    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_turn_off(LightAppManager1 * lightAppManager, GDBusMethodInvocation * invocation, G_GNUC_UNUSED void * userdata)
{

    unsigned int available_rpm = 0;
    unsigned int available_tpk = 0;
    g_autoptr(GError) error    = NULL;

    g_print("Check for update result: RPM=%u TPK=%u\n", available_rpm, available_tpk);

    LightingMgr().InitiateAction(LightingManager::OFF_ACTION);

    light_app_manager1_set_on(lightAppManager, FALSE);
    light_app_manager1_complete_turn_off(lightAppManager, invocation);
    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

CHIP_ERROR dbusInit(gpointer)
{
    pid_t pid = gettid();
    ChipLogProgress(DeviceLayer, "dbusInit: pid %d", pid);

    g_autoptr(GDBusConnection) bus = NULL;
    g_autoptr(GError) error        = NULL;
    ChipLogProgress(NotSpecified, "XXX dbusInit");

    if ((bus = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error)) == NULL)
    {
        ChipLogError(NotSpecified, "Couldn't get D-Bus bus: %s\n", error->message);
        return CHIP_ERROR_NOT_CONNECTED;
    }
    dbus_manager = g_dbus_object_manager_server_new("/");

    LightAppObjectSkeleton * object = light_app_object_skeleton_new("/lightapp");

    LightAppManager1 * lightAppManager = light_app_manager1_skeleton_new();
    g_signal_connect(lightAppManager, "handle-turn-on", G_CALLBACK(on_turn_on), NULL);
    g_signal_connect(lightAppManager, "handle-turn-off", G_CALLBACK(on_turn_off), NULL);
    light_app_object_skeleton_set_manager1(object, lightAppManager);
    g_object_unref(lightAppManager);

    g_dbus_object_manager_server_export(dbus_manager, G_DBUS_OBJECT_SKELETON(object));
    g_dbus_object_manager_server_set_connection(dbus_manager, bus);
    g_object_unref(object);

    g_bus_own_name_on_connection(bus, "org.tizen.matter.example.lighting", G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired,
                                 on_bus_lost, NULL, NULL);
    ChipLogProgress(NotSpecified, "XXX dbusInit completed");

    return CHIP_NO_ERROR;
}

void initCallback()
{
    chip::DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(dbusInit, static_cast<void *>(nullptr));
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
    app.SetInitializedCb(initCallback);
    return app.RunMainLoop();
}
