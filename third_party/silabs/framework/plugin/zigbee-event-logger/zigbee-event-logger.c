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
 * @brief Routines for the Zigbee Event Logger plugin, which provide a means of
 *        recording zigbee network events in a ring buffer.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "stack/include/zigbee-event-logger-gen.h"
#include "app/framework/plugin/compact-logger/compact-logger.h"

#ifdef EZSP_HOST
 #define getNetworkParameters(nodeType, params) ezspGetNetworkParameters(nodeType, params)
#else // EZSP_HOST
 #define getNetworkParameters(nodeType, params) emberGetNetworkParameters(params)
#endif // EZSP_HOST

void emberAfPluginZigbeeEventLoggerInitCallback(void)
{
  emberAfPluginCompactLoggerInit();
#ifndef EZSP_HOST
  uint32_t bootCount;
  halCommonGetToken(&bootCount, TOKEN_STACK_BOOT_COUNTER);
  uint16_t resetType = halGetExtendedResetInfo();
  emberAfPluginZigBeeEventLoggerAddReset(RESET_BASE_TYPE(resetType),
                                         RESET_EXTENDED_FIELD(resetType));
  emberAfPluginZigBeeEventLoggerAddBootEvent(bootCount, resetType);
#endif // EZSP_HOST
}

void emberAfPluginZigbeeEventLoggerStackStatusCallback(EmberStatus status)
{
  EmberStatus retVal;
  EmberNetworkParameters networkParameters;
  EmberMultiPhyRadioParameters radioParameters;
#ifdef EZSP_HOST
  EmberNodeType nodeType;
#endif // EZSP_HOST

  // We'll do a join network message (if applicable), followed by a stack status
  // message

  if ((status == EMBER_NETWORK_UP) || (status == EMBER_CHANNEL_CHANGED)) {
    retVal = getNetworkParameters(&nodeType, &networkParameters);
    if (retVal == EMBER_SUCCESS) {
      // The following does not work for simultaneous dual radio devices
      retVal = emberGetRadioParameters(0, &radioParameters);
    }
    if (retVal == EMBER_SUCCESS) {
      if (status == EMBER_NETWORK_UP) {
        emberAfPluginZigBeeEventLoggerAddJoinNetwork(networkParameters.panId,
                                                     networkParameters.radioChannel,
                                                     radioParameters.radioPage,
                                                     networkParameters.extendedPanId);
      } else {
        emberAfPluginZigBeeEventLoggerAddChannelChange(radioParameters.radioPage,
                                                       networkParameters.radioChannel);
      }
    }
  }

  emberAfPluginZigBeeEventLoggerAddStackStatus(status);
}

void emberAfPluginZigbeeEventLoggerZigbeeKeyEstablishmentCallback(EmberEUI64 partner,
                                                                  EmberKeyStatus status)
{
  EmberStatus emberStatus;
  EmberEUI64 trustCenterEui64;

  if (status == EMBER_VERIFY_LINK_KEY_SUCCESS) {
    emberStatus = emberLookupEui64ByNodeId(EMBER_TRUST_CENTER_NODE_ID,
                                           trustCenterEui64);
    if ((emberStatus == EMBER_SUCCESS)
        && (0 == MEMCOMPARE(partner, trustCenterEui64, EUI64_SIZE))) {
      emberAfPluginZigBeeEventLoggerAddTrustCenterLinkKeyChange();
    }
  }
}

void emberAfPluginCompactLoggerUtcTimeSetCallback(uint32_t currentUtcTimeSeconds)
{
  uint32_t secondsSinceBoot = halCommonGetInt32uMillisecondTick()
                              / MILLISECOND_TICKS_PER_SECOND;
  emberAfPluginZigBeeEventLoggerAddTimeSync(currentUtcTimeSeconds,
                                            secondsSinceBoot);
}
