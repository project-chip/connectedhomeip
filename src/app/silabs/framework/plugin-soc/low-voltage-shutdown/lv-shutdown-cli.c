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
 * @brief CLI for the LV Shutdown plugin.
 *******************************************************************************
   ******************************************************************************/

#include "af.h"
#include "lv-shutdown.h"

#ifdef EMBER_AF_LEGACY_CLI
  #error The Low Voltage Shutdown plugin is not compatible with the legacy CLI.
#endif

// ****** CLI Command handlers ******
void emberAfPluginLowVoltageShutdownEnableCommand(void)
{
  bool enable = emberUnsignedCommandArgument(0);
  emberAfPluginLowVoltageShutdownEnable(enable);
}

void emberAfPluginLowVoltageShutdownStatusCommand(void)
{
  emberAfAppPrint("Low voltage shutdown logic is ");
  if (emberAfPluginLowVoltageShutdownEnabled()) {
    emberAfAppPrintln("enabled.");
  } else {
    emberAfAppPrintln("disabled.");
  }
}

void emberAfPluginLowVoltageShutdownPrintVddCommand()
{
  emberAfAppPrintln("VDD voltage is %d mV",
                    emberAfPluginLowVoltageShutdownGetVoltage());
}
