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
 * @brief Routines for the Comms Hub Function Sub Ghz plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "stack/include/event.h"        // emberEventControlSetInactive
#include "app/framework/plugin/meter-mirror/meter-mirror.h"

#include "comms-hub-function-sub-ghz.h"
#include "comms-hub-function-sub-ghz-types.h"

// There are no endpoints on the Sub-GHz Cluster, but emberAfWriteAttribute()
// requires one regardless. So we use the default throughout.
#define ZCL_SUB_GHZ_CLUSTER_ENDPOINT    1

//----------------------------------------------------------------------------
// Globals

EmberEventControl emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventControl;

//----------------------------------------------------------------------------
// Module level definitions

/** @brief Keep track of the current state in the channel change sequence
 *
 * The sub-GHz CHF channel change comprises several steps:
 * 1. Set the EnergyScanPending Functional Notification flag
 * 2. Wait until the GSME reads that flag or one GSME wake up period,
 *    whichever comes first
 * 3. Perform an energy scan and determine the new page and channel
 * 4. Set the ChannelChangePending Functional Notification flag
 *    and the ChannelChange attribute
 * 5. Wait until the GSME reads that attribute or two GSME wake up periods,
 *    whichever comes first
 * 6. Change the channel
 *
 * GBCS currently assumes only one GSME but there is a future expansion plan
 * to remove that limitation. To make this plugin future proof, we do not assume
 * a single GSME but we do assume that each GSME has completed its registration
 * and allocated a mirror.
 */
typedef enum {
  CHANNEL_CHANGE_STATE_IDLE = 0,        // makes initializing an array easier
  CHANNEL_CHANGE_STATE_IGNORED,         // ignore this device (e.g. not sub-GHz)
  CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_SET,
  CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_READ,
  CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_PENDING_FLAG_AND_ATTRIBUTE_SET,
  CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_PENDING_FLAG_READ,
  CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_ATTRIBUTE_READ
} ChannelChangeState;

/** @brief The per-device state of the state machine
 *
 * We use the allocated mirror(s) to identify our devices. Mirrors are needed
 * because GSMEs must read Functional Notification flags on mirror endpoints.
 *
 * Mirrors are alocated in a contiguous block the size of EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS
 * starting from EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START.
 * So there is a simple offset: channelChangeState[x] corresponds to the mirror
 * endpoint x + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START and vice-versa.
 */
static ChannelChangeState channelChangeState[EMBER_AF_PLUGIN_METER_MIRROR_MAX_MIRRORS] = { 0 };

/** @brief The sub-GHz child count
 *
 * The entire sequence applies only if there are any sub-GHz GSMEs on the network.
 * If not, then we can proceed immediately to changing the channel.
 * The count is updated every time we start the sequence.
 */
static uint8_t subGhzChildCount = 0;

//----------------------------------------------------------------------------
// Local functions

/** @brief Check whether the given child is a sub-GHz device
 */
static bool isChildSubGhz(EmberEUI64 eui64)
{
  const uint8_t childTableSize = emberAfGetChildTableSize();
  uint8_t i;

  // Iterate through the child table and try to find the device's child data
  for (i = 0; i < childTableSize; ++i) {
    EmberChildData childData;
    if (emberAfGetChildData(i, &childData) == EMBER_SUCCESS
        && MEMCOMPARE(childData.eui64, eui64, EUI64_SIZE) == 0) {
      return childData.phy == PHY_INDEX_PRO2PLUS;
    }
  }

  // Child not found, treat it as "not sub-GHz"
  return false;
}

/** @brief Check whether all mirror endpoints are at an expected state
 */
static bool checkStateForAllMirrors(ChannelChangeState expectedState)
{
  uint8_t mirrorIndex;
  for (mirrorIndex = 0;
       mirrorIndex < sizeof channelChangeState / sizeof channelChangeState[0];
       mirrorIndex++) {
    if (channelChangeState[mirrorIndex] != CHANNEL_CHANGE_STATE_IGNORED
        && channelChangeState[mirrorIndex] != expectedState) {
      return false;     // not all mirrors are at the required state, bail out
    }
  }
  return true;          // all mirrors are at the required state
}

/** @brief Check whether any mirror endpoint is at an expected state
 */
