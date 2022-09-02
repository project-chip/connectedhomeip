/***************************************************************************//**
 * @file
 * @brief Configuration file for CLI.
 * @version x.y.z
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_CLI_CONFIG_EXAMPLE_H
#define SL_CLI_CONFIG_EXAMPLE_H

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/

// <o SL_CLI_EXAMPLE_TASK_PRIORITY> CLI instance task priority
// <osPriorityLow1=> low + 1
// <osPriorityLow2=> low + 2
// <osPriorityLow3=> low + 3
// <osPriorityLow4=> low + 4
// <osPriorityLow5=> low + 5
// <osPriorityLow6=> low + 6
// <osPriorityLow7=> low + 7
// <osPriorityBelowNormal=> below normal
// <osPriorityBelowNormal1=> below normal + 1
// <osPriorityBelowNormal2=> below normal + 2
// <osPriorityBelowNormal3=> below normal + 3
// <osPriorityBelowNormal4=> below normal + 4
// <osPriorityBelowNormal5=> below normal + 5
// <osPriorityBelowNormal6=> below normal + 6
// <osPriorityBelowNormal7=> below normal + 7
// <osPriorityNormal=> normal
// <osPriorityNormal1=> normal + 1
// <osPriorityNormal2=> normal + 2
// <osPriorityNormal3=> normal + 3
// <osPriorityNormal4=> normal + 4
// <osPriorityNormal5=> normal + 5
// <osPriorityNormal6=> normal + 6
// <osPriorityNormal7=> normal + 7
// <osPriorityAboveNormal => above normal
// <osPriorityAboveNormal1=> above normal + 1
// <osPriorityAboveNormal2=> above normal + 2
// <osPriorityAboveNormal3=> above normal + 3
// <osPriorityAboveNormal4=> above normal + 4
// <osPriorityAboveNormal5=> above normal + 5
// <osPriorityAboveNormal6=> above normal + 6
// <osPriorityAboveNormal7=> above normal + 7
// <osPriorityHigh=> high
// <osPriorityHigh1=> high + 1
// <osPriorityHigh2=> high + 2
// <osPriorityHigh3=> high + 3
// <osPriorityHigh4=> high + 4
// <osPriorityHigh5=> high + 5
// <osPriorityHigh6=> high + 6
// <osPriorityHigh7=> high + 7
// <osPriorityRealtime=> realtime
// <osPriorityRealtime1=> realtime + 1
// <osPriorityRealtime2=> realtime + 2
// <osPriorityRealtime3=> realtime + 3
// <osPriorityRealtime4=> realtime + 4
// <osPriorityRealtime5=> realtime + 5
// <osPriorityRealtime6=> realtime + 6
// <osPriorityRealtime7=> realtime + 7
// <i> Default: osPriorityNormal1
// <i> Priority of the CLI instance task using the CMSIS order and using the enum name
  #define SL_CLI_EXAMPLE_TASK_PRIORITY      osPriorityNormal1

// <o SL_CLI_EXAMPLE_TASK_STACK_SIZE> Task stack size
// <i> Default: 400
// <i> Stack size for the CLI task in words.
  #define SL_CLI_EXAMPLE_TASK_STACK_SIZE       400

// <o SL_CLI_EXAMPLE_TASK_START_DELAY_MS> CLI task start delay
// <i> Default: 0
// <i> Specifies the delay in milliseconds before the cli task start to poll for input.
  #define SL_CLI_EXAMPLE_TASK_START_DELAY_MS   0

// <o SL_CLI_EXAMPLE_TASK_LOOP_DELAY_MS> CLI task loop delay
// <i> Default: 50
// <i> Specifies the delay in milliseconds after the cli task has polled the input.
  #define SL_CLI_EXAMPLE_TASK_LOOP_DELAY_MS    50

// <o SL_CLI_EXAMPLE_IOSTREAM_HANDLE> The iostream handle used by the cli instance
// <i> Default: sl_iostream_get_default()
  #define SL_CLI_EXAMPLE_IOSTREAM_HANDLE    sl_iostream_get_default()

// <o SL_CLI_EXAMPLE_COMMAND_GROUP> The default command group.
// <i> Default: sl_cli_default_command_group.
// <i> Specifies a pointer to the command group that shall be used as the
// <i> default command group for the cli instance.
// <i> The template file that is creating cli instances will also create one
// <i> command group for each cli instance. During initialization, the
// <i> sl_cli_default_command_group will be updated for each instance to point
// <i> to the instance version of the command group. This makes it possible to
// <i> use the same variable for all instances, but each getting its own
// <i> command group.
// <i> The application can provide a group created elsewhere in the
// <i> application or use an empty definition for no default group.
// <i> Please note: A command group can only be used once.
  #define SL_CLI_EXAMPLE_COMMAND_GROUP      sl_cli_default_command_group

#endif // SL_CLI_CONFIG_EXAMPLE_H

// <<< end of configuration section >>>
