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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// Copyright 2014 Silicon Laboratories, Inc.
//
//

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"

// Event control struct declarations
EmberEventControl button0EventControl;
EmberEventControl button1EventControl;
EmberEventControl identifyEventControl;

static uint8_t const happyTune[] = {
  NOTE_B4, 1,
  0, 1,
  NOTE_B5, 1,
  0, 0
};
static uint8_t const sadTune[] = {
  NOTE_B5, 1,
  0, 1,
  NOTE_B4, 5,
  0, 0
};

static bool holdingButton0 = false;
static bool holdingButton1 = false;

static uint32_t identifyDurationMs = 0; // milliseconds
#define DEFAULT_IDENTIFY_DURATION_MS (3 * MILLISECOND_TICKS_PER_SECOND)

// Repair declarations
#define BUTTON_PRESS_NETWORK_REPAIR_DELAY 10
#define MS_ALLOWED_FOR_REPAIR (MILLISECOND_TICKS_PER_SECOND << 1)
enum {
  REPAIR_SUCCESS,
  REPAIR_IN_PROGRESS,
  REPAIR_FAILED
};
typedef uint8_t RepairStatus;
static uint16_t buttonPressTime = 0;
RepairStatus repairConnection(EmberEventControl *eventControl);

void button0EventHandler(void)
{
  emberEventControlSetInactive(button0EventControl);

  // If the button is being held down > 1/4 second we perform hold
  // behavior, resetting to factory new.
  if (halButtonState(BUTTON0) == BUTTON_PRESSED) {
    emberAfCorePrintln("Resetting to factory new");
    emberAfZllResetToFactoryNew();
    halPlayTune_P(sadTune, true);
    holdingButton0 = true;

    // If the button HAD been held down, we don't really do anything for
    // this button, just reset the holding flag.
  } else if (holdingButton0) {
    holdingButton0 = false;

    // Otherwise, we react to a quick press and release by repairing the
    // connection and then sending a broadcast off command.
  } else {
    RepairStatus status = repairConnection(&button0EventControl);
    if (status == REPAIR_SUCCESS) {
      EmberStatus status;
      emberAfCorePrintln("Broadcasting \"off\" command");
      emberAfFillCommandOnOffClusterOff();
      emberAfSetCommandEndpoints(emberAfEndpointFromIndex(0),
                                 EMBER_BROADCAST_ENDPOINT);
      status = emberAfSendCommandBroadcast(EMBER_SLEEPY_BROADCAST_ADDRESS);
      if (status != EMBER_SUCCESS) {
        emberAfCorePrintln("ERR: Broadcasting \"off\" command failed 0x%x", status);
      }
    }
  }
}

void button1EventHandler(void)
{
  emberEventControlSetInactive(button1EventControl);

  // If the button is pressed when this event fires, we know it
  // was held down for > 1/4 second and perform hold behavior, initiating
  // a touch link.
  if (halButtonState(BUTTON1) == BUTTON_PRESSED) {
    emberAfCorePrintln("Initiating touch link");
    emberAfZllInitiateTouchLink();
    holdingButton1 = true;

    // If the button HAD been held down, we know it was released following a hold
    // If the touchlink is still in progress, we abort.
  } else if (holdingButton1) {
    if (emberAfZllTouchLinkInProgress()) {
      emberAfCorePrintln("Aborting touch link");
      emberAfZllAbortTouchLink();
    }
    holdingButton1 = false;

    // Otherwise, we know this was a quick press and release inside 1/4 second.
    // So we repair the connection and send an on broadcast.
  } else {
    RepairStatus status = repairConnection(&button1EventControl);
    if (status == REPAIR_SUCCESS) {
      EmberStatus status;
      emberAfCorePrintln("Broadcasting \"on\" command");
      emberAfFillCommandOnOffClusterOn();
      emberAfSetCommandEndpoints(emberAfEndpointFromIndex(0),
                                 EMBER_BROADCAST_ENDPOINT);
      status = emberAfSendCommandBroadcast(EMBER_SLEEPY_BROADCAST_ADDRESS);
      if (status != EMBER_SUCCESS) {
        emberAfCorePrintln("ERR: Broadcasting \"on\" command failed 0x%x", status);
      }
    }
  }
}

