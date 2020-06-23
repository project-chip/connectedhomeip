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
 * @brief Definitions for the Update TC Link Key plugin, which provides a way
 *        for joining devices to request a new link key after joining a Zigbee
 *        3.0 network.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "update-tc-link-key.h"

static bool inRequest = false;

EmberEventControl emberAfPluginUpdateTcLinkKeyBeginTcLinkKeyUpdateEventControl;
#define beginTcLinkKeyUpdateEvent (emberAfPluginUpdateTcLinkKeyBeginTcLinkKeyUpdateEventControl)

// Setting the default timer to a day.
static uint32_t LinkKeyUpdateTimerMilliseconds = MILLISECOND_TICKS_PER_DAY;

// -----------------------------------------------------------------------------
// Public API

EmberStatus emberAfPluginUpdateTcLinkKeyStart(void)
{
  EmberStatus status;

  status = emberUpdateTcLinkKey(EMBER_AF_PLUGIN_UPDATE_TC_LINK_KEY_MAX_ATTEMPTS);
  if (status == EMBER_SUCCESS) {
    inRequest = true;
  }

  return status;
}

bool emberAfPluginUpdateTcLinkKeyStop(void)
{
  bool wasInRequest = inRequest;
  inRequest = false;
  return wasInRequest;
}

void emberAfPluginUpdateTcLinkKeySetDelay(uint32_t delayMs)
{
  emberEventControlSetDelayMS(beginTcLinkKeyUpdateEvent, delayMs);
}

void emberAfPluginUpdateTcLinkKeySetInactive(void)
{
  emberEventControlSetInactive(beginTcLinkKeyUpdateEvent);
}

void emberAfPluginUpdateTcLinkKeyZigbeeKeyEstablishmentCallback(EmberEUI64 partner,
                                                                EmberKeyStatus status)
{
  if (inRequest) {
    emberAfCorePrint("%p:", EMBER_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME);

    if ((status == EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED)
        || (status == EMBER_VERIFY_LINK_KEY_SUCCESS)) {
      emberAfCorePrint(" New key established:");
    } else if (status != EMBER_APP_LINK_KEY_ESTABLISHED) {
      emberAfCorePrint(" Error:");
    }
    emberAfCorePrintln(" 0x%X", status);
    emberAfCorePrint("Partner: ");
    emberAfCorePrintBuffer(partner, EUI64_SIZE, true); // withSpace?
    emberAfCorePrintln("");

    // Anything greater than EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED is the
    // final state for a joining device
    if (status > EMBER_TRUST_CENTER_LINK_KEY_ESTABLISHED) {
      inRequest = false;
    }

    emberAfPluginUpdateTcLinkKeyStatusCallback(status);
  }
}

// =============================================================================
// Begin Update of TC Link Key

// The TC link key may be updated on a timely basis, if desired. For instance,
// if a device is joining a network where the Trust Center is pre-R21 and the
// device knows that the TC will be upgraded to post-R21 at some point, the
// joining device may choose to update its Trust Center link key on a timely
// basis such that, eventually, the update completes successfully.
// Other applications may choose to regularly update their TC link key for
// security reasons.

void emberAfPluginUpdateTcLinkKeyBeginTcLinkKeyUpdateEventHandler(void)
{
  if (!inRequest) {
    emberEventControlSetInactive(beginTcLinkKeyUpdateEvent);
    EmberStatus status = emberAfPluginUpdateTcLinkKeyStart();
    emberAfCorePrintln("%p: %p: 0x%X",
                       EMBER_AF_PLUGIN_UPDATE_TC_LINK_KEY_PLUGIN_NAME,
                       "Starting update trust center link key process",
                       status);
    if (status != EMBER_SUCCESS) {
      emberLeaveNetwork();
      emAfPluginNetworkSteeringCleanup(status);
    }
  }
  emberAfPluginUpdateTcLinkKeySetDelay(LinkKeyUpdateTimerMilliseconds);
}

void emberAfPluginSetTCLinkKeyUpdateTimerMilliSeconds(uint32_t timeInMilliseconds)
{
  LinkKeyUpdateTimerMilliseconds = timeInMilliseconds;
}
