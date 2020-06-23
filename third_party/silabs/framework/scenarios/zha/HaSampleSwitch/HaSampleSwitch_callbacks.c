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

#include "app/framework/include/af.h"
#include "app/framework/plugin/ezmode-commissioning/ez-mode.h"

// Event control struct declarations
EmberEventControl buttonEventControl;

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
static uint8_t const waitTune[] = {
  NOTE_B4, 1,
  0, 0
};

#define REPAIR_DELAY_MS 10
#define REPAIR_LIMIT_MS (MILLISECOND_TICKS_PER_SECOND << 1)

static bool on = false;
static uint16_t buttonPressTime;
static uint16_t clusterIds[] = { ZCL_ON_OFF_CLUSTER_ID };

extern bool tuneDone;

void buttonEventHandler(void)
{
  emberEventControlSetInactive(buttonEventControl);
  if (halButtonState(BUTTON0) == BUTTON_PRESSED) {
    EmberNetworkStatus state = emberAfNetworkState();
    if (state == EMBER_JOINED_NETWORK) {
      emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                                 | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
                                ZCL_ON_OFF_CLUSTER_ID,
                                (on ? ZCL_OFF_COMMAND_ID : ZCL_ON_COMMAND_ID),
                                "");
      emberAfGetCommandApsFrame()->profileId           = emberAfPrimaryProfileId();
      emberAfGetCommandApsFrame()->sourceEndpoint      = emberAfPrimaryEndpoint();
      emberAfGetCommandApsFrame()->destinationEndpoint = EMBER_BROADCAST_ENDPOINT;
      if (emberAfSendCommandUnicastToBindings() == EMBER_SUCCESS) {
        on = !on;
      } else {
        halPlayTune_P(sadTune, true);
      }
    } else if (state == EMBER_NO_NETWORK) {
      halPlayTune_P((emberAfStartSearchForJoinableNetwork() == EMBER_SUCCESS
                     ? waitTune
                     : sadTune),
                    true);
    } else {
      if (REPAIR_LIMIT_MS
          < elapsedTimeInt16u(buttonPressTime,
                              halCommonGetInt16uMillisecondTick())) {
        halPlayTune_P(sadTune, true);
      } else {
        if (state == EMBER_JOINED_NETWORK_NO_PARENT
            && !emberStackIsPerformingRejoin()) {
          halPlayTune_P((emberFindAndRejoinNetwork(true, 0) == EMBER_SUCCESS
                         ? waitTune
                         : sadTune),
                        true);
        }
        emberEventControlSetDelayMS(buttonEventControl, REPAIR_DELAY_MS);
      }
    }
  } else if (halButtonState(BUTTON1) == BUTTON_PRESSED) {
    emberAfEzmodeClientCommission(emberAfPrimaryEndpoint(),
                                  EMBER_AF_EZMODE_COMMISSIONING_CLIENT_TO_SERVER,
                                  clusterIds,
                                  COUNTOF(clusterIds));
  }
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be
 * notified of changes to the stack status and take appropriate action.  The
 * application should return true if the status has been handled and should
 * not be handled by the application framework.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_UP) {
    halPlayTune_P(happyTune, true);
  } else if (status == EMBER_NETWORK_DOWN) {
    halPlayTune_P(sadTune, true);
  }
  return false;
}

/** @brief emberAfHalButtonIsrCallback
 *
 *
 */
// Hal Button ISR Callback
// This callback is called by the framework whenever a button is pressed on the
// device. This callback is called within ISR context.
void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
  if ((button == BUTTON0 || button == BUTTON1)
      && state == BUTTON_PRESSED
      && !emberEventControlGetActive(buttonEventControl)) {
    buttonPressTime = halCommonGetInt16uMillisecondTick();
    emberEventControlSetActive(buttonEventControl);
  }
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

/** @brief Client Complete
 *
 * This function is called by the EZ-Mode Commissioning plugin when client
 * commissioning completes.
 *
 * @param bindingIndex The binding index that was created or
 * ::EMBER_NULL_BINDING if an error occurred.  Ver.: always
 */
void emberAfPluginEzmodeCommissioningClientCompleteCallback(uint8_t bindingIndex)
{
}

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

/** @brief Finished
 *
 * This callback is fired when the network-find plugin is finished with the
 * forming or joining process.  The result of the operation will be returned
 * in the status parameter.
 *
 * @param status   Ver.: always
 */
void emberAfPluginNetworkFindFinishedCallback(EmberStatus status)
{
}

/** @brief Get Radio Power For Channel
 *
 * This callback is called by the framework when it is setting the radio power
 * during the discovery process. The framework will set the radio power
 * depending on what is returned by this callback.
 *
 * @param channel   Ver.: always
 */
int8_t emberAfPluginNetworkFindGetRadioPowerForChannelCallback(uint8_t channel)
{
  return EMBER_AF_PLUGIN_NETWORK_FIND_RADIO_TX_POWER;
}

/** @brief Join
 *
 * This callback is called by the plugin when a joinable network has been
 * found.  If the application returns true, the plugin will attempt to join
 * the network.  Otherwise, the plugin will ignore the network and continue
 * searching.  Applications can use this callback to implement a network
 * blacklist.
 *
 * @param networkFound   Ver.: always
 * @param lqi   Ver.: always
 * @param rssi   Ver.: always
 */
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork *networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}
