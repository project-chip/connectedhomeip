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

#include "uic_stdin.hpp"
#include "uic_stdin_process.h"

#include "Options.h"
#include "app/server/OnboardingCodesUtil.h"
#include "app/server/Server.h"
#include "matter_node_state_monitor.hpp"

using namespace chip;
using namespace unify::matter_bridge;
static matter_node_state_monitor * node_state_monitor = NULL;
#define LOG_TAG "matter_bridge_cli"

// CLI handling
static sl_status_t commission_cli_func(const handle_args_t & arg)
{
    LinuxDeviceOptions::GetInstance().payload.commissioningFlow = chip::CommissioningFlow::kStandard;
    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);
    Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    return SL_STATUS_OK;
}

void set_matter_node_state_monitor_for_cli(matter_node_state_monitor & n)
{
    node_state_monitor = &n;
}

static sl_status_t epmap_cli_func(const handle_args_t & arg)
{
    sl_log_debug(LOG_TAG, "epmap_cli_func" );
    if (node_state_monitor)
    {
        node_state_monitor->display_map();
    }
    return SL_STATUS_OK;
}

command_map_t unify_cli_commands = {
    { "commission", { "Open commissioning window", commission_cli_func } },
    { "epmap", { "Show endpoint map", epmap_cli_func } },
};

sl_status_t matter_bridge_cli_init()
{
    uic_stdin_add_commands(unify_cli_commands);
    return SL_STATUS_OK;
}
