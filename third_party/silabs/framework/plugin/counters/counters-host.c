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
 * @brief Provides the host utility functions for counters.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/common/common.h"
#include "counters.h"
#include "counters-ota.h"

uint16_t emberCounters[EMBER_COUNTER_TYPE_COUNT];
uint16_t emberCountersThresholds[EMBER_COUNTER_TYPE_COUNT];

void emberAfPluginCountersInitCallback(void)
{
  emberAfPluginCountersClear();
  emberAfPluginCountersResetThresholds();
}

void ezspCounterRolloverHandler(EmberCounterType type)
{
  emberAfPluginCountersRolloverCallback(type);
}

void emberAfPluginCountersClear(void)
{
  EzspStatus status;
  status = ezspSetValue(EZSP_VALUE_CLEAR_COUNTERS, 0, NULL);
  if (status != EZSP_SUCCESS) {
    emberAfCorePrintln("ezsp counters failed %u", status);
  }
}

void emberAfPluginCountersResetThresholds(void)
{
  //Clear local copy of thresholds.

  EzspStatus status;
  status = ezspSetValue(EZSP_VALUE_RESET_COUNTER_THRESHOLDS, 0, NULL);
  if (status != EZSP_SUCCESS) {
    emberAfCorePrintln("ezsp Reset Threshold error %u", status);
  } else {
    MEMSET(emberCountersThresholds, 0xFF, sizeof(emberCountersThresholds));
  }
}

void emberAfPluginCountersSetThreshold(EmberCounterType type, uint16_t threshold)
{
  EzspStatus status;
  uint8_t ezspThreshold[3];
  ezspThreshold[0] = type;
  ezspThreshold[1] = LOW_BYTE(threshold);
  ezspThreshold[2] = HIGH_BYTE(threshold);
  status = ezspSetValue(EZSP_VALUE_SET_COUNTER_THRESHOLD, sizeof(ezspThreshold), ezspThreshold);
  if (status != EZSP_SUCCESS) {
    emberAfCorePrintln("ezsp Set Threshold error %u", status);
  } else {
    emberCountersThresholds[type] = threshold;
  }
}
