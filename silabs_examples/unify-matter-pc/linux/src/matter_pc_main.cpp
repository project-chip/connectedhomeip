/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

// Matter components
#include "app/server/Server.h"
#include <AppMain.h>
#include <filesystem>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

// MPC Components
#include "dotdot_mqtt_to_matter.hpp"
#include "mpc_application_monitoring.h"
#include "mpc_attribute_resolver_fixt.h"
#include "mpc_attribute_store.h"
#include "mpc_cli.hpp"
#include "mpc_config.h"
#include "mpc_config_fixt.h"
#include "mpc_datastore_fixt.h"
#include "mpc_nw_monitor.h"

// Unify components
extern "C" {
#include "attribute_resolver.h"
#include "attribute_store_fixt.h"
#include "datastore_fixt.h"
#include "dotdot_mqtt.h"
#include "sl_log.h"
#include "sl_status.h"
#include "uic_init.h"
#include "uic_main.h"
#include "uic_main_loop.h"
#include "unify_dotdot_attribute_store.h"
}

// Generic incldues
#include <stdlib.h>
#include <thread>

// Log tag
#define LOG_TAG "unify-matter-pc"

#define CMAKE_PROJECT_VERSION "0.9"

using namespace chip;
using namespace chip::app;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

static bool matter_running;
std::mutex unify_mutex;

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

void mpc_schedule_contiki() {
    call_unify_event_queue(0);
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

// List of init functions.
static uic_fixt_setup_step_t uic_fixt_setup_steps_list[] = { { &mpc_config_fixt_setup, "MPC Config" },
                                                             { &mpc_application_monitoring_init, "MPC ApplicationMonitoring" },
                                                             { &mpc_datastore_fixt_setup, "Datastore" },
                                                             { &attribute_store_init, "Attribute store" },
                                                             { &mpc_cli_init, "Matter Protocol Controller Command Line Interface" },
                                                             { &mpc_attribute_resolver_init, "MPC attribute resolver" },
                                                             /**
                                                              * Initializes the ZCL/DotDot specialization of the Attribute Store.
                                                              * MUST be initialized before uic_mqtt_dotdot_init
                                                              */
                                                             { &unify_dotdot_attribute_store_init, "Unify DotDot Attribute Store" },
                                                             { &uic_mqtt_dotdot_init, "DotDot MQTT" },
                                                             { &mpc_attribute_store_init, "MPC attribute store" },
                                                             { NULL, "Terminator" } };

// List of shutdown functions
// static uic_fixt_shutdown_step_t uic_fixt_shutdown_steps_list[]
//   = {{&attribute_store_teardown, "Attribute store"},
//     {&datastore_fixt_teardown, "Datastore"},
//     {NULL, "Terminator"}};

/**
 * @brief Main function for the Mater Protocol Controller (MPC)
 *
 * @param argc
 * @param argv
 * @return 0 on success (EXIT_SUCCESS), any other code in case of error
 */
int main(int argc, char ** argv)
{
    // Initialize the MPC Configuration, add new keys/default values as needed.
    if (mpc_config_init())
    {
        return EXIT_FAILURE;
    }

    // Initialize UIC to read configuration and start-up contiki unify processes
    if (SL_STATUS_OK != uic_init(uic_fixt_setup_steps_list, argc, argv, CMAKE_PROJECT_VERSION))
    {
        return -1;
    }

    // Makes sure our required state folders exists
    std::filesystem::create_directories(FATCONFDIR);
    std::filesystem::create_directories(SYSCONFDIR);
    std::filesystem::create_directories(LOCALSTATEDIR);

    mpc_on_off_cluster_mapper_init();

    // Set up configuration and args for Matter and matter application initialization
    const char * __argv__[] = { "matter_protocol_controller", nullptr };
    int __argc__            = sizeof(__argv__) / sizeof(const char *) - 1;

    auto & opt = LinuxDeviceOptions::GetInstance();
    auto cfg   = mpc_get_config();

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

    // Matter and Matter Application initialization
    VerifyOrDie(ChipLinuxAppInit(__argc__, const_cast<char **>(__argv__)) == 0);

    sl_log_info(LOG_TAG, "Starting the MPC main loop");

    // Run uic_main and return result
    matter_running = true;
    auto handle    = run_unify();

    // Run matter application main loop
    ChipLinuxAppMainLoop();

    matter_running = false;
    handle.join();
    return 0;
}

/* dummy init called by ChipLinuxAppMainLoop */
void ApplicationInit()
{
    // The network monitor uses Matter function, which is why this needs to be inited after
    // the Matter code has been initalized.
    mpc_nw_monitor_init();
}

void ApplicationShutdown() {}
