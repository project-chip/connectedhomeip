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

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/green-power-common/green-power-common.h"

#include EMBER_AF_API_NETWORK_CREATOR
#include EMBER_AF_API_NETWORK_CREATOR_SECURITY
#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_FIND_AND_BIND_TARGET
#include EMBER_AF_API_ZLL_PROFILE

#define LIGHT_ENDPOINT (1)

EmberEventControl commissioningLedEventControl;
EmberEventControl findingAndBindingEventControl;

void commissioningLedEventHandler(void)
{
  emberEventControlSetInactive(commissioningLedEventControl);

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    uint16_t identifyTime;
    emberAfReadServerAttribute(LIGHT_ENDPOINT,
                               ZCL_IDENTIFY_CLUSTER_ID,
                               ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                               (uint8_t *)&identifyTime,
                               sizeof(identifyTime));
    if (identifyTime > 0) {
      halToggleLed(COMMISSIONING_STATUS_LED);
      emberEventControlSetDelayMS(commissioningLedEventControl,
                                  LED_BLINK_PERIOD_MS << 1);
    } else {
      halSetLed(COMMISSIONING_STATUS_LED);
    }
  } else {
    EmberStatus status = emberAfPluginNetworkSteeringStart();
    emberAfCorePrintln("%p network %p: 0x%X", "Join", "start", status);
  }
}

void findingAndBindingEventHandler()
{
  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
    emberEventControlSetInactive(findingAndBindingEventControl);
    emberAfCorePrintln("Find and bind target start: 0x%X",
                       emberAfPluginFindAndBindTargetStart(LIGHT_ENDPOINT));
  }
}

static void setZllState(uint16_t clear, uint16_t set)
{
  EmberTokTypeStackZllData token;
  emberZllGetTokenStackZllData(&token);
  token.bitmask &= ~clear;
  token.bitmask |= set;
  emberZllSetTokenStackZllData(&token);
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  // Make sure to change the ZLL factory new state based on whether or not
  // we are on a network.
  if (status == EMBER_NETWORK_DOWN) {
    halClearLed(COMMISSIONING_STATUS_LED);
    setZllState(0, (EMBER_ZLL_STATE_FACTORY_NEW | EMBER_ZLL_STATE_PROFILE_INTEROP));
  } else if (status == EMBER_NETWORK_UP) {
    halSetLed(COMMISSIONING_STATUS_LED);
    setZllState(EMBER_ZLL_STATE_FACTORY_NEW, EMBER_ZLL_STATE_PROFILE_INTEROP);

    emberEventControlSetActive(findingAndBindingEventControl);
  }

  // This value is ignored by the framework.
  return false;
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */
void emberAfMainInitCallback(void)
{
  emberEventControlSetActive(commissioningLedEventControl);
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);

  if (status != EMBER_SUCCESS) {
    // Initialize our ZLL security now so that we are ready to be a touchlink
    // target at any point.
    status = emberAfZllSetInitialSecurityState();
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error: cannot initialize ZLL security: 0x%X", status);
    }

    status = emberAfPluginNetworkCreatorStart(false); // distributed
    emberAfCorePrintln("%p network %p: 0x%X", "Form", "start", status);
  }
}

/** @brief Complete
 *
 * This callback notifies the user that the network creation process has
 * completed successfully.
 *
 * @param network The network that the network creator plugin successfully
 * formed. Ver.: always
 * @param usedSecondaryChannels Whether or not the network creator wants to
 * form a network on the secondary channels Ver.: always
 */
void emberAfPluginNetworkCreatorCompleteCallback(const EmberNetworkParameters *network,
                                                 bool usedSecondaryChannels)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Form distributed",
                     "complete",
                     EMBER_SUCCESS);
}

/** @brief Server Init
 *
 * On/off cluster, Server Init
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOnOffClusterServerPostInitCallback(uint8_t endpoint)
{
  // At startup, trigger a read of the attribute and possibly a toggle of the
  // LED to make sure they are always in sync.
  emberAfOnOffClusterServerAttributeChangedCallback(endpoint,
                                                    ZCL_ON_OFF_ATTRIBUTE_ID);
}

/** @brief Server Attribute Changed
 *
 * On/off cluster, Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(uint8_t endpoint,
                                                       EmberAfAttributeId attributeId)
{
  // When the on/off attribute changes, set the LED appropriately.  If an error
  // occurs, ignore it because there's really nothing we can do.
  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    bool onOff;
    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_ON_OFF_CLUSTER_ID,
                                   ZCL_ON_OFF_ATTRIBUTE_ID,
                                   (uint8_t *)&onOff,
                                   sizeof(onOff))
        == EMBER_ZCL_STATUS_SUCCESS) {
      if (onOff) {
        halSetLed(ON_OFF_LIGHT_LED);
      } else {
        halClearLed(ON_OFF_LIGHT_LED);
      }
    }
  }
}

/** @brief Hal Button Isr
 *
 * This callback is called by the framework whenever a button is pressed on the
 * device. This callback is called within ISR context.
 *
 * @param button The button which has changed state, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.  Ver.: always
 * @param state The new state of the button referenced by the button parameter,
 * either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED
 * if the button has been released.  Ver.: always
 */
static uint8_t comm_state;

void emberAfHalButtonIsrCallback(uint8_t button, uint8_t state)
{
  if (state == BUTTON_PRESSED) {
    if (button == BUTTON0) {
      if (comm_state == 0) {
        // comm enter start
        emberAfGreenPowerClusterGpSinkCommissioningModeCallback(0x09,    //options - (Involve Proxy | Enter)
                                                                0xFFFF,  //addr
                                                                0xFFFF,  //addr
                                                                LIGHT_ENDPOINT); //light Endpoint
        comm_state = 1;
        halSetLed(BOARDLED1);
      } else {
        // comm enter stop
        emberAfGreenPowerClusterGpSinkCommissioningModeCallback(0x08,    //options - (Involve Proxy | Exit)
                                                                0xFFFF,  //addr
                                                                0xFFFF,  //addr
                                                                LIGHT_ENDPOINT); //light endpoint
        comm_state = 0;
        halClearLed(BOARDLED1);
      }
    } else if (button == BUTTON1) {
      // Find and binf Target start
      emberEventControlSetActive(findingAndBindingEventControl);
    }
  }
}

/** @brief Gpd Commissioning
 *
 * This function is called by the Green Power Server upon expiry of either the switch commissioning
 * or multisensor commissioning timer.
 *
 * @param appInfo   Ver.: always
 */
void emberAfGreenPowerServerCommissioningTimeoutCallback(uint8_t commissioningTimeoutType,
                                                         uint8_t numberOfEndpoints,
                                                         uint8_t * endpoints)
{
  // Commissioning exit, clear the state and LED indication.
  comm_state = 0;
  halClearLed(BOARDLED1);
}

/** @brief Gpd Commissioning
 *
 * This function is called by the Green Power Server upon the completion of the pairing
 * and to indicate the closure of the pairing session.
 *
 * @param appInfo   Ver.: always
 */
void emberAfGreenPowerServerPairingCompleteCallback(uint8_t numberOfEndpoints,
                                                    uint8_t * endpoints)
{
  emberAfCorePrint("%p : No of Eps = %x EPs[",
                   __FUNCTION__,
                   numberOfEndpoints);
  emberAfCorePrintBuffer(endpoints, numberOfEndpoints, true);
  emberAfCorePrintln("]");
  halClearLed(BOARDLED1);
}
