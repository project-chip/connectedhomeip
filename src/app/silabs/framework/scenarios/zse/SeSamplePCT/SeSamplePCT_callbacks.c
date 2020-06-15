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

// This callback file is created for your convenience. You may add application code
// to this file. If you regenerate this file over a previous version, the previous
// version will be overwritten and any code you have added will be lost.

#include "app/framework/include/af.h"
#include "app/framework/plugin/esi-management/esi-management.h"
#include "app/framework/util/af-main.h"

//------------------------------------------------------------------------------
// Globals

// Event control struct declaration
EmberEventControl priceQueryControl;

#define MY_ENDPOINT 1

// Assume we have at least one index in the ESI Table but we only care about
// a single ESI.
#define ESI_INDEX  0

#define PRICE_QUERY_DELAY_MINUTES 1

//------------------------------------------------------------------------------
// Forward Declarations

//------------------------------------------------------------------------------

void priceQuery(void)
{
  // Send a get current price query

  EmberAfPluginEsiManagementEsiEntry* esi = emberAfPluginEsiManagementEsiLookUpByIndex(ESI_INDEX);
  if (esi == NULL) {
    emberAfCorePrintln("Error: No ESI's exist in table that can be queried for Price!");
  } else {
    EmberNodeType nodeType;
    EmberNetworkParameters parameters;
    EmberStatus status = emberAfGetNetworkParameters(&nodeType,
                                                     &parameters);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error: Could not determine node type.");
    } else {
      // The parameter to this macro is 'command options'.
      // This is defined in the spec with only 1 bit, indicating
      // whether the receiver is an RxOnWhenIdle=true device.
      emberAfFillCommandPriceClusterGetCurrentPrice(nodeType
                                                    != EMBER_SLEEPY_END_DEVICE);
      emberAfSetCommandEndpoints(MY_ENDPOINT, esi->endpoint);
      if (status != emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                              esi->nodeId)) {
        emberAfCorePrintln("Failed to send price query.");
      }
    }
  }
  emberEventControlSetDelayMinutes(priceQueryControl, PRICE_QUERY_DELAY_MINUTES);
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be
 * notified of changes to the stack status and take appropriate action.  The
 * return code from this callback is ignored by the framework.  The framework
 * will always process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  return false;
}

/** @brief Registration
 *
 * This callback is called when the device joins a network and the process of
 * registration is complete. This callback provides a success value of true if
 * the registration process was successful and a value of false if
 * registration failed.
 *
 * @param success true if registration succeeded, false otherwise.  Ver.:
 * always
 */
void emberAfRegistrationCallback(bool success)
{
  priceQuery();
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
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork * networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
  return true;
}

/** @brief Event Action
 *
 * This function is called by the demand response and load control client
 * plugin whenever an event status changes within the DRLC event table.  The
 * list of possible event status values is defined by the ZCL spec and is
 * listed in the Application Framework's generated enums located in enums.h.
 * For example, an event status may be:
 * AMI_EVENT_STATUS_LOAD_CONTROL_EVENT_COMMAND_RX indicating that a properly
 * formatted event was received; AMI_EVENT_STATUS_EVENT_STARTED indicating
 * that an event has started; AMI_EVENT_STATUS_THE_EVENT_HAS_BEEN_CANCELED,
 * indicating that the event was canceled.  This callback is intended to give
 * the device an opportunity to take action on the event in question.  For
 * instance if an event starts, the device should take the appropriate event
 * action on the hardware.  This callback returns a bool, if returned value
 * is true, then a notification will be send over the air automatically to the
 * originator of the event.  If it is false, then nothing will be sent back to
 * the originator of the event.  Please note that in order for your
 * application to be ZigBee compliant, a notification must be sent over the
 * air to the originator of the event, so a value of false should only be
 * returned if your application code takes care of sending this message or
 * there is some other reason a message does not need to be sent by the
 * framework.
 *
 * @param loadControlEvent Actual event  Ver.: always
 * @param eventStatus Status of event  Ver.: always
 * @param sequenceNumber Sequence number  Ver.: always
 */
bool emberAfPluginDrlcEventActionCallback(EmberAfLoadControlEvent * loadControlEvent,
                                          EmberAfAmiEventStatus eventStatus,
                                          uint8_t sequenceNumber)
{
  return true;
}

/** @brief Price Started
 *
 * This function is called by the Price client plugin whenever a price starts.
 *
 * @param price The price that has started.  Ver.: always
 */
void emberAfPluginPriceClientPriceStartedCallback(EmberAfPluginPriceClientPrice * price)
{
  // Assume US dollars, kwh, and less than $1 price.
  // In theory we could decode that from the price entry.
  emberAfCorePrint("Current Price: $0.%d/kwh. Duration:", price->price);
  emberAfCorePrintln((price->durationInMinutes == 0xFFFF
                      ? " forever"
                      : " %d minutes"),
                     price->durationInMinutes);
}

/** @brief Price Expired
 *
 * This function is called by the Price client plugin whenever a price
 * expires.
 *
 * @param price The price that has expired.  Ver.: always
 */
void emberAfPluginPriceClientPriceExpiredCallback(EmberAfPluginPriceClientPrice * price)
{
  emberAfCorePrintln("Price expired.");
}

/** @brief Button Event
 *
 * This allows another module to get notification when a button is pressed and
 * the button joining plugin does not handle it.  For example, if the device
 * is already joined to the network and button 0 is pressed.  Button 0
 * normally forms or joins a network.  This callback is NOT called in ISR
 * context so there are no restrictions on what code can execute.
 *
 * @param buttonNumber The button number that was pressed.  Ver.: always
 * @param buttonPressDurationMs The number of milliseconds the button was pressed.
 */
void emberAfPluginButtonJoiningButtonEventCallback(uint8_t buttonNumber,
                                                   uint32_t buttonPressDurationMs)
{
}
