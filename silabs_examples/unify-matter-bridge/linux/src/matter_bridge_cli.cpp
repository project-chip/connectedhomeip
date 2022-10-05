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


#include "uic_stdin_process.h"
#include "uic_stdin.hpp"

#include "Options.h"
#include "app/server/Server.h"
#include "app/server/OnboardingCodesUtil.h"

using namespace chip;

// CLI handling
static sl_status_t commission_cli_func(const handle_args_t &arg) {
    LinuxDeviceOptions::GetInstance().payload.commissioningFlow = chip::CommissioningFlow::kStandard;
    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);
    Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    return SL_STATUS_OK;
}


command_map_t unify_cli_commands = {
    {"commission",{"Open commissioning window", commission_cli_func} },
};



sl_status_t matter_bridge_cli_init() {
  uic_stdin_add_commands(unify_cli_commands);
  return SL_STATUS_OK;
}