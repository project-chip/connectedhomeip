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
 * @brief CLI for the Throughtput Legacy plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "throughput-legacy.h"

#if !defined(EMBER_AF_GENERATE_CLI)
EmberCommandEntry emberAfPluginThroughputLegacyCommands[] = {
  emberCommandEntryAction("inflight", emAfPluginThroughputSetInFlightCount, "u", "Set the number of packets in flight during the test."),
  emberCommandEntryAction("duration", emAfPluginThroughputSetDuration, "w", "Set the duration in ms for the test."),
  emberCommandEntryAction("start", emAfPluginThroughputStartTest, "", "Start the throughput test."),
  emberCommandEntryAction("end", emAfPluginThroughputEndTest, "", "Abort the test while running."),
  emberCommandEntryAction("result", emAfPluginThroughputPrintResult, "", "Show the results of the last test."),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI
