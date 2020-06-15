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
#include "app/framework/util/af-main.h" // emberAfGetBindingTableSize
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "app/framework/plugin/dmp-ui-demo/dmp-ui.h"

#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_ZLL_PROFILE
#include EMBER_AF_API_FIND_AND_BIND_INITIATOR

#define MAX_NUM_LIGHTS_IN_NWK               2
#define SWITCH_ENDPOINT                     (1)
#define LOCAL_DEVICE_TABLE_FULL             0xFF
#define LOCAL_DEVICE_TABLE_ENTRY_NOT_FOUND  0xFE
#define MAX_ALLOWED_FAILED_ATTEMPTS         10
#define SERVICE_DISCOVERY_TIMEOUT_MS        (7000)
#define INVALID_RESPONSE                    (EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED | EMBER_AF_UNICAST_SERVICE_DISCOVERY_TIMEOUT \
                                             | EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE | EMBER_AF_BROADCAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE)

enum {
  INITIAL_STATE,
  INITIATE_PRIMARY_DISCOVERY,
  INITIATE_SECOND_DISCOVERY,
  WAITING_FOR_RESPONSE
};

typedef struct {
  EmberEUI64 eui;
  EmberNodeId nodeId;
  uint8_t numberOfFailedMatchDesc;
  uint8_t bindingTableIndex;
  bool isRemoteBindingDone;
}localDeviceTable_t;

// Custom event stubs. Custom events will be run along with all other events in
// the application framework. They should be managed using the Ember Event API
// documented in stack/include/events.h

// Event control struct declarations
EmberEventControl commissioningEventControl;
EmberEventControl buttonEventControl;
EmberEventControl lcdPermitJoinEventControl;
EmberEventControl lcdMainMenuDisplayEventControl;
static int buttonPressed = -1;

static localDeviceTable_t localDeviceTable[MAX_NUM_LIGHTS_IN_NWK];
static bool intendDeassociate = false;
static EmberAfServiceDiscoveryStatus responseRecvd;
static bool discoveryComplete = false;
static bool startDiscovery = false;

// Event function forward declarations
void commissioningEventHandler(void);
void buttonEventHandler(void);
void lcdPermitJoinEventHandler(void);
void lcdMainMenuDisplayEventHandler(void);

/**
 * @brief
 *   Clears the requested index from the local device table.
 *
 * @param[in] uint8_t, index of the local device table to be cleared.
 *
 * @return
 *      void
 */
static void localDeviceTableClearIndex(uint8_t index)
{
  if (index != 0xFF) {
    memset(&localDeviceTable[index], 0xFF, sizeof(localDeviceTable_t));
    localDeviceTable[index].isRemoteBindingDone = false;
    localDeviceTable[index].numberOfFailedMatchDesc = 0;
    localDeviceTable[index].bindingTableIndex = 0xFF;
  }
}

/**
 * @brief
 *   Checks if the given index in the local device table has a valid entry.
 *
 * @param[in] uint8_t, index of the local device table to check.
 *
 * @return
 *      bool, true if the index is valid.
 */
static bool localTableIsValidIndex(uint8_t index)
{
  EmberEUI64 tempEui = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  bool returnVal = false;
  if (index < MAX_NUM_LIGHTS_IN_NWK) {
    returnVal = (memcmp(localDeviceTable[index].eui, tempEui, EUI64_SIZE) != 0);
  }

  return returnVal;
}

/**
 * @brief
 *   Returns the number of lights detected in the network.
 *
 * @param[in] void
 *
 * @return
 *      uint8_t, Number of lights in the network.
 */
static uint8_t localDeviceTableGetNumofLightDiscovered(void)
{
  uint8_t lightsOnNwk = 0;
  for (int i = 0; i < MAX_NUM_LIGHTS_IN_NWK; i++) {
    if (localTableIsValidIndex(i)) {
      lightsOnNwk++;
    }
  }
  return lightsOnNwk;
}

/**
 * @brief
 *   Clears the local device table.
 *
 * @param[in] void
 *
 * @return
 *      void
 */
static void localDeviceTableClear(void)
{
  for (int i = 0; i < MAX_NUM_LIGHTS_IN_NWK; i++) {
    localDeviceTableClearIndex(i);
  }
}

/**
 * @brief
 *   Returns the next available free entry in the local device table.
 *
 * @param[in] void
 *
 * @return
 *      uint8_t, index of the next available free entry or LOCAL_DEVICE_TABLE_FULL.
 */
static uint8_t localDeviceTableGetNextFreeEntry(void)
{
  uint8_t returnVal = LOCAL_DEVICE_TABLE_FULL;
  for (uint8_t i = 0; i < MAX_NUM_LIGHTS_IN_NWK; i++) {
    if (!localTableIsValidIndex(i)) {
      returnVal = i;
      break;
    }
  }
  return returnVal;
}

/**
 * @brief
 *   Looks for a device entry in local device table based on its EUI.
 *
 * @param[in] EmberEUI64, EUI64 of the device that needs to be found.
 *
 * @return
 *      uint8_t, index of the device with specified EUI64 or LOCAL_DEVICE_TABLE_ENTRY_NOT_FOUND.
 */
static uint8_t localDeviceTableLookUpByEui(EmberEUI64 eui)
{
  uint8_t returnVal = LOCAL_DEVICE_TABLE_ENTRY_NOT_FOUND;
  for (uint8_t i = 0; i < MAX_NUM_LIGHTS_IN_NWK; i++) {
    if (!memcmp(eui, &localDeviceTable[i].eui, EUI64_SIZE)) {
      returnVal = i;
      break;
    }
  }
  return returnVal;
}

/**
 * @brief
 *   updates the local device table based on the binding table entries.
 *
 * @param[in] void
 *
 * @return
 *      void
 */
static void localDeviceTableUpdate(void)
{
  uint8_t bindingTableSize = emberAfGetBindingTableSize();
  uint8_t numofLightsDisccovered;
  EmberBindingTableEntry bindingEntry;

  for (int i = 0; i < bindingTableSize; i++) {
    if (emberGetBinding(i, &bindingEntry) == EMBER_SUCCESS) {
      if (localDeviceTableLookUpByEui(bindingEntry.identifier) == LOCAL_DEVICE_TABLE_ENTRY_NOT_FOUND) {
        if ((bindingEntry.type == EMBER_UNICAST_BINDING)
            && (bindingEntry.clusterId == ZCL_ON_OFF_CLUSTER_ID)) {
          uint8_t freeEntry = localDeviceTableGetNextFreeEntry();
          EmberNodeId destinationNodeId = emberLookupNodeIdByEui64(bindingEntry.identifier);

          if (freeEntry != LOCAL_DEVICE_TABLE_FULL) {
            memcpy(&localDeviceTable[freeEntry], bindingEntry.identifier, EUI64_SIZE);
            localDeviceTable[freeEntry].nodeId = destinationNodeId;
            localDeviceTable[freeEntry].bindingTableIndex = i;
          }
        }
      }
    }
  }

  numofLightsDisccovered = localDeviceTableGetNumofLightDiscovered();
  dmpUiSetNumLightsDiscovered(numofLightsDisccovered);
}

/**
 * @brief
 *   Initiate find and Bind on the switch.
 *
 * @param[in] void
 *
 * @return
 *      void
 */
static void startFindingAndBindingForInitiator(void)
{
  dmpUiDisplayZigBeeState(DMP_UI_DISCOVERING);
  EmberStatus status = emberAfPluginFindAndBindInitiatorStart(SWITCH_ENDPOINT);
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "start", status);
}

/**
 * @brief
 *   Looks for a device entry in binding table based on its EUI.
 *
 * @param[in] EmberEUI64, EUI64 of the device that needs to be found.
 *
 * @return
 *      uint8_t, binding table index of the device if found, 0xFF otherwise.
 */
static uint8_t getBindingTableEntry(EmberEUI64 eui64)
{
  uint8_t bindingTableIndex;

  for (bindingTableIndex = 0; bindingTableIndex < EMBER_BINDING_TABLE_SIZE; bindingTableIndex++) {
    EmberBindingTableEntry entry;
    if (emberGetBinding(bindingTableIndex, &entry) == EMBER_SUCCESS) {
      if ((entry.type == EMBER_UNICAST_BINDING)
          && (entry.clusterId == ZCL_ON_OFF_CLUSTER_ID)
          && (MEMCOMPARE(entry.identifier, eui64, EUI64_SIZE) == 0)) {
        break;
      }
    }
  }
  return ((bindingTableIndex == EMBER_BINDING_TABLE_SIZE) ? 0xFF : bindingTableIndex);
}

/**
 * @brief
 *   Callback function for device discoveries.
 *
 * @param[in] EmberAfServiceDiscoveryResult *, pointer to the discovery result.
 *
 * @return
 *      void
 */
void deviceDiscoveredCb(const EmberAfServiceDiscoveryResult* result)
{
  responseRecvd = result->status;

  if (responseRecvd != EMBER_AF_BROADCAST_SERVICE_DISCOVERY_RESPONSE_RECEIVED) {
    discoveryComplete = true;
  }
}

/**
 * @brief
 *   State machine to run the periodic discovery process.
 *
 * @param[in] void
 *
 * @return
 *      void
 */
static void runDiscoveryStateMachine(void)
{
  static int state = INITIAL_STATE;
  static uint8_t index = 0;
  static uint16_t discoveryResponseWaitStartTime = 0;
  bool sendMatchDesc = false;

  switch (state) {
    case INITIAL_STATE:
    {
      // Default state
      if (startDiscovery) {
        // If discovery is triggered in the commissioning process, change the state to
        // INITIATE_PRIMARY_DISCOVERY.
        state = INITIATE_PRIMARY_DISCOVERY;
        startDiscovery = false;
      }
    }
    break;

    case INITIATE_PRIMARY_DISCOVERY:
    {
      // state to send a unicast match descriptor for index 0
      // in the local device table.
      index = 0;

      if (localTableIsValidIndex(0)) {
        // If there is a vlid entry in index 0 of local device table,
        // make sendMatchDesc as true, change the state to WAITING_FOR_RESPONSE,
        // make the discoveryComplete variable as false.
        sendMatchDesc = true;
        state = WAITING_FOR_RESPONSE;
        discoveryComplete = false;
      } else {
        // If no valid device is found in index 0, try index 1.
        state = INITIATE_SECOND_DISCOVERY;
      }
    }
    break;

    case INITIATE_SECOND_DISCOVERY:
    {
      // state to send a unicast match descriptor for index 1
      // in the local device table.
      index = 1;

      if (localTableIsValidIndex(1)) {
        // If there is a vlid entry in index 1 of local device table,
        // make sendMatchDesc as true, change the state to WAITING_FOR_RESPONSE,
        // make the discoveryComplete variable as false.
        sendMatchDesc = true;
        state = WAITING_FOR_RESPONSE;
        discoveryComplete = false;
      } else {
        // If no valid device is found in index 1, switch to default state.
        state = INITIAL_STATE;
      }
    }
    break;

    case WAITING_FOR_RESPONSE:
    {
      // This state tracks the discovery process and updates the local device table and the
      // binding table according to the unicast discovery results.
      //
      // The status of the discovery itself can be deduced by the state of discoveryComplete and
      // responseRecvd variables. This state also monitors the time taken for the discovery process
      // itself. The discovery timeout value is set to 5 seconds for this project (check
      // EMBER_AF_DISCOVERY_TIMEOUT_QS in the project defines). If the statemachine does not receive
      // a callback within SERVICE_DISCOVERY_TIMEOUT_MS, which is 7 seconds, it will treat the discovery
      // attempt to have failed.

      bool discoveryFailed = true;
      uint16_t timeTakenForDiscovery = halCommonGetInt16uMillisecondTick() - discoveryResponseWaitStartTime;

      // If the discovery was complete or if we have timed-out...
      if (discoveryComplete || (timeTakenForDiscovery > SERVICE_DISCOVERY_TIMEOUT_MS)) {
        emberAfCustom1Println("discoveryComplete = %s discoveryResponseWaitStartTime = %d timeTakenForDiscovery: %d",
                              (discoveryComplete ? "True" : "False"), discoveryResponseWaitStartTime, timeTakenForDiscovery);
        discoveryComplete = false;
        // change the state according tto the index being processed, if the index processed is 0, do one more discovery
        // for index1, else go to default state.
        state = ((index == 0) ? INITIATE_SECOND_DISCOVERY : INITIAL_STATE);

        if (responseRecvd == EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE) {
          // If we have received the response, mark the discovery as successful.
          discoveryFailed = false;
        }

        if (discoveryFailed) {
          // If the discovery failed, increment the failure counter.
          localDeviceTable[index].numberOfFailedMatchDesc++;
          emberAfCustom1Println("!!!!No response for Match Desc, Index = %d Number of attempts: %d!!!!", index, localDeviceTable[index].numberOfFailedMatchDesc);
        } else {
          // If the discovery succeeded, reset the failure counter.
          localDeviceTable[index].numberOfFailedMatchDesc = 0;
        }
      }
    }
    break;
  }

  if (sendMatchDesc) {
    // If we need to send out a unicast discovery...
    emberAfCustom1Println("************* Starting Discovery for index %d *****************", index);
    emberAfCustom1Println("index = %x, nodeId = 0x%2x, EUI = 0x%x%x%x%x%x%x%x%x",
                          index, localDeviceTable[index].nodeId, localDeviceTable[index].eui[7], localDeviceTable[index].eui[6],
                          localDeviceTable[index].eui[5], localDeviceTable[index].eui[4], localDeviceTable[index].eui[3],
                          localDeviceTable[index].eui[2], localDeviceTable[index].eui[1], localDeviceTable[index].eui[0]);

    // Check if we have valid nodeId in the local device table..
    if (localDeviceTable[index].nodeId < 0xFFF8) {
      // If yes, note the time when the discovery process started and send out the request.
      responseRecvd = INVALID_RESPONSE;
      discoveryResponseWaitStartTime = halCommonGetInt16uMillisecondTick();
      emberAfFindDevicesByProfileAndCluster(localDeviceTable[index].nodeId,
                                            HA_PROFILE_ID,
                                            ZCL_ON_OFF_CLUSTER_ID,
                                            EMBER_AF_SERVER_CLUSTER_DISCOVERY,
                                            deviceDiscoveredCb);
    } else {
      // If we have a invalid nodeId, mark this attempt as failure and try to lookup the nodeId andsend a network address request.
      emberAfCustom1Println("Invalid NodeId!! looking up nodeId using Eui64", index);
      localDeviceTable[index].nodeId = emberLookupNodeIdByEui64(localDeviceTable[index].eui);
      localDeviceTable[index].numberOfFailedMatchDesc++;

      if (localDeviceTable[index].nodeId < 0xFFF8) {
        emberNetworkAddressRequest(localDeviceTable[index].eui,
                                   false,            // report kids?
                                   0);               // child start index
      }
      // change the state according to the index being processed.
      state = ((index == 0) ? INITIATE_SECOND_DISCOVERY : INITIAL_STATE);
    }
  }

  if (localDeviceTable[index].numberOfFailedMatchDesc >= MAX_ALLOWED_FAILED_ATTEMPTS) {
    uint8_t numofLightsDisccovered = localDeviceTableGetNumofLightDiscovered();
    // If we have met the failure threshold for the given index..
    emberAfCustom1Println("Clearing local device table and bindings!!");
    // delete the binding table entry..
    emberDeleteBinding(localDeviceTable[index].bindingTableIndex);
    // clear the local device table entry..
    localDeviceTableClearIndex(index);
    dmpUiSetNumLightsDiscovered(numofLightsDisccovered);
    dmpUiClrLcdDisplayMainScreen();
  }
}

// Event function
void commissioningEventHandler(void)
{
  EmberStatus status;
  EmberNetworkStatus state = emberAfNetworkState();
  emberEventControlSetInactive(commissioningEventControl);

  switch (state) {
    case EMBER_NO_NETWORK:
    {
      // If we dont have a network try to join a network
      EmberEUI64 wildcardEui64 = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
      EmberKeyData centralizedKey = { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 };
      // Add Centralized link key to transient key table. This is not necessary for Z3 devices as the stack will retry with the correct key.
      // But doing this ensures that the association succeeds at the first attempt and avoids retries
      emberAddTransientLinkKey(wildcardEui64, &centralizedKey);
      dmpUiDisplayZigBeeState(DMP_UI_JOINING);
      status = emberAfPluginNetworkSteeringStart();
      break;
    }

    case EMBER_JOINED_NETWORK_NO_PARENT:
    {
      status = emberFindAndRejoinNetwork(true, 0);
      break;
    }

    case EMBER_JOINING_NETWORK:
    case EMBER_LEAVING_NETWORK:
    {
      break;
    }

    case EMBER_JOINED_NETWORK:
    {
      static int oneMinuteTracker = 6;
      uint8_t numofLightsDisccovered = 0;

      if (intendDeassociate == true) {
        // If we intend to disassociate, leave the network and clear binding table
        status = emberLeaveNetwork();
        emberClearBindingTable();
      } else {
        // If it is a periodic event, try to update the local device table.
        localDeviceTableUpdate();

        numofLightsDisccovered = localDeviceTableGetNumofLightDiscovered();
        dmpUiSetNumLightsDiscovered(numofLightsDisccovered);

        if (numofLightsDisccovered >= 2) {
          // If we dont find a free entry in the local device table, update the LCD to use two lights.

          dmpUiLightUpdateLight(DMP_UI_LIGHT_UNCHANGED, DMP_UI_LIGHT_UNCHANGED);
        } else {
          // If we find a free entry in the local device table, we have not found the second light,
          oneMinuteTracker--;

          if (oneMinuteTracker <= 0) {
            // try to initiate find and bind every 60 seconds.
            oneMinuteTracker = 6;
            startFindingAndBindingForInitiator();
          }
        }

        // trigger the periodiv discovery process
        startDiscovery = true;

        dmpUiClrLcdDisplayMainScreen();

        // re-trigger the periodic event.
        emberEventControlSetDelayMS(commissioningEventControl, 10000);
      }
      break;
    }
  }
}

void buttonEventHandler(void)
{
  EmberStatus status;
  uint16_t bindingTableIndex = 0, dmpLightIndex = 0xFF, dmpSleepyLightIndex = 0xFF;
  emberEventControlSetInactive(buttonEventControl);
  // Make a note of the current network state.
  EmberNetworkStatus state = emberAfNetworkState();
  if (state == EMBER_JOINED_NETWORK) {
    // If we are on the network, prepare a external buffer to send toggle command.
    emberAfFillExternalBuffer((ZCL_CLUSTER_SPECIFIC_COMMAND
                               | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
                              ZCL_ON_OFF_CLUSTER_ID,
                              ZCL_TOGGLE_COMMAND_ID,
                              "");
    emberAfGetCommandApsFrame()->profileId           = emberAfProfileIdFromIndex(0);
    emberAfGetCommandApsFrame()->sourceEndpoint      = emberAfEndpointFromIndex(0);

    for (int i = 0; i < emberAfGetBindingTableSize(); i++) {
      uint16_t remoteNodeId = emberGetBindingRemoteNodeId(i);
      emberAfCustom1Println("NodeId at index %d : 0x%2x", bindingTableIndex, remoteNodeId);
      // Find the index for sleepy light and non-sleepy light (always with nodeId 0x0000 as
      // a non-sleepy light is the co-ordinator of our demo).
      if (remoteNodeId == 0) {
        dmpLightIndex = i;
      } else if (remoteNodeId < 0xFFF8) {
        dmpSleepyLightIndex = i;
      }

      if (dmpLightIndex != 0xFF
          && dmpSleepyLightIndex != 0xFF) {
        // break out of for loop once we find one sleepy and one non-sleepy indices.
        break;
      }
    }

    // update the bindingTableIndex, based on the switch pressed.
    // If BUTTON1 is pressed control non-sleepy light, else control sleepy light.
    if (buttonPressed == BUTTON1) {
      bindingTableIndex = dmpLightIndex;
    } else {
      bindingTableIndex = dmpSleepyLightIndex;
    }

    // Send the toggle command to the binding table entry in index, bindingTableIndex.
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_VIA_BINDING, bindingTableIndex);
    emberAfCustom1Println("Sent toggle via binding to index %d : 0x%X", bindingTableIndex, status);

    if (status != EMBER_SUCCESS) {
      // If the send fails, try to send it to all bindings.
      status = emberAfSendCommandUnicastToBindings();
      emberAfCustom1Println("Sent toggle via binding to all indices : 0x%X", status);
    }

    if (status != EMBER_SUCCESS) {
      // If that fails too, try to control non-sleepy light.
      emberAfCustom1Println("Sent toggle via binding: 0x%X", status);
      emberAfGetCommandApsFrame()->destinationEndpoint = 0x01;
      status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, 0); // It is assumed that this switch is working with DMPLight demo
      emberAfCustom1Println("Sent toggle via ucast 0x%X", status);
    }
  }
}

void lcdMainMenuDisplayEventHandler(void)
{
  emberEventControlSetInactive(lcdMainMenuDisplayEventControl);
  dmpUiLightOff();
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
void emberAfHalButtonIsrCallback(int8u button,
                                 int8u state)
{
  static uint16_t button0PressTime;
  static uint16_t button1PressTime;
  uint16_t currentTime = 0;
  // by default dont trigger any events.
  bool triggerButtonEvent = false;
  bool triggerCommissioningEvent = false;
  // make a note of the network state.
  EmberNetworkStatus nwkState = emberAfNetworkState();

  switch (button) {
    case BUTTON0:
    {
      // when the button is pressed..
      if (state == BUTTON_PRESSED) {
        // make a note of when the button was pressed.
        button0PressTime = halCommonGetInt16uMillisecondTick();
      } else {
        // when released..
        if (nwkState == EMBER_NO_NETWORK) {
          // if we are not part of any network, trigger commissioning event.
          triggerCommissioningEvent = true;
        } else {
          currentTime = halCommonGetInt16uMillisecondTick();

          // If the button was pressed for more than 3 seconds (long press)..
          if ((currentTime - button0PressTime) > 3000) {
            for (int i = 0; i < 2; i++) {
              if (localTableIsValidIndex(i)) {
                // make sure to mark the isRemoteBindingDone as false
                // this is needed as the remote binding is set based
                // on this flag.
                localDeviceTable[i].isRemoteBindingDone = false;
              }
            }
            // start find and bind initiator.
            startFindingAndBindingForInitiator();
          } else if (!emberEventControlGetActive(buttonEventControl)) {
            // On BUTTON0 short press, trigger button event.
            triggerButtonEvent = true;
          }
        }
      }
    }
    break;

    case BUTTON1:
    {
      // when the button is pressed..
      if (state == BUTTON_PRESSED) {
        // make a note of when the button was pressed.
        button1PressTime = halCommonGetInt16uMillisecondTick();
      } else {
        // when released..
        if (nwkState == EMBER_NO_NETWORK) {
          // if we are not part of any network, trigger commissioning event.
          triggerCommissioningEvent = true;
        } else {
          currentTime = halCommonGetInt16uMillisecondTick();

          // If the button was pressed for more than 3 seconds (long press)..
          if ((currentTime - button1PressTime) > 3000) {
            // On BUTTON1 long press, set intendDeassociate and trigger commissioning event.
            // This forces the device out of the network.
            intendDeassociate = true;
            triggerCommissioningEvent = true;
          } else if (!emberEventControlGetActive(buttonEventControl)) {
            // On BUTTON1 short press, trigger button event.
            triggerButtonEvent = true;
          }
        }
      }
      break;
    }
  }

  if (triggerButtonEvent) {
    buttonPressed = button;
    emberEventControlSetActive(buttonEventControl);
  }

  if (triggerCommissioningEvent) {
    emberEventControlSetActive(commissioningEventControl);
  }
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
boolean emberAfStackStatusCallback(EmberStatus status)
{
  emberAfCorePrintln("emberAfStackStatusCallback");
  switch (status) {
    case EMBER_NETWORK_UP:
    {
      // Start periodic event to track the devices,
      emberEventControlSetDelayMS(commissioningEventControl, 10000);
      emberAfCorePrintln("Setting commissioning Event!!!");
      // update local device table,
      localDeviceTableUpdate();
      // try to find new lights
      startFindingAndBindingForInitiator();
      // When network is up, change the network state on LCD,
      dmpUiDisplayZigBeeState(DMP_UI_NETWORK_UP);
    }
    break;

    case EMBER_NETWORK_DOWN:
      // When network is up, switch the LCD display to single bulb.
      dmpUiLightOff();
      dmpUiSetNumLightsDiscovered(0);
      dmpUiClrLcdDisplayMainScreen();
      // When network is up, display network state as No Nwk on LCD,
      dmpUiDisplayZigBeeState(DMP_UI_NO_NETWORK);
      // Reset the global variable,
      intendDeassociate = false;
      // clear the local device table,
      localDeviceTableClear();

      // when the switch leaves the network, it need to clear the binding table
      // it will broadcast leave command, the DMP light will remove the binding
      // for the switch in its local binding.
      if (emberNetworkState() == EMBER_NO_NETWORK) {
        // Clear binding table.
        emberClearBindingTable();
      }
      break;
  }

  return false;
}

/** @brief Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 *
 * @param status Status code describing the completion of the find and bind
 * process Ver.: always
 */
void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
{
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "complete", status);
  dmpUiDisplayZigBeeState(DMP_UI_NETWORK_UP);

// if the switch has successfully found a light and set local binding entry for sending Onoff command
// it then need to set binding on the light to enable the reporting feature on the light

  if (status == EMBER_SUCCESS) {
    EmberBindingTableEntry bindingEntry;
    uint8_t bindingTableSize = emberAfGetBindingTableSize();
    uint8_t i;

    for (i = 0; i < bindingTableSize; i++) {
      if (emberGetBinding(i, &bindingEntry) == EMBER_SUCCESS) {
        if ((bindingEntry.type == EMBER_UNICAST_BINDING)
            && (bindingEntry.clusterId == ZCL_ON_OFF_CLUSTER_ID)) {
          // If a ON OFF device is found in the binding table..
          if (localDeviceTableLookUpByEui(bindingEntry.identifier) == LOCAL_DEVICE_TABLE_ENTRY_NOT_FOUND) {
            // If there is no entry for this device in the local device table..
            uint8_t freeEntry = localDeviceTableGetNextFreeEntry();
            EmberNodeId destinationNodeId = emberLookupNodeIdByEui64(bindingEntry.identifier);

            if (freeEntry != LOCAL_DEVICE_TABLE_FULL) {
              // If there is free entry for this device in the local device table..
              // create a new entry.
              memcpy(&localDeviceTable[freeEntry], bindingEntry.identifier, EUI64_SIZE);
              localDeviceTable[freeEntry].nodeId = destinationNodeId;

              if (!localDeviceTable[freeEntry].isRemoteBindingDone) {
                // Set the remote binding fo rthis device.
                localDeviceTable[freeEntry].bindingTableIndex = i;
                localDeviceTable[freeEntry].isRemoteBindingDone = true;
                emberBindRequest(destinationNodeId,
                                 bindingEntry.identifier,
                                 bindingEntry.remote,
                                 ZCL_ON_OFF_CLUSTER_ID,
                                 UNICAST_BINDING,
                                 emberGetEui64(),
                                 0,     // multicast group identifier - ignored
                                 bindingEntry.local,
                                 EMBER_AF_DEFAULT_APS_OPTIONS);
                emberEventControlSetDelayMS(commissioningEventControl, 2000);
              }// if (!localDeviceTable[freeEntry].isRemoteBindingDone)
            }// if(freeEntry != LOCAL_DEVICE_TABLE_FULL)
          }// if(localDeviceTableLookUpByEui(bindingEntry.identifier) == LOCAL_DEVICE_TABLE_ENTRY_NOT_FOUND)
        }// if ((bindingEntry.type == EMBER_UNICAST_BINDING) && (bindingEntry.clusterId == ZCL_ON_OFF_CLUSTER_ID))
      }// if (emberGetBinding(i, &bindingEntry) == EMBER_SUCCESS)
    }// for (i = 0; i < bindingTableSize; i++)
  }// if (status == EMBER_SUCCESS)
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
    dmpUiDisplayZigBeeState(DMP_UI_NO_NETWORK);
  } else {
    startFindingAndBindingForInitiator();
  }
}

