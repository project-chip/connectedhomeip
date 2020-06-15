/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief CLI for the Idle Sleep plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "idle-sleep.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The Idle/Sleep plugin is not compatible with the legacy CLI.
#endif

void emberAfPluginIdleSleepStatusCommand(void)
{
  emberAfCorePrintln("Stay awake when not joined: %p",
                     (emAfStayAwakeWhenNotJoined ? "yes" : "no"));
  emberAfCorePrintln("Forced stay awake:          %p",
                     (emAfForceEndDeviceToStayAwake ? "yes" : "no"));
}

void emberAfPluginIdleSleepStayAwakeCommand(void)
{
  bool stayAwake = (bool)emberUnsignedCommandArgument(0);
  if (stayAwake) {
    emberAfCorePrintln("Forcing device to stay awake");
  } else {
    emberAfCorePrintln("Allowing device to go to sleep");
  }
  emberAfForceEndDeviceToStayAwake(stayAwake);
}

void emberAfPluginIdleSleepAwakeWhenNotJoinedCommand(void)
{
  emAfStayAwakeWhenNotJoined = (bool)emberUnsignedCommandArgument(0);
}
