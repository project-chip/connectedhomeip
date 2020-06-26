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
 * @brief CLI APIs for the Counters plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PLUGIN_COUNTERS_CLI_H
#define SILABS_PLUGIN_COUNTERS_CLI_H

void emberAfPluginCountersPrintCommand(void);
void emberAfPluginCountersSimplePrintCommand(void);
void emberAfPluginCountersPrintThresholdsCommand(void);
void emberAfPluginCountersSetThresholdCommand(void);
void emberAfPluginCountersSendRequestCommand(void);
void emberAfPluginCounterPrintCountersResponse(EmberMessageBuffer message);

/** Args: destination id, clearCounters (bool) */
void sendCountersRequestCommand(void);

/** Utility function for printing out the OTA counters response. */
void printCountersResponse(EmberMessageBuffer message);

#endif // SILABS_PLUGIN_COUNTERS_CLI_H