/** @brief Touch Link Complete
 *
 * This function is called by the ZLL Commissioning plugin when touch linking
 * completes.
 *
 * @param networkInfo The ZigBee and ZLL-specific information about the network
 * and target. Ver.: always
 * @param deviceInformationRecordCount The number of sub-device information
 * records for the target. Ver.: always
 * @param deviceInformationRecordList The list of sub-device information
 * records for the target. Ver.: always
 */
void emberAfPluginZllCommissioningTouchLinkCompleteCallback(const EmberZllNetwork *networkInfo,
                                                            uint8_t deviceInformationRecordCount,
                                                            const EmberZllDeviceInfoRecord *deviceInformationRecordList)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Touchlink",
                     "complete",
                     EMBER_SUCCESS);

  startFindingAndBindingForInitiator();
}

/** @brief Touch Link Failed
 *
 * This function is called by the ZLL Commissioning plugin if touch linking
 * fails.
 *
 * @param status The reason the touch link failed. Ver.: always
 */
void emberAfPluginZllCommissioningTouchLinkFailedCallback(EmberAfZllCommissioningStatus status)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Touchlink",
                     "complete",
                     EMBER_ERR_FATAL);
}

/** @brief Main Tick
 *
 * Whenever main application tick is called, this callback will be called at the
 * end of the main tick execution.
 *
 */
