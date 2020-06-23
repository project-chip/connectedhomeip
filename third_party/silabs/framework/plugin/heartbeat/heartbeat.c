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
 * @brief Simple code to blink a light to simulate a heartbeat.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include PLATFORM_HEADER
#include CONFIGURATION_HEADER

void emberAfPluginHeartbeatTickCallback(void)
{
  static uint32_t lastMs = 0;
  uint32_t nowMs = halCommonGetInt32uMillisecondTick();
  if (EMBER_AF_PLUGIN_HEARTBEAT_PERIOD_QS * MILLISECOND_TICKS_PER_QUARTERSECOND
      < elapsedTimeInt32u(lastMs, nowMs)) {
    halToggleLed(BOARD_HEARTBEAT_LED);
    lastMs = nowMs;
  }
}
