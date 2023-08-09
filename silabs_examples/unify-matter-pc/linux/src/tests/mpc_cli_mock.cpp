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
#include "mpc_cli_mock.h"

/* uic stdin mocks start */
static command_map_t commands;
// The prompt string for the command line interface

void uic_stdin_add_commands(const std::map<std::string, std::pair<std::string, handler_func>> & append_commands)
{
    commands.insert(append_commands.begin(), append_commands.end());
}

sl_status_t mpc_stdin_handle_command(const char * command)
{
    std::string command_str(command);
    std::string cmd(command_str.substr(0, command_str.find(' ')));
    auto iter = commands.find(cmd);
    if (iter != commands.end())
    {
        handle_args_t args;
        // add <command> as first arg
        args.push_back(cmd);
        // If there is a space after the command, look for "," seperated args
        // e.g. <command> <arg1>,<arg2>,<arg3>
        size_t pos = 0;
        if ((pos = command_str.find(' ')) != std::string::npos)
        {
            // erase "<command> " from the command_str.
            command_str.erase(0, pos + 1);
            const std::string delimiter(",");
            // look for "," seperated args in command_str
            while ((pos = command_str.find(delimiter)) != std::string::npos)
            {
                args.push_back(command_str.substr(0, pos));
                // erase <arg> from command_str
                command_str.erase(0, pos + delimiter.length());
            }
            // add last argument as well
            args.push_back(command_str);
        }
        return iter->second.second(args);
    }

    return SL_STATUS_FAIL;
}
