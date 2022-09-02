/***************************************************************************//**
 * @file
 * @brief Configuration file for CLI.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_CLI_CONFIG_H
#define SL_CLI_CONFIG_H

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/

// <h>CLI Framework Configuration

// <s SL_CLI_UNIT_SEPARATOR> String of the ascii unit separator.
// <i> Default: "\x1f"
// <i> Define the string that separates two commands.
#define SL_CLI_UNIT_SEPARATOR              "\x1f"

// <o SL_CLI_UNIT_SEPARATOR_CHAR> Char of the ascii unit separator.
// <i> Default: 0x1fU
// <i> Define the command prompt indicating that a new command may be written.
#define SL_CLI_UNIT_SEPARATOR_CHAR         0x1fU

// <o SL_CLI_MAX_INPUT_ARGUMENTS> Max number of input arguments <0-32>
// <i> Default: 8
// <i> Define the number of input arguments the application needs.
#define SL_CLI_MAX_INPUT_ARGUMENTS         16

// <o SL_CLI_INPUT_BUFFER_SIZE> Size of input buffer <8-256>
// <i> Default: 128
// <i> Define the maximum number of input characters needed by a command,
// <i> including the arguments.
#define SL_CLI_INPUT_BUFFER_SIZE           128

// <o SL_CLI_OUTPUT_BUFFER_SIZE> Size of output buffer <8-256>
// <i> Default: 128
// <i> Define the maximum number of characters in an output string.
#define SL_CLI_OUTPUT_BUFFER_SIZE          128

// <s SL_CLI_PROMPT_STRING> New command prompt
// <i> Default: "> "
// <i> Define the command prompt indicating that a new command may be written.
#define SL_CLI_PROMPT_STRING               "host> "

// <q SL_CLI_HELP_DESCRIPTION_ENABLED> Enable help descriptions
// <i> Default: 1
// <i> If enabled, 'help' command will print out help messages for commands
#define SL_CLI_HELP_DESCRIPTION_ENABLED    1

// <o SL_CLI_HELP_CMD_PRE> Indentation size when printing help
// <i> Default: "2"
// <i> Define the indentation size for help message printouts.
#define SL_CLI_HELP_CMD_PRE                2

// <o SL_CLI_HELP_CMD_SIZE> Size of command name
// <i> Default: "30"
// <i> Define the size of command names when printing help messages.
#define SL_CLI_HELP_CMD_SIZE               30

// <q SL_CLI_LOCAL_ECHO> Enable local echo
// <i> Default: 1
// <i> If enabled, what is written in the terminal will be echoed back, so the
// <i> user will see what is written.
#define SL_CLI_LOCAL_ECHO                  1

// <q SL_CLI_ADVANCED_INPUT_HANDLING> Enable advanced input handling
// <i> Default: 1
// <i> If enabled, the CLI is extended with arrow handling, autocompletion
// <i> of commands, and delete/backspace in the middle of words.
#define SL_CLI_ADVANCED_INPUT_HANDLING     1

// <o SL_CLI_NUM_HISTORY_BYTES> Max number of bytes of history <0-4096>
// <i> Default: 512
// <i> Define the number of bytes of history that are saved.
#define SL_CLI_NUM_HISTORY_BYTES           512
// </h>

#if SL_CLI_NUM_HISTORY_BYTES < SL_CLI_INPUT_BUFFER_SIZE && SL_CLI_NUM_HISTORY_BYTES != 0
  #error "Number of history bytes needs to be zero or bigger than input buffer"
#endif

// <q SL_CLI_IGNORE_COMMAND_CASE> Ignore command case
// <i> Default: 1
// <i> If enabled, the CLI will ignore the case for commands.
#define SL_CLI_IGNORE_COMMAND_CASE     1

#endif // SL_CLI_CONFIG_H

// <<< end of configuration section >>>