static bool checkStateForAnyMirror(ChannelChangeState expectedState)
{
  uint8_t mirrorIndex;
  for (mirrorIndex = 0;
       mirrorIndex < sizeof channelChangeState / sizeof channelChangeState[0];
       mirrorIndex++) {
    if (channelChangeState[mirrorIndex] == expectedState) {
      return true;      // at least one mirror is at the expected state
    }
  }
  return false;         // none of the mirrors are at the expected state
}

/** @brief Update state machine state for all registered devices
 */
static void updateStateForAllMirrors(ChannelChangeState newState,
                                     bool includeIgnored)
{
  uint8_t mirrorIndex;
  for (mirrorIndex = 0;
       mirrorIndex < sizeof channelChangeState / sizeof channelChangeState[0];
       mirrorIndex++) {
    if (channelChangeState[mirrorIndex] != CHANNEL_CHANGE_STATE_IGNORED
        || includeIgnored) {
      channelChangeState[mirrorIndex] = newState;
    }
  }
}

/** @brief Update flags and state
 *
 * When supporting more than one GSME, we wait until all of them are at the same
 * state in the sequence (see checkStateForAllMirrors() above) and then advance
 * them all at the same time to the next state using this function.
 */
static EmberAfPluginCommsHubFunctionStatus updateFlagsAndStateForAllMirrors(uint32_t setMask,
                                                                            ChannelChangeState newState)
{
  uint8_t mirrorIndex;
  for (mirrorIndex = 0;
       mirrorIndex < sizeof channelChangeState / sizeof channelChangeState[0];
       mirrorIndex++) {
    if (channelChangeState[mirrorIndex] != CHANNEL_CHANGE_STATE_IGNORED) {
      EmberAfPluginCommsHubFunctionStatus status =
        emAfUpdateFunctionalNotificationFlagsByEndpoint(mirrorIndex
                                                        + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START,
                                                        0xFFFFFFFF,
                                                        setMask);
      if (status != EMBER_AF_CHF_STATUS_SUCCESS) {
        return status;
      }
      channelChangeState[mirrorIndex] = newState;
    }
  }
  return EMBER_AF_CHF_STATUS_SUCCESS;
}

/** @brief Check and clear the Notification flag
 *
 * Called when we receive a callback that a GSME has read the Functional
 * Notification flags. Checks that the GSME is at the expected state and if so,
 * clears the flag and advances the GSME to the next state. Once all GSMEs have
 * read the flag, then they can all be moved along the sequence.
 */
static bool checkStateAndClearFlag(uint8_t mirrorEndpoint,
                                   ChannelChangeState expectedState,
                                   ChannelChangeState newState,
                                   uint32_t resetMask)
{
  const int8_t mirrorIndex = mirrorEndpoint
                             - EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
  if (channelChangeState[mirrorIndex] != expectedState) {
    return false;       // not the flag we are expecting
  }
  if (emAfUpdateFunctionalNotificationFlagsByEndpoint(mirrorEndpoint,
                                                      resetMask,
                                                      0) == EMBER_AF_CHF_STATUS_SUCCESS) {
    channelChangeState[mirrorIndex] = newState;
  }
  return checkStateForAllMirrors(newState);
}

/** @brief Execute the channel change and clear the ChannelChange attribute
 *
 * Assume the page/channel mask has been set previously during the sequence.
 */
static EmberAfStatus executeChannelChange(void)
{
  EmberAfAttributeType dataType;
  uint32_t pageChannelMask;
  EmberAfStatus status;

  status = emberAfReadAttribute(ZCL_SUB_GHZ_CLUSTER_ENDPOINT,
                                ZCL_SUB_GHZ_CLUSTER_ID,
                                ZCL_SUB_GHZ_CLUSTER_CHANNEL_CHANGE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)&pageChannelMask,
                                sizeof pageChannelMask,
                                &dataType);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  }

  status = emberEnergyScanRequest(EMBER_SLEEPY_BROADCAST_ADDRESS,
                                  pageChannelMask,
                                  0xFE,
                                  0);
  if (status != EMBER_ZCL_STATUS_SUCCESS) {
    return status;
  }

  pageChannelMask = 0;
  status = emberAfWriteAttribute(ZCL_SUB_GHZ_CLUSTER_ENDPOINT,
                                 ZCL_SUB_GHZ_CLUSTER_ID,
                                 ZCL_SUB_GHZ_CLUSTER_CHANNEL_CHANGE_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER,
                                 (uint8_t*)&pageChannelMask,
                                 ZCL_BITMAP32_ATTRIBUTE_TYPE);
  return status;
}

