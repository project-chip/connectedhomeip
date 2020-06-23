/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief CLI for the Price Common plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "price-common.h"

//=============================================================================
// Functions

void emAfPluginPriceCommonClusterGetAdjustedStartTimeCli(void)
{
  uint32_t startTimeUTc = (uint32_t)emberUnsignedCommandArgument(0);
  uint8_t durationType = (uint8_t)emberUnsignedCommandArgument(1);
  uint32_t adjustedStartTime;
  adjustedStartTime = emberAfPluginPriceCommonClusterGetAdjustedStartTime(startTimeUTc,
                                                                          durationType);
  UNUSED_VAR(adjustedStartTime);
  emberAfPriceClusterPrintln("adjustedStartTime: 0x%4X", adjustedStartTime);
}

void emAfPluginPriceCommonClusterConvertDurationToSecondsCli(void)
{
  uint32_t startTimeUtc = (uint32_t)emberUnsignedCommandArgument(0);
  uint32_t duration = (uint32_t)emberUnsignedCommandArgument(1);
  uint8_t durationType = (uint8_t)emberUnsignedCommandArgument(2);
  emberAfPriceClusterPrintln("seconds: %d",
                             emberAfPluginPriceCommonClusterConvertDurationToSeconds(startTimeUtc,
                                                                                     duration,
                                                                                     durationType));
}

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginPriceCommonCommands[] = {
  emberCommandEntryAction("adj-st-t",
                          emAfPluginPriceCommonClusterGetAdjustedStartTimeCli,
                          "wu",
                          "Calculates a new UTC start time value based on the duration type parameter."),
  emberCommandEntryAction("cnvrt-durn-to-sec",
                          emAfPluginPriceCommonClusterConvertDurationToSecondsCli,
                          "wwu",
                          "Converts the duration to a number of seconds based on the duration type parameter."),
  emberCommandEntryTerminator(),
};
#endif
