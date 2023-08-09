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
#include "mpc_cli.hpp"
#include "uic_stdin.hpp"
#include <map>

void mpc_stdin_add_commands(const std::map<std::string, std::pair<std::string, handler_func>> & append_commands);

sl_status_t mpc_stdin_handle_command(const char * command);
