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
 * @brief APIs and defines for the Counters plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PLUGIN_COUNTERS_H
#define SILABS_PLUGIN_COUNTERS_H

#define EMBER_APPLICATION_HAS_COUNTER_ROLLOVER_HANDLER
/**
 * The ith entry in this array is the count of events of EmberCounterType i.
 */
extern uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];
/**
 * The ith entry in this array is the count of threshold values set for
 * the corresponding ith event in emberCounters. The default value is set to
 * 0xFFFF and can be changed by an application by calling
 * emberSet
 */
#ifdef EMBER_APPLICATION_HAS_COUNTER_ROLLOVER_HANDLER
extern uint16_t emberCountersThresholds[EMBER_COUNTER_TYPE_COUNT];
#endif

#if !defined(EMBER_MULTI_NETWORK_STRIPPED)
#define MULTI_NETWORK_COUNTER_TYPE_COUNT 17
/**
 * The value at the position [n,i] in this matrix is the count of events of
 * per-network EmberCounterType i for network n.
 */
extern uint16_t emberMultiNetworkCounters[EMBER_SUPPORTED_NETWORKS]
[MULTI_NETWORK_COUNTER_TYPE_COUNT];
#endif // EMBER_MULTI_NETWORK_STRIPPED

/** Reset the counters to zero. */
void emberAfPluginCountersClear(void);
/** Resets counter thresholds to 0xFFFF. **/
void emberAfPluginCountersResetThresholds(void);
/** Set the threshold for a particular counter type **/
void emberAfPluginCountersSetThreshold(EmberCounterType type, uint16_t threshold);

#endif // SILABS_PLUGIN_COUNTERS_H