/** @brief Execute channel change with all the extra actions
 *
 * executeChannelChange() is never called bare, always in a context.
 * This helper simply provides the full context.
 */
static EmberAfPluginCommsHubFunctionSubGhzChannelChangeOutcome
executeChannelChangeAndResetStates(EmberAfPluginCommsHubFunctionSubGhzChannelChangeOutcome outcomeIfSuccess,
                                   EmberAfPluginCommsHubFunctionSubGhzChannelChangeOutcome outcomeIfFailed,
                                   EmberAfStatus *status)
{
  updateStateForAllMirrors(CHANNEL_CHANGE_STATE_IDLE, true);

  *status = executeChannelChange();
  return *status == EMBER_ZCL_STATUS_SUCCESS
         ? outcomeIfSuccess
         : outcomeIfFailed;
}

//----------------------------------------------------------------------------
// Public functions

/** @brief Trigger the sub-GHz channel change sequence
 *
 * Set the flags and start the timer. The rest is done in handlers and callbacks.
 */
EmberAfPluginCommsHubFunctionStatus emberAfCommsHubFunctionSubGhzStartChannelChangeSequence(void)
{
  EmberAfPluginCommsHubFunctionStatus status;
  uint8_t mirrorIndex;

  // Sanity check #1. Do not start the sequence if one is already in progress.
  if (!checkStateForAllMirrors(CHANNEL_CHANGE_STATE_IDLE)) {
    return EMBER_AF_CHF_STATUS_NO_ACCESS;
  }

  // Sanity check #2. At least one sub-GHz GSME must have allocated a mirror.
  // If not, proceed to the energy scan immediately.
  for (mirrorIndex = 0;
       mirrorIndex < sizeof channelChangeState / sizeof channelChangeState[0];
       mirrorIndex++) {
    const uint8_t mirrorEndpoint = mirrorIndex
                                   + EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
    EmberEUI64 mirrorEui = EMBER_NULL_EUI64;

    emberAfPluginCommsHubFunctionPrint("CHF: EP %d: ", mirrorEndpoint);

    if (emberAfPluginMeterMirrorIsMirrorUsed(mirrorEndpoint)
        && emberAfPluginMeterMirrorGetEui64ByEndpoint(mirrorEndpoint, mirrorEui)
        && isChildSubGhz(mirrorEui)) {
      emberAfPluginCommsHubFunctionPrintln("sub-GHz device");
      subGhzChildCount++;
    } else {
      emberAfPluginCommsHubFunctionPrintln("unallocated, unknown or not %p",
                                           "sub-GHz device");
      channelChangeState[mirrorIndex] = CHANNEL_CHANGE_STATE_IGNORED;
    }
  }

  if (subGhzChildCount == 0) {
    emberAfPluginCommsHubFunctionSubGhzChannelChangeCallback(EMBER_AF_CHF_SUB_GHZ_ENERGY_SCAN_ENABLED_NORMAL,
                                                             EMBER_ZCL_STATUS_SUCCESS);
    return EMBER_AF_CHF_STATUS_SUCCESS;
  }

  // Start the sequence by setting the EnergyScanPending Functional Notification
  // flag on each allocated mirror endpoint.
  status = updateFlagsAndStateForAllMirrors(EMBER_AF_METERING_FNF_ENERGY_SCAN_PENDING,
                                            CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_SET);
  if (status != EMBER_AF_CHF_STATUS_SUCCESS) {
    return status;
  }

  // Start the timer and proceed to step 2 in our sequence
  emberAfEventControlSetDelay(&emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventControl,
                              EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_SUB_GHZ_GSME_WAKE_UP_PERIOD
                              * MILLISECOND_TICKS_PER_MINUTE);
  return status;
}

/** @brief Complete the sub-GHz channel change sequence
 *
 * The application may call this function following an energy scan.
 * If both page and channel are 0, the sequence is terminated immediately and
 * the channel is not changed.
 */
