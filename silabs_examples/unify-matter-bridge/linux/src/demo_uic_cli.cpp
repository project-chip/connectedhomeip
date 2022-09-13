/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "demo_uic_cli.h"

// UIC components
#include "sl_log.h"
#include "uic_stdin.hpp"

/// File descripter for output stream
static int out_stream;

// UIC demo application pre decleration of application commands functions
static sl_status_t handle_demo(const handle_args_t & arg);

#define LOG_TAG "demo_uic_cli"

// Command map
const std::map<std::string, std::pair<std::string, handler_func>> commands = {
    { "demo_command", { "Demo command doing nothing but adding a command to base CLI", handle_demo } }
};

// Public
void demo_uic_cli_setup()
{
    uic_stdin_add_commands(commands);
    uic_stdin_set_prompt_string("DEMO_UIC> ");
    out_stream = uic_stdin_get_output_fd();
}

// Internal
static sl_status_t handle_demo(const handle_args_t & arg)
{
    sl_log_debug(LOG_TAG, "UIC demo CLI printing args");

    for (auto it = arg.begin(); it != arg.end(); ++it)
    {
        printf("arg: %s\n", it->c_str());
    }

    return SL_STATUS_OK;
}
