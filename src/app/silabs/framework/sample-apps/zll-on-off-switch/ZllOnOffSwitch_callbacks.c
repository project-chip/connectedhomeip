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

static uint8_t PGM happyTune[] = {
  NOTE_B4, 1,
  0, 1,
  NOTE_B5, 1,
  0, 0
};
static uint8_t PGM sadTune[] = {
  NOTE_B5, 1,
  0, 1,
  NOTE_B4, 5,
  0, 0
};

static bool holdingButton0 = false;
static bool holdingButton1 = false;

static uint32_t identifyDurationMs = 0; // milliseconds
#define DEFAULT_IDENTIFY_DURATION_MS (3 * MILLISECOND_TICKS_PER_SECOND)

void button0EventHandler(void)
{
  emberAfCorePrintln("Button 0 pressed!");
  if (halButtonState(BUTTON0) == BUTTON_PRESSED) {
    emberAfCorePrintln("Resetting to factory new");
    emberAfZllResetToFactoryNew();
    halPlayTune_P(sadTune, true);
    holdingButton0 = true;
  } else if (holdingButton0) {
    holdingButton0 = false;
  } else {
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
  emberEventControlSetInactive(button0EventControl);
}

void button1EventHandler(void)
{
  emberAfCorePrintln("Button 1 pressed!");
  if (halButtonState(BUTTON1) == BUTTON_PRESSED) {
    emberAfCorePrintln("Initiating touch link");
    emberAfZllInitiateTouchLink();
    holdingButton1 = true;
  } else if (holdingButton1) {
    if (emberAfZllTouchLinkInProgress()) {
      emberAfCorePrintln("Aborting touch link");
      emberAfZllAbortTouchLink();
    }
    holdingButton1 = false;
  } else {
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
  emberEventControlSetInactive(button1EventControl);
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

void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
  EmberEventControl *event = (button == BUTTON0
                              ? &button0EventControl
                              : &button1EventControl);
  if (state == BUTTON_PRESSED) {
    emberEventControlSetDelayMS(*event, MILLISECOND_TICKS_PER_QUARTERSECOND);
  } else {
    emberEventControlSetActive(*event);
  }
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