void identifyEventHandler(void)
{
  if (identifyDurationMs == 0) {
    halClearLed(BOARDLED1);
    emberEventControlSetInactive(identifyEventControl);
  } else {
    halToggleLed(BOARDLED1);
    if (identifyDurationMs >= MILLISECOND_TICKS_PER_QUARTERSECOND) {
      identifyDurationMs -= MILLISECOND_TICKS_PER_QUARTERSECOND;
    } else {
      identifyDurationMs = 0;
    }
    emberEventControlSetDelayMS(identifyEventControl,
                                MILLISECOND_TICKS_PER_QUARTERSECOND);
  }
}

// This isr is called twice, once when the button is pressed
// and again when it is released. When the button is pressed, a button
// event is scheduled for 1/4 second in the future. If the button is
// still pressed when the event fires, we know it was held down.
// When the button is released, the event is scheduled immediately, if it HAD
// been held down we can abort the hold behavior, otherwise react to a quick
// press and release.
void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
  EmberEventControl *event = (button == BUTTON0
                              ? &button0EventControl
                              : &button1EventControl);
  buttonPressTime = halCommonGetInt16uMillisecondTick();
  if (state == BUTTON_PRESSED) {
    emberEventControlSetDelayMS(*event, MILLISECOND_TICKS_PER_QUARTERSECOND);
  } else {
    emberEventControlSetActive(*event);
  }
}

extern bool tuneDone; // declared in buzzer.c

/** @brief Ok To Sleep
 *
 * This function is called by the Idle/Sleep plugin before sleeping.  The
 * application should return true if the device may sleep or false otherwise.
 *
 * @param durationMs The maximum duration in milliseconds that the device will
 * sleep.  Ver.: always
 */
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  return tuneDone;
}

/** @brief Wake Up
 *
 * This function is called by the Idle/Sleep plugin after sleeping.
 *
 * @param durationMs The duration in milliseconds that the device slept.
 * Ver.: always
 */
void emberAfPluginIdleSleepWakeUpCallback(uint32_t durationMs)
{
}

/** @brief Ok To Idle
 *
 * This function is called by the Idle/Sleep plugin before idling.  The
 * application should return true if the device may idle or false otherwise.
 *
 */
bool emberAfPluginIdleSleepOkToIdleCallback(void)
{
  return tuneDone;
}

/** @brief Active
 *
 * This function is called by the Idle/Sleep plugin after idling.
 *
 */
void emberAfPluginIdleSleepActiveCallback(void)
{
}

/** @brief Initial Security State
 *
 * This function is called by the ZLL Commissioning plugin to determine the
 * initial security state to be used by the device.  The application must
 * populate the ::EmberZllInitialSecurityState structure with a configuration
 * appropriate for the network being formed, joined, or started.  Once the
 * device forms, joins, or starts a network, the same security configuration
 * will remain in place until the device leaves the network.
 *
 * @param securityState The security configuration to be populated by the
 * application and ultimately set in the stack.  Ver.: always
 */
void emberAfPluginZllCommissioningInitialSecurityStateCallback(EmberZllInitialSecurityState *securityState)
{
  // By default, the plugin will configure the stack for the certification
  // encryption algorithm.  Devices that are certified should instead use the
  // master encryption algorithm and set the appropriate encryption key and
  // pre-configured link key.
}

/** @brief Touch Link Complete
 *
 * This function is called by the ZLL Commissioning plugin when touch linking
 * completes.
 *
 * @param networkInfo The ZigBee and ZLL-specific information about the
 * network and target.  Ver.: always
 * @param deviceInformationRecordCount The number of sub-device information
 * records for the target.  Ver.: always
 * @param deviceInformationRecordList The list of sub-device information
 * records for the target.  Ver.: always
 */
void emberAfPluginZllCommissioningTouchLinkCompleteCallback(const EmberZllNetwork *networkInfo,
                                                            uint8_t deviceInformationRecordCount,
                                                            const EmberZllDeviceInfoRecord *deviceInformationRecordList)
{
  uint8_t i;
  emberAfCorePrint("Touch link with 0x%2x (", networkInfo->nodeId);
  emberAfCoreDebugExec(emberAfPrintBigEndianEui64(networkInfo->eui64));
  emberAfCorePrintln(") complete");
  emberAfCoreFlush();
  for (i = 0; i < deviceInformationRecordCount; i++) {
    emberAfCorePrintln("sub device %d: 0x%x 0x%2x 0x%2x 0x%x 0x%x",
                       i,
                       deviceInformationRecordList[i].endpointId,
                       deviceInformationRecordList[i].profileId,
                       deviceInformationRecordList[i].deviceId,
                       deviceInformationRecordList[i].version,
                       deviceInformationRecordList[i].groupIdCount);
    emberAfCoreFlush();
  }
  halPlayTune_P(happyTune, true);
}

