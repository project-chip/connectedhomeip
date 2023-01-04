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
#include "group_command_translator.hpp"
#include "matter_node_state_monitor.hpp"
#include <iostream>

using namespace chip;
using namespace unify::matter_bridge;
static matter_node_state_monitor * node_state_monitor = NULL;
static group_translator * group_mapping_instance      = NULL;

#define LOG_TAG "matter_bridge_cli"

// CLI handling
static sl_status_t commission_cli_func(const handle_args_t & arg)
{
    LinuxDeviceOptions::GetInstance().payload.commissioningFlow = chip::CommissioningFlow::kStandard;
    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);
    Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    return SL_STATUS_OK;
}

static sl_status_t close_commission_cli_func(const handle_args_t & arg)
{
    Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    return SL_STATUS_OK;
}

void set_mapping_display_instance(matter_node_state_monitor & n, group_translator & m)
{
    node_state_monitor     = &n;
    group_mapping_instance = &m;
}

static sl_status_t epmap_cli_func(const handle_args_t & arg)
{
    sl_log_debug(LOG_TAG, "epmap_cli_func");
    if (node_state_monitor)
    {
        node_state_monitor->display_map(std::cout);
    }
    return SL_STATUS_OK;
}

static sl_status_t groups_map_cli_func(const handle_args_t & arg)
{
    sl_log_debug(LOG_TAG, "groups_map_cli_func");
    if (group_mapping_instance)
    {
        group_mapping_instance->display_group_mapping(std::cout);
    }
    return SL_STATUS_OK;
}

command_map_t unify_cli_commands = {
    { "commission", { "Open commissioning window", commission_cli_func } },
    { "closecommission", { "Close the commissioning window", close_commission_cli_func } },
    { "epmap", { "Show endpoint map", epmap_cli_func } },
    { "groups_map", { "Show Matter vs Unify groups map", groups_map_cli_func } },
};

sl_status_t matter_bridge_cli_init()
{
    uic_stdin_add_commands(unify_cli_commands);
    return SL_STATUS_OK;
}
