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

// Application library
#include "bridged_device_basic_info_attribute_translator.hpp"
#include "command_translator.hpp"
#include "feature_map_attr_override.hpp"
#include "group_command_translator.hpp"
#include "matter_bridge_cli.hpp"
#include "matter_bridge_config.h"
#include "matter_bridge_config_fixt.h"
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

#define CMAKE_PROJECT_VERSION "1.0"

constexpr const char * LOG_TAG = "unify_matter_bridge";

static bool matter_running;
static std::mutex unify_mutex;


static void call_unify_event_queue(intptr_t)
{
    unify_mutex.lock();
    bool shutdown = !uic_main_loop_run();
    unify_mutex.unlock();
    if( shutdown ) {
        matter_running = false;
        Server::GetInstance().DispatchShutDownAndStopEventLoop();
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

    uic_init(uic_fixt_setup_steps_list, argc, argv, CMAKE_PROJECT_VERSION);


    const char * __argv__[] = { "matter_bridge", nullptr };
    int __argc__            = sizeof(__argv__) / sizeof(const char *) - 1;

    auto & opt = LinuxDeviceOptions::GetInstance();
    auto cfg   = matter_bridge_get_config();

    opt.payload.commissioningFlow = CommissioningFlow::kStandard;
    opt.payload.rendezvousInformation.Emplace().ClearAll();
    opt.payload.rendezvousInformation.Emplace().Set(RendezvousInformationFlag::kOnNetwork);
    opt.mWiFi   = false;
    opt.mThread = false;

    opt.KVS               = cfg->kvs_path;
    opt.payload.productID = cfg->product_id;
    opt.payload.vendorID  = cfg->vendor_id;

    if (CHIP_NO_ERROR != InterfaceId::InterfaceNameToId(cfg->interface, opt.interfaceId))
    {
        sl_log_error(LOG_TAG, "Unable to select interface %s", cfg->interface);
        return -1;
    }
    VerifyOrDie(ChipLinuxAppInit(__argc__, const_cast<char **>(__argv__)) == 0);

    device_translator matter_device_translator;
    UnifyEmberInterface ember_interface;
    matter_node_state_monitor node_state_monitor(matter_device_translator, ember_interface);
    UnifyMqtt unify_mqtt_handler;

    // Initializing Group cluster command handler
    group_translator m_group_translator(matter_data_storage::instance());
    GroupClusterCommandHandler group_handler(node_state_monitor, unify_mqtt_handler, m_group_translator);

    // Initializing Bridged Device Basic Info attributes update handler
    BridgedDeviceBasicInfoAttributeAccess bridge_device_basic_handler(node_state_monitor, unify_mqtt_handler);

    // Initializing OnOff command handler
    OnOffClusterCommandHandler on_cmd_handler(node_state_monitor, unify_mqtt_handler, m_group_translator);
    OnOffAttributeAccessOverride on_off_attribute_handler(node_state_monitor, unify_mqtt_handler);

    // Initializing Identify Cluster Commands handler
    IdentifyClusterCommandHandler identify_cluster_commands_handler(node_state_monitor, unify_mqtt_handler, m_group_translator);
    IdentifyAttributeAccess identify_attribute_handler(node_state_monitor, unify_mqtt_handler);

    // Initializing Level Cluster handler
    LevelControlClusterCommandHandler level_cluster_commands_handler(node_state_monitor, unify_mqtt_handler, m_group_translator);
    LevelControlAttributeAccessOverride level_attribute_handler(node_state_monitor, unify_mqtt_handler);

    // Initializing color controller cluster command handler
    ColorControlClusterCommandHandler color_control_commands_handler(node_state_monitor, unify_mqtt_handler, m_group_translator);
    ColorControllerAttributeAccessOverride color_control_attribute_handler(node_state_monitor, unify_mqtt_handler);

    // Initializing OccupancySensing command handler
    OccupancySensingClusterCommandHandler occupancy_sensing_command_handler(node_state_monitor, unify_mqtt_handler,
                                                                            m_group_translator);
    OccupancySensingAttributeAccess occupancy_sensing_attribute_access(node_state_monitor, unify_mqtt_handler);

    // Initializing Temperature Measurement command handler
    TemperatureMeasurementClusterCommandHandler temperature_measurement_command_handler(node_state_monitor, unify_mqtt_handler,
                                                                                        m_group_translator);
    TemperatureMeasurementAttributeAccess temperature_measurement_attribute_access(node_state_monitor, unify_mqtt_handler);

    matter_running = true;
    auto handle    = run_unify();
    ChipLinuxAppMainLoop();
    matter_running = false;
    handle.join();
    return 0;
}
