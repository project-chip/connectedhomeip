/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "matter.h"
#include <pthread.h>

#include <thread>
#include <unistd.h>

// Application library
#include "bridged_device_basic_info_attribute_translator.hpp"
#include "group_command_translator.hpp"
#include "identify_attribute_translator.hpp"
#include "identify_command_translator.hpp"
#include "level_attribute_translator.hpp"
#include "level_command_translator.hpp"
#include "matter_bridge_config.h"
#include "matter_bridge_config_fixt.h"
#include "matter_device_translator.hpp"
#include "matter_node_state_monitor.hpp"
#include "on_off_attribute_translator.hpp"
#include "on_off_command_translator.h"

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

void init_ember_endpoints()
{
    PlatformMgr().ScheduleWork([](intptr_t) {
        // Disable last fixed endpoint, which is used as a placeholder for all of the
        // supported clusters so that ZAP will generated the requisite code.
        emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
    });
}

static void stop_the_loop(intptr_t)
{
    PlatformMgr().StopEventLoopTask();
}

std::thread run_unify()
{
    std::thread unify_thread([]() {
        while (matter_running)
        {
            PlatformMgr().LockChipStack();
            bool shutdown = !uic_main_loop_run();
            PlatformMgr().UnlockChipStack();
            uic_main_wait_for_file_descriptors();

            if (shutdown)
            {
                PlatformMgr().ScheduleWork(stop_the_loop);
                break;
            }
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
                                                          { NULL, "Terminator" } };

    uic_init(uic_fixt_setup_steps_list, argc, argv, CMAKE_PROJECT_VERSION);

    const char * __argv__[] = { "matter_bridge", nullptr };
    int __argc__            = sizeof(__argv__) / sizeof(const char *) - 1;

    auto & opt = LinuxDeviceOptions::GetInstance();
    auto cfg   = matter_bridge_get_config();

    opt.payload.commissioningFlow = CommissioningFlow::kStandard;
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
    matter_node_state_monitor node_state_monitor(matter_device_translator);

    // Initializing OnOff command handler
    GroupClusterCommandHandler group_handler(node_state_monitor);
    OnOffClusterCommandHandler on_cmd_handler(node_state_monitor);
    OnOffAttributeAccess on_off_atttr_handler(node_state_monitor);
    // Initializing Identify Cluster Commands handler
    IdentifyClusterCommandHandler identify_cluster_commands_handler(node_state_monitor);
    IdentifyAttributeAccess identify_attribute_handler(node_state_monitor);
    // Initializing Bridged Device Basic Info attributes update handler
    BridgedDeviceBasicInfoAttributeAccess bridge_device_basic_handler(node_state_monitor);
    // Initializing Level Cluster handler
    // LevelClusterCommandHandler level_cluster_commands_handler(node_state_monitor);
    // LevelAttributeAccess level_attribute_handler(node_state_monitor);

    matter_running = true;
    auto handle    = run_unify();
    init_ember_endpoints();
    ChipLinuxAppMainLoop();
    matter_running = false;
    handle.join();
    return 0;
}
