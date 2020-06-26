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
 * @brief SoC routines for the Zigbee Event Logger plugin.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "stack/include/zigbee-event-logger-gen.h"
#include "app/framework/plugin/compact-logger/compact-logger.h"

void emberSwitchNetworkKeyHandler(uint8_t sequenceNumber)
{
  emberAfPluginZigBeeEventLoggerAddNetworkKeySequenceChange(sequenceNumber);
}

void emberChildJoinHandler(uint8_t index, bool joining)
{
  EmberStatus status;
  EmberChildData childData;

  status = emberGetChildData(index, &childData);
  if (status == EMBER_SUCCESS) {
    if (joining) {
      emberAfPluginZigBeeEventLoggerAddChildAdded(childData.id,
                                                  childData.eui64);
    } else {
      emberAfPluginZigBeeEventLoggerAddChildRemoved(childData.id,
                                                    childData.eui64);
    }
  }
}

void emberDutyCycleHandler(uint8_t channelPage,
                           uint8_t channel,
                           EmberDutyCycleState state)
{
  EmberStatus emberStatus;
  EmberDutyCycleLimits dutyCycleLimits;

  emberStatus = emberGetDutyCycleLimits(&dutyCycleLimits);

  if (emberStatus == EMBER_SUCCESS) {
    emberAfPluginZigBeeEventLoggerAddDutyCycleChange(state,
                                                     dutyCycleLimits.limitThresh,
                                                     dutyCycleLimits.critThresh,
                                                     dutyCycleLimits.suspLimit);
  }
}
