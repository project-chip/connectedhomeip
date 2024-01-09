/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "matter.h"
#include <pthread.h>

#include <thread>
#include <unistd.h>

#include "Options.h"
#include "app/server/Server.h"

// including ninja generated version file
#include "chip_version.hpp"


// Application library
#include "attribute_translator.hpp"
#include "bridged_device_basic_info_attribute_translator.hpp"
#include "cluster_emulator.hpp"
#include "command_translator.hpp"
#include "group_command_translator.hpp"
#include "matter_bridge_cli.hpp"
#include "matter_bridge_config.h"
#include "matter_bridge_config_fixt.h"
#include "matter_bridge_qrcode_publisher.hpp"
#include "matter_device_translator.hpp"
#include "matter_node_state_monitor.hpp"

extern "C" {
// Unify library
#include "sl_log.h"
#include "sl_status.h"
#include "uic_init.h"
#include "uic_main.h"
#include "uic_main_loop.h"
}

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

using namespace unify::matter_bridge;

#define CMAKE_PROJECT_VERSION "1.1"

constexpr const char * LOG_TAG = "unify_matter_bridge";

static bool matter_running;
static std::mutex unify_mutex;

static void call_unify_event_queue(intptr_t)
{
    unify_mutex.lock();
    bool shutdown = !uic_main_loop_run();
    unify_mutex.unlock();
    if (shutdown)
    {
        matter_running = false;
        Server::GetInstance().GenerateShutDownEvent();
        PlatformMgr().ScheduleWork([](intptr_t) { PlatformMgr().StopEventLoopTask(); });
    }
}

std::thread run_unify()
{
    std::thread unify_thread([]() {
        while (matter_running)
        {
            uic_main_wait_for_file_descriptors();
            // It seems that ScheduleWork is not thread safe, which is why its protected
            // by a mutex
            unify_mutex.lock();
            PlatformMgr().ScheduleWork(call_unify_event_queue);
            unify_mutex.unlock();
        }
    });
    return unify_thread;
}

int main(int argc, char * argv[])
{
    if (matter_bridge_config_init())
    {
        return -1;
    }

    // Setup fixtures
    uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = { { matter_bridge_config_fixt_setup, "Matter Bridge config fixture" },
                                                          { matter_bridge_cli_init, "Matter Bridge Command Line Interface" },
                                                          { NULL, "Terminator" } };

    if (SL_STATUS_OK != uic_init(uic_fixt_setup_steps_list, argc, argv, CMAKE_PROJECT_VERSION))
    {
        return -1;
    }

    sl_log_info(LOG_TAG, "Current Matter Version:  %s \n", CHIP_VERSION);
    sl_log_info(LOG_TAG, "Current Matter Version SHA: %s \n", CHIP_SHA);

    const char * __argv__[] = { "matter_bridge", nullptr };
    int __argc__            = sizeof(__argv__) / sizeof(const char *) - 1;

    auto & opt = LinuxDeviceOptions::GetInstance();
    auto cfg   = matter_bridge_get_config();

    opt.payload.commissioningFlow = CommissioningFlow::kStandard;
    opt.payload.rendezvousInformation.Emplace().ClearAll();
    opt.payload.rendezvousInformation.Emplace().Set(RendezvousInformationFlag::kOnNetwork);
    opt.mWiFi                = false;
    opt.mThread              = false;
    opt.KVS                  = cfg->kvs_path;
    opt.payload.productID    = cfg->product_id;
    opt.payload.vendorID     = cfg->vendor_id;
    opt.payload.setUpPINCode = cfg->pin & 0xffffff;
    opt.discriminator        = chip::Optional<uint16_t>(cfg->discriminator & 0xfff);

    if (CHIP_NO_ERROR != InterfaceId::InterfaceNameToId(cfg->interface, opt.interfaceId))
    {
        sl_log_error(LOG_TAG, "Unable to select interface %s", cfg->interface);
        return -1;
    }
    VerifyOrDie(ChipLinuxAppInit(__argc__, const_cast<char **>(__argv__)) == 0);

    sl_log_info(LOG_TAG, "Starting Matter Bridge with strict device mapping: %s", cfg->strict_device_mapping ? "enabled" : "disabled");
    device_translator matter_device_translator = device_translator(cfg->strict_device_mapping);
    UnifyEmberInterface ember_interface;
    ClusterEmulator emulator;
    matter_node_state_monitor node_state_monitor(matter_device_translator, emulator, ember_interface);
    UnifyMqtt unify_mqtt_handler;
    group_translator m_group_translator(matter_data_storage::instance());
    set_mapping_display_instance(node_state_monitor, m_group_translator);

#define INIT_ZAP_TRANSLATOR(a)                                                                                                     \
    a##AttributeAccess a##_attribute_instance(node_state_monitor, unify_mqtt_handler, matter_device_translator);                   \
    a##ClusterCommandHandler a##_command_instance(node_state_monitor, unify_mqtt_handler, m_group_translator);

    // Initializing Group cluster command handler
    GroupClusterCommandHandler group_handler(node_state_monitor, unify_mqtt_handler, m_group_translator);

    // Initializing Bridged Device Basic Info attributes update handler
    BridgedDeviceBasicInfoAttributeAccess bridge_device_basic_handler(node_state_monitor, unify_mqtt_handler,
                                                                      matter_device_translator);

    INIT_ZAP_TRANSLATOR(OnOff);
    INIT_ZAP_TRANSLATOR(Identify);
    INIT_ZAP_TRANSLATOR(LevelControl);
    INIT_ZAP_TRANSLATOR(ColorControl);
    INIT_ZAP_TRANSLATOR(Thermostat);

    // Sensor clusters
    INIT_ZAP_TRANSLATOR(IlluminanceMeasurement);
    INIT_ZAP_TRANSLATOR(OccupancySensing);
    INIT_ZAP_TRANSLATOR(TemperatureMeasurement);
    INIT_ZAP_TRANSLATOR(PressureMeasurement);
    INIT_ZAP_TRANSLATOR(FlowMeasurement);
    INIT_ZAP_TRANSLATOR(RelativeHumidityMeasurement);

    QRCodePublisher qr_code_publisher(unify_mqtt_handler);
    set_qr_code_publisher(qr_code_publisher);

    matter_running = true;
    auto handle    = run_unify();
    ChipLinuxAppMainLoop();
    matter_running = false;
    handle.join();
    return 0;
}