/** @brief Touch Link Failed
 *
 * This function is called by the ZLL Commissioning plugin if touch linking
 * fails.
 *
 * @param status The reason the touch link failed.  Ver.: always
 */
void emberAfPluginZllCommissioningTouchLinkFailedCallback(EmberAfZllCommissioningStatus status)
{
  emberAfCorePrintln("Touch link failed: 0x%x", status);
  halPlayTune_P(sadTune, true);
}

/** @brief Group Identifier Count
 *
 * This function is called by the ZLL Commissioning plugin to determine the
 * number of group identifiers in use by a specific endpoint on the device.
 * The total number of group identifiers on the device, which are shared by
 * all endpoints, is defined by ::EMBER_ZLL_GROUP_ADDRESSES.
 *
 * @param endpoint The endpoint for which the group identifier count is
 * requested.  Ver.: always
 */
uint8_t emberAfPluginZllCommissioningGroupIdentifierCountCallback(uint8_t endpoint)
{
  // Devices with multiple endpoints will have to distribute the group ids
  // available to the device among the individual endpoints.  This application
  // has one endpoint, so it uses all available group ids.
  return EMBER_ZLL_GROUP_ADDRESSES;
}

/** @brief Group Identifier
 *
 * This function is called by the ZLL Commissioning plugin to obtain
 * information about the group identifiers in use by a specific endpoint on
 * the device.  The application should populate the record with information
 * about the group identifier and return true.  If no information is available
 * for the given endpoint and index, the application should return false.
 *
 * @param endpoint The endpoint for which the group identifier is requested.
 * Ver.: always
 * @param index The index of the group on the endpoint.  Ver.: always
 * @param record The group information record.  Ver.: always
 */
bool emberAfPluginZllCommissioningGroupIdentifierCallback(uint8_t endpoint,
                                                          uint8_t index,
                                                          EmberAfPluginZllCommissioningGroupInformationRecord *record)
{
  // Devices with multiple endpoints will have to distribute the group ids
  // available to the device among the individual endpoints.  This application
  // has one endpoint, so it uses all available group ids in order.
  EmberTokTypeStackZllData token;
  emberZllGetTokenStackZllData(&token);
  record->groupId = token.myGroupIdMin + index;
  record->groupType = 0x00; // fixed at zero
  return true;
}

/** @brief Endpoint Information Count
 *
 * This function is called by the ZLL Commissioning plugin to determine the
 * number of remote endpoints controlled by a specific endpoint on the local
 * device.
 *
 * @param endpoint The local endpoint for which the remote endpoint
 * information count is requested.  Ver.: always
 */
uint8_t emberAfPluginZllCommissioningEndpointInformationCountCallback(uint8_t endpoint)
{
  // Devices that control remote endpoints individually will have to maintain
  // a list of those endpoints.  This application does not control remote
  // endpoints individually.
  return 0x00;
}

/** @brief Endpoint Information
 *
 * This function is called by the ZLL Commissioning plugin to obtain
 * information about the remote endpoints controlled by a specific endpoint on
 * the local device.  The application should populate the record with
 * information about the remote endpoint and return true.  If no information
 * is available for the given endpoint and index, the application should
 * return false.
 *
 * @param endpoint The local endpoint for which the remote endpoint
 * information is requested.  Ver.: always
 * @param index The index of the remote endpoint information on the local
 * endpoint.  Ver.: always
 * @param record The endpoint information record.  Ver.: always
 */
bool emberAfPluginZllCommissioningEndpointInformationCallback(uint8_t endpoint,
                                                              uint8_t index,
                                                              EmberAfPluginZllCommissioningEndpointInformationRecord *record)
{
  // Devices that control remote endpoints individually will have to maintain
  // a list of those endpoints.  This application does not control remote
  // endpoints individually.
  return false;
}

