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
#include "app/util/common/form-and-join.h"
#include "app/framework/plugin/ezmode-commissioning/ez-mode.h"

#define LED BOARDLED1
#define PJOIN_DURATION_S 60

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

static void pjoin(void)
{
  if (emberPermitJoining(PJOIN_DURATION_S) == EMBER_SUCCESS) {
    halPlayTune_P(happyTune, true);
  } else {
    halPlayTune_P(sadTune, true);
  }
}

void buttonEventHandler(void)
{
  // On a press-and-hold button event, form a network if we don't have one or
  // permit joining if we do.  If we form, we'll automatically permit joining
  // when we come up.
  emberEventControlSetInactive(buttonEventControl);
  if (halButtonState(BUTTON0) == BUTTON_PRESSED) {
    if (emberAfNetworkState() == EMBER_NO_NETWORK) {
      halPlayTune_P(((emberFormAndJoinIsScanning()
                      || emberAfFindUnusedPanIdAndForm() == EMBER_SUCCESS)
                     ? waitTune
                     : sadTune),
                    true);
    } else {
      pjoin();
    }
  } else if (halButtonState(BUTTON1) == BUTTON_PRESSED) {
    emberAfEzmodeServerCommission(emberAfPrimaryEndpoint());
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
  // Whenever the network comes up, permit joining.  If we go down, let the
  // user know, although this shouldn't happen.
  if (status == EMBER_NETWORK_UP) {
    pjoin();
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
  if ((button == BUTTON0 || button == BUTTON1) && state == BUTTON_PRESSED) {
    emberEventControlSetActive(buttonEventControl);
  }
}

/** @brief Broadcast Sent
 *
 * This function is called when a new MTORR broadcast has been successfully
 * sent by the concentrator plugin.
 *
 */
void emberAfPluginConcentratorBroadcastSentCallback(void)
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

/** @brief Configured
 *
 * This callback is called by the Reporting plugin whenever a reporting entry
 * is configured, including when entries are deleted or updated.  The
 * application can use this callback for scheduling readings or measurements
 * based on the minimum and maximum reporting interval for the entry.  The
 * application should return EMBER_ZCL_STATUS_SUCCESS if it can support the
 * configuration or an error status otherwise.  Note: attribute reporting is
 * required for many clusters and attributes, so rejecting a reporting
 * configuration may violate ZigBee specifications.
 *
 * @param entry   Ver.: always
 */
EmberAfStatus emberAfPluginReportingConfiguredCallback(const EmberAfPluginReportingEntry * entry)
{
  return EMBER_ZCL_STATUS_SUCCESS;
}