EmberAfPluginCommsHubFunctionStatus emberAfCommsHubFunctionSubGhzCompleteChannelChangeSequence(uint32_t pageChannelMask)
{
  EmberAfPluginCommsHubFunctionStatus status;

  // Page and channel 0 terminate any ongoing sequence immediately
  if (pageChannelMask == 0) {
    emberAfPluginCommsHubFunctionPrintln("CHF: operation cancelled");
    updateStateForAllMirrors(CHANNEL_CHANGE_STATE_IDLE, true);
    emberEventControlSetInactive(emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventControl);
    return EMBER_AF_CHF_STATUS_SUCCESS;
  }

  // Sanity check #1. Are there any sub-GHz devices present?
  // If not, change the channel immediately.
  if (subGhzChildCount == 0) {
    EmberAfStatus status;
    const EmberAfPluginCommsHubFunctionSubGhzChannelChangeOutcome
      outcome = executeChannelChangeAndResetStates(EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_SUCCESS_NORMAL,
                                                   EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_FAIL_NORMAL,
                                                   &status);
    emberAfPluginCommsHubFunctionSubGhzChannelChangeCallback(outcome, status);
    return EMBER_AF_CHF_STATUS_SUCCESS;
  }

  // Sanity check #2. All GSMEs must have read the EnergyScanPending flag.
  if (!checkStateForAllMirrors(CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_READ)) {
    return EMBER_AF_CHF_STATUS_NO_ACCESS;
  }

  // Resume the sequence by setting the ChannelChangePending Functional Notification
  // flag on each allocated mirror endpoint.
  status = updateFlagsAndStateForAllMirrors(EMBER_AF_METERING_FNF_CHANNEL_CHANGE_PENDING,
                                            CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_PENDING_FLAG_AND_ATTRIBUTE_SET);
  if (status != EMBER_AF_CHF_STATUS_SUCCESS) {
    return status;
  }

  // Also set the ChannelChange attribute
  if (emberAfWriteAttribute(ZCL_SUB_GHZ_CLUSTER_ENDPOINT,
                            ZCL_SUB_GHZ_CLUSTER_ID,
                            ZCL_SUB_GHZ_CLUSTER_CHANNEL_CHANGE_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            (uint8_t*)&pageChannelMask,
                            ZCL_BITMAP32_ATTRIBUTE_TYPE) != EMBER_ZCL_STATUS_SUCCESS) {
    return EMBER_AF_CHF_STATUS_FNF_ATTR_FAILURE;
  }

  // Start the timer and proceed to step 5 in our sequence
  emberAfEventControlSetDelay(&emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventControl,
                              EMBER_AF_PLUGIN_COMMS_HUB_FUNCTION_SUB_GHZ_GSME_WAKE_UP_PERIOD
                              * 2       // GBCS says we need to wait for two wakeup periods
                              * MILLISECOND_TICKS_PER_MINUTE);
  return status;
}

//----------------------------------------------------------------------------
// Private functions

/** @brief An incoming ReadAttributes message handler
 *
 * Used to advance channelChangeState to the next state
 */
