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
//
// This file is generated. Please do not edit manually.
//
//

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"
#include "hal/hal.h"
#include "app/framework/plugin/ezmode-commissioning/ez-mode.h"

static bool foundLight = false;
static bool lookingForLight = false;
static EmberNodeId lightId = EMBER_NULL_NODE_ID;

static void lightDiscoveryCallback(const EmberAfServiceDiscoveryResult* result)
{
  uint8_t i;

  if (emberAfHaveDiscoveryResponseStatus(result->status)) {
    const EmberAfEndpointList* epList
      = (const EmberAfEndpointList*)result->responseData;
    emberAfCorePrintln("found light: 0x%2x", result->matchAddress);
    for (i = 0; i < epList->count; i++) {
      emberAfCorePrintln("  ept: 0x%x", epList->list[i]);
    }
    // store information of the first light we found
    if (!foundLight) {
      // We must copy the data locally because the HAL routine wants
      // a non-const pointer.
      EmberNodeId nodeId = result->matchAddress;

      // we store the values in token for SoC platform and store the values
      // in RAM for host platform.
#ifndef EZSP_HOST
      halCommonSetToken(TOKEN_HALIGHT_NODE, &nodeId);
#else
      lightId = nodeId;
#endif // EZSP_HOST
      foundLight = true;
    }
  } else if (result->status == EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE) {
    emberAfCorePrintln("This is all the lights found.");
    lookingForLight = false;
  }
}

/** @brief Main Tick
 *
 * Whenever main application tick is called, this callback will be called at
 * the end of the main tick execution.
 *
 */
void emberAfMainTickCallback(void)
{
  EmberStatus status;

#ifndef EZSP_HOST
  halCommonGetToken(&lightId, TOKEN_HALIGHT_NODE);
#endif

  // if we are not part of the network, clear the tokens associated with
  // HALight; if not already done so.
  if (emberAfNetworkState() != EMBER_JOINED_NETWORK
      && !emberStackIsPerformingRejoin()) {
    if (lightId != EMBER_NULL_NODE_ID) {
      emberAfCorePrintln("Clearing known light ID.");
      lightId = EMBER_NULL_NODE_ID;
      foundLight = false;
#ifndef EZSP_HOST
      halCommonSetToken(TOKEN_HALIGHT_NODE, &lightId);
#endif
    }
  } else {
    // if we are on the network, discover the light if we have not done it
    // already.
    if (lightId == EMBER_NULL_NODE_ID && !foundLight && !lookingForLight) {
      emberAfCorePrintln("Find a light");
      status
        = emberAfFindDevicesByProfileAndCluster(
        EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
        HA_PROFILE_ID,
        ZCL_ON_OFF_CLUSTER_ID,
        EMBER_AF_SERVER_CLUSTER_DISCOVERY,
        lightDiscoveryCallback);
      if (status != EMBER_SUCCESS) { // server cluster
        emberAfDebugPrintln("Cannot find light, error 0x%x", status);
      } else {
        // the bool is used so we do not keep sending match descriptor
        // request everytime mainTick is call while waiting for match
        // descriptor response.
        lookingForLight = true;
      }
    } // check lightId value
  }
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

/** @brief Select File Descriptors
 *
 * This function is called when the Gateway plugin will do a select() call to
 * yield the processor until it has a timed event that needs to execute.  The
 * function implementor may add additional file descriptors that the
 * application will monitor with select() for data ready.  These file
 * descriptors must be read file descriptors.  The number of file descriptors
 * added must be returned by the function (0 for none added).
 *
 * @param list A pointer to a list of File descriptors that the function
 * implementor may append to  Ver.: always
 * @param maxSize The maximum number of elements that the function implementor
 * may add.  Ver.: always
 */
int emberAfPluginGatewaySelectFileDescriptorsCallback(int* list,
                                                      int maxSize)
{
  return 0;
}

/** @brief Button Event
 *
 * This allows another module to get notification when a button is pressed and
 * released but the button joining plugin did not handle it.  This callback is
 * NOT called in ISR context so there are no restrictions on what code can
 * execute.
 *
 * @param buttonNumber The button number that was pressed.  Ver.: always
 * @param buttonPressDurationMs The length of time button was held down before
 * it was released.  Ver.: always
 */
void emberAfPluginButtonJoiningButtonEventCallback(uint8_t buttonNumber,
                                                   uint32_t buttonPressDurationMs)
{
  // We assume the first endpoint (0) is the one to use for end-device bind / EZ-Mode
  uint8_t endpoint = emberAfEndpointFromIndex(0);
  emberAfCorePrintln("EZ-Mode Commission:%x", endpoint);
  emberAfEzmodeServerCommission(endpoint);
}

/**
 * @brief A callback invoked when a network status message
 * is received that informs the application of the over-the-air
 * error codes for the specific destination.
 *
 * Note: Network analyzer may flag this message as "route error" which
 * is the old name for the "network status" command.
 *
 * This handler is a superset of emberIncomingRouteErrorHandler.
 * The old API was only invoking the handler for a couple of the possible
 * error codes and these were being translated into EmberStatus.
 *
 * To make the API more generic and extensible in future, the new
 * API emberIncomingNetworkStatusHandler simply copies the OTA
 * error code as is. If the application includes this callback,
 * it must define @c EMBER_APPLICATION_HAS_INCOMING_NETWORK_STATUS_HANDLER
 * in its configuration header.
 * @param errorCode  uint8_t
 * @param target  The short ID of the remote node.
 */
void emberIncomingNetworkStatusHandler(uint8_t errorCode,
                                       EmberNodeId target)
{
  emberAfPushCallbackNetworkIndex();
  emberAfCorePrintln("ErrorCode : 0x%1X, Target : 0x%2X ", errorCode, target);
  emberAfPopNetworkIndex();
}
