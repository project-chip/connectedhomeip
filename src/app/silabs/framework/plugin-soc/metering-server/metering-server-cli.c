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
 * @brief CLI for the Metering Server plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "metering-interface.h"
#include "metering-server.h"

// plugin simple-metering-server profiles <data:1>
void emberAfPluginMeteringServerCliProfiles(void)
{
#if (EMBER_AF_PLUGIN_SIMPLE_METERING_SERVER_METER_PROFILES != 0)
  emberAfPluginMeteringServerInterfaceEnableProfiles(
    (uint8_t)emberUnsignedCommandArgument(0));
#else
  emberAfCorePrintln("Not applicable for 0 configured profiles.");
#endif
}

void emberAfPluginMeteringServerCliDisableFastPolling(void)
{
  bool setFastPolling = (bool)emberUnsignedCommandArgument(0);
  emberAfPluginMeteringServerEnableFastPolling(setFastPolling);
}
