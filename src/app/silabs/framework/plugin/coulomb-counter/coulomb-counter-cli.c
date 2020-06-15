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
 * @brief CLI for the Coulomb Counter plugin.
 *******************************************************************************
   ******************************************************************************/

// CLI should only be available in SoC apps
#ifndef EMBER_AF_NCP
#include "app/framework/include/af.h"
#include "coulomb-counter.h"

#ifndef EMBER_AF_GENERATE_CLI
  #error The Coulomb Counter plugin is not compatible with the legacy CLI.
#endif

void emberAfPluginCoulombCounterCurrentCoreModeCommand(void)
{
  uint16_t coreMode = emberAfPluginCommandCoulombCounterGetCurrentCoreMode();

  emberAfCorePrint("Current EM mode: ");
  switch (coreMode & EMBER_AF_PLUGIN_COULOMB_COUNTER_EM_MODE_MASK) {
    case EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_SLEEP:
      emberAfCorePrintln("Sleep");
      break;
    case EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_IDLE:
      emberAfCorePrintln("Idle");
      break;
    case EMBER_AF_PLUGIN_COULOMB_COUNTER_CORE_MODE_AWAKE:
      emberAfCorePrintln("Awake");
      break;
    default:
      emberAfCorePrintln("Invalid");
      return;
  }

  emberAfCorePrint("Current Radio state: ");
  coreMode &= EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_EVENT_MASK;
  if ((coreMode & EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_MODE_TX) != 0U) {
    emberAfCorePrintln("Tx");
  } else if ((coreMode & EMBER_AF_PLUGIN_COULOMB_COUNTER_RADIO_MODE_RX) != 0U) {
    emberAfCorePrintln("Rx");
  } else {
    emberAfCorePrintln("Idle");
  }
}

void emberAfPluginCoulombCounterAdditiveModesCommand(void)
{
  uint16_t additiveModes = emberAfPluginCommandCoulombCounterGetAdditiveModesBitfield();
  uint8_t i;

  for (i = 0; i < COULOMB_COUNTER_MAX_NB_USER_DEFINED_ADDITIVE_STATES; ++i) {
    emberAfCorePrintln("%u: %s", i, ((additiveModes >> i & 0x1) ? "ON" : "OFF"));
  }
}

void emberAfPluginCoulombCounterEnergyUsageCommand(void)
{
  emberAfCorePrintln("Current energy usage: %lu", emberAfPluginCommandCoulombCounterGetEnergyUsage());
}

void emberAfPluginCoulombCounterResetEnergyUsageCommand(void)
{
  emberAfPluginCommandCoulombCounterResetEnergyUsage();
  emberAfCorePrintln("Energy usage data has been reset");
}
#endif //EMBER_AF_NCP