void emAfCommsHubFunctionSubGhzReadAttributeNotification(EmberNodeId source,
                                                         EmberAfClusterId clusterId,
                                                         uint16_t attrId)
{
  EmberEUI64 eui64;
  uint8_t mirrorEndpoint;

  if (clusterId == ZCL_SIMPLE_METERING_CLUSTER_ID
      && attrId == ZCL_FUNCTIONAL_NOTIFICATION_FLAGS_ATTRIBUTE_ID) {
    // A client is reading the Functional Notification flags
    // If it is one of our GSMEs, then:
    // 1. Note the flag has beenn read by this GSME and clear it
    // 2. Check if this is the last GSME we were waiting for
    // 3. If it was, advace to the next step
    if (emberLookupEui64ByNodeId(source, eui64) == EMBER_SUCCESS
        && emberAfPluginMeterMirrorGetEndpointByEui64(eui64, &mirrorEndpoint)) {
      emberAfPluginCommsHubFunctionPrintln("CHF: GSME 0x%2x reading Functional Notification flags",
                                           source);
      // There are two notification flags. Work out which one this is about.
      if (checkStateAndClearFlag(mirrorEndpoint,
                                 CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_SET,
                                 CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_READ,
                                 ~EMBER_AF_METERING_FNF_ENERGY_SCAN_PENDING)) {
        // All GSMEs have read the EnergyScanPending flag.
        // Notify the application that it can start the energy scan.
        emberAfEventControlSetDelay(&emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventControl,
                                    0); // offload the job to the timer
      } else if (checkStateAndClearFlag(mirrorEndpoint,
                                        CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_PENDING_FLAG_AND_ATTRIBUTE_SET,
                                        CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_PENDING_FLAG_READ,
                                        ~EMBER_AF_METERING_FNF_CHANNEL_CHANGE_PENDING)) {
        // All GSMEs have read the ChannelChangePending flag.
        // Nothing more to do here, but the GSMEs still need to read the ChannelChange attribute.
      } else {
        // MISRA no op
      }
    }
  } else if (clusterId == ZCL_SUB_GHZ_CLUSTER_ID
             && attrId == ZCL_SUB_GHZ_CLUSTER_CHANNEL_CHANGE_ATTRIBUTE_ID) {
    // A client is reading the ChannelChange attribute
    // If it is one of our GSMEs, then:
    // 1. Note the attribute has beenn read by this GSME
    // 2. Check if this is the last GSME we were waiting for
    // 3. If it was, clear the attribute and complete the sequence
    if (emberLookupEui64ByNodeId(source, eui64) == EMBER_SUCCESS
        && emberAfPluginMeterMirrorGetEndpointByEui64(eui64, &mirrorEndpoint)) {
      emberAfPluginCommsHubFunctionPrintln("CHF: GSME 0x%2x reading Sub-GHz Cluster attributes",
                                           source);
      const int8_t mirrorIndex = mirrorEndpoint
                                 - EMBER_AF_PLUGIN_METER_MIRROR_ENDPOINT_START;
      if (channelChangeState[mirrorIndex] == CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_PENDING_FLAG_READ) {
        channelChangeState[mirrorIndex] = CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_ATTRIBUTE_READ;
      }
      if (checkStateForAllMirrors(CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_ATTRIBUTE_READ)) {
        // All GSMEs have read the ChannelChange attribute.
        // Allow the GSME to go back to sleep and then complete the channel change.
        emberAfEventControlSetDelay(&emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventControl,
                                    30 * MILLISECOND_TICKS_PER_SECOND);
      }
    }
  } else {
    // MISRA no op
  }
}

/** @brief GSME timeout event handler
 *
 * Used as a fallback in case the GSME fails to read the notification flags.
 * We also use it in normal (non-time-out) cases, to off-load the execution
 * of certain operations from the callback context.
 */
void emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventHandler(void)
{
  EmberAfPluginCommsHubFunctionSubGhzChannelChangeOutcome outcome;
  EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

  emberEventControlSetInactive(emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventControl);

  emberAfPluginCommsHubFunctionPrintln("CHF: emberAfPluginCommsHubFunctionSubGhzGsmeTimeOutEventHandler()");

  if (checkStateForAllMirrors(CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_ATTRIBUTE_READ)) {
    // All GSMEs have read all the attributes.
    outcome = executeChannelChangeAndResetStates(EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_SUCCESS_NORMAL,
                                                 EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_FAIL_NORMAL,
                                                 &status);
  } else if (checkStateForAnyMirror(CHANNEL_CHANGE_STATE_CHANNEL_CHANGE_PENDING_FLAG_AND_ATTRIBUTE_SET)) {
    // Some GSMEs have not read the ChannelChangePending flag
    // or ChannelChange attribute yet.
    outcome = executeChannelChangeAndResetStates(EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_SUCCESS_TIMEOUT,
                                                 EMBER_AF_CHF_SUB_GHZ_CHANNEL_CHANGE_FAIL_TIMEOUT,
                                                 &status);
  } else if (checkStateForAllMirrors(CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_READ)) {
    // All GSMEs have read the EnergyScanPending flag;
    outcome = EMBER_AF_CHF_SUB_GHZ_ENERGY_SCAN_ENABLED_NORMAL;
  } else {
    // Some GSMEs have not read the EnergyScanPending flag yet.
    outcome = EMBER_AF_CHF_SUB_GHZ_ENERGY_SCAN_ENABLED_TIMEOUT;
    updateStateForAllMirrors(CHANNEL_CHANGE_STATE_ENERGY_SCAN_PENDING_FLAG_READ, false);
  }

  emberAfPluginCommsHubFunctionSubGhzChannelChangeCallback(outcome, status);
}
