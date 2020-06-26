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
 * @brief Host routines for the Zigbee Event Logger plugin.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "stack/include/zigbee-event-logger-gen.h"
#include "app/framework/plugin/compact-logger/compact-logger.h"

void ezspSwitchNetworkKeyHandler(uint8_t sequenceNumber)
{
  emberAfPluginZigBeeEventLoggerAddNetworkKeySequenceChange(sequenceNumber);
}

void ezspChildJoinHandler(uint8_t index,
                          bool joining,
                          EmberNodeId childId,
                          EmberEUI64 childEui64,
                          EmberNodeType childType)
{
  if (joining) {
    emberAfPluginZigBeeEventLoggerAddChildAdded(childId,
                                                childEui64);
  } else {
    emberAfPluginZigBeeEventLoggerAddChildRemoved(childId,
                                                  childEui64);
  }
}

void ezspDutyCycleHandler(uint8_t channelPage,
                          uint8_t channel,
                          EmberDutyCycleState state,
                          uint8_t totalDevices,
                          EmberPerDeviceDutyCycle *arrayOfDeviceDutyCycles)
{
  EmberStatus emberStatus;
  EmberDutyCycleLimits dutyCycleLimits;

  emberStatus = ezspGetDutyCycleLimits(&dutyCycleLimits);

  if (emberStatus == EMBER_SUCCESS) {
    emberAfPluginZigBeeEventLoggerAddDutyCycleChange(state,
                                                     dutyCycleLimits.limitThresh,
                                                     dutyCycleLimits.critThresh,
                                                     dutyCycleLimits.suspLimit);
  }
}