/** @brief Identify
 *
 * This function is called by the ZLL Commissioning plugin to notify the
 * application that it should take an action to identify itself.  This
 * typically occurs when an Identify Request is received via inter-PAN
 * messaging.
 *
 * @param durationS If the duration is zero, the device should exit identify
 * mode.  If the duration is 0xFFFF, the device should remain in identify mode
 * for the default time.  Otherwise, the duration specifies the length of time
 * in seconds that the device should remain in identify mode.  Ver.: always
 */
void emberAfPluginZllCommissioningIdentifyCallback(uint16_t durationS)
{
  if (durationS != 0) {
    halStackIndicatePresence();
  }
  identifyDurationMs = (durationS == 0xFFFF
                        ? DEFAULT_IDENTIFY_DURATION_MS
                        : durationS * MILLISECOND_TICKS_PER_SECOND);
  emberEventControlSetActive(identifyEventControl);
}

/** @brief Reset To Factory New
 *
 * This function is called by the ZLL Commissioning plugin when a request to
 * reset to factory new is received.  The plugin will leave the network, reset
 * attributes managed by the framework to their default values, and clear the
 * group and scene tables.  The application should perform any other necessary
 * reset-related operations in this callback, including resetting any
 * externally-stored attributes.
 *
 */
void emberAfPluginZllCommissioningResetToFactoryNewCallback(void)
{
}

/** @brief Join
 *
 * This callback is called by the ZLL Commissioning plugin when a joinable
 * network has been found.  If the application returns true, the plugin will
 * attempt to join the network.  Otherwise, the plugin will ignore the network
 * and continue searching.  Applications can use this callback to implement a
 * network blacklist.  Note that this callback is not called during touch
 * linking.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginZllCommissioningJoinCallback(EmberZigbeeNetwork *networkFound,
                                               uint8_t lqi,
                                               int8_t rssi)
{
  return true;
}

/** @brief repairConnection
 * This function does its best to repair the network connection. This
 * function will reschedule the associated event
 * until either the connection is repaired or MS_ALLOWED_FOR_REPAIR
 * is up.
 *
 * @return RepairStatus, one of REPAIR_SUCCESS, REPAIR_IN_PROGRESS,
 * REPAIR_FAILED
 */
RepairStatus repairConnection(EmberEventControl *eventControl)
{
  uint16_t currentTime = halCommonGetInt16uMillisecondTick();
  uint16_t elapsedTime = elapsedTimeInt16u(buttonPressTime, currentTime);
  EmberNetworkStatus status = emberAfNetworkState();
  RepairStatus repairStatus = REPAIR_FAILED;

  switch (status) {
    case EMBER_NO_NETWORK:
      repairStatus = REPAIR_FAILED;
      break;
    case EMBER_JOINED_NETWORK:
      repairStatus = REPAIR_SUCCESS;
      break;
    default:
    {
      if (elapsedTime > MS_ALLOWED_FOR_REPAIR) {
        repairStatus = REPAIR_FAILED;
      } else {
        if ((emberAfNetworkState() == EMBER_JOINED_NETWORK_NO_PARENT)
            && (!emberStackIsPerformingRejoin())) {
          emberFindAndRejoinNetwork(true, // secure rejoin
                                    0);   // current channel
        }
        emberEventControlSetDelayMS(*eventControl, BUTTON_PRESS_NETWORK_REPAIR_DELAY);
        repairStatus = REPAIR_IN_PROGRESS;
      }
    }
    break;
  }

  // DEBUG PRINT
  if (repairStatus == REPAIR_FAILED) {
    emberAfCorePrintln("repair failed! %x", emberAfNetworkState());
    halPlayTune_P(sadTune, false);
  } else if (repairStatus == REPAIR_SUCCESS) {
    emberAfCorePrintln("ms to repair %d ms",
                       elapsedTime);
  } else {
    emberAfCorePrintln("repair in progress...");
  }

  return repairStatus;
}

/** @brief Poll Completed
 *
 * This function is called by the application framework after a poll is
 * completed.
 *
 * @param status Return status of a completed poll operation  Ver.: always
 */
void emberAfPluginEndDeviceSupportPollCompletedCallback(EmberStatus status)
{
}