void emberAfMainTickCallback(void)
{
  runDiscoveryStateMachine();
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup. Any
 * code that you would normally put into the top of the application's main()
 * routine should be put into this function. This is called before the clusters,
 * plugins, and the network are initialized so some functionality is not yet
 * available.
        Note: No callback in the Application Framework is
 * associated with resource cleanup. If you are implementing your application on
 * a Unix host where resource cleanup is a consideration, we expect that you
 * will use the standard Posix system calls, including the use of atexit() and
 * handlers for signals such as SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If
 * you use the signal() function to register your signal handler, please mind
 * the returned value which may be an Application Framework function. If the
 * return value is non-null, please make sure that you call the returned
 * function from your handler to avoid negating the resource cleanup of the
 * Application Framework itself.
 *
 */
void emberAfMainInitCallback(void)
{
  dmpUiInit();
  dmpUiDisplayHelp();
  emberEventControlSetDelayMS(lcdMainMenuDisplayEventControl, 10000);
  localDeviceTableClear();
}

/** @brief Report Attributes
 *
 * This function is called by the application framework when a Report Attributes
 * command is received from an external device.  The application should return
 * true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this command.  Ver.: always
 * @param buffer Buffer containing the list of attribute report records.  Ver.:
 * always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
boolean emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                        int8u *buffer,
                                        int16u bufLen)
{
  uint16_t bufIndex = 0;
  bool on = false;

  EmberAfAttributeId attributeId;
  attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                     bufIndex,
                                                     bufLen);

  if ((clusterId == ZCL_ON_OFF_CLUSTER_ID) && (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID)) {
    on = buffer[3];
  }

  if (localDeviceTableGetNumofLightDiscovered() >= 2) {
    uint16_t sourceNodeId = emberAfCurrentCommand()->source;
    DmpUiLightState_t lightState = (on ? DMP_UI_LIGHT_ON : DMP_UI_LIGHT_OFF);

    emberAfCorePrintln("sourceNodeId = %2x", sourceNodeId);

    if (sourceNodeId != 0) {
      dmpUiLightUpdateLight(DMP_UI_LIGHT_UNCHANGED, lightState);
    } else {
      dmpUiLightUpdateLight(lightState, DMP_UI_LIGHT_UNCHANGED);
    }
  } else {
    if (on == false) {
      dmpUiLightOff();
    } else {
      dmpUiLightOn();
    }
  }

  emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
  return TRUE;
}

/** @brief Pre ZDO Message Received
 *
 * This function passes the application an incoming ZDO message and gives the
 * appictation the opportunity to handle it. By default, this callback returns
 * false indicating that the incoming ZDO message has not been handled and
 * should be handled by the Application Framework.
 *
 * @param emberNodeId   Ver.: always
 * @param apsFrame   Ver.: always
 * @param message   Ver.: always
 * @param length   Ver.: always
 */
boolean emberAfPreZDOMessageReceivedCallback(EmberNodeId emberNodeId,
                                             EmberApsFrame* apsFrame,
                                             int8u* message,
                                             int16u length)
{
  if (emberNodeId != emberGetNodeId()) {
    if (apsFrame->clusterId == END_DEVICE_ANNOUNCE) {
      EmberEUI64 tempEui64;
      MEMMOVE(tempEui64, &(message[3]), EUI64_SIZE);

      if (getBindingTableEntry(tempEui64) == 0xFF) {
        emberAfCorePrintln("New Device Joined!!!");
        startFindingAndBindingForInitiator();
      }
    }
  }
  return false;
}
