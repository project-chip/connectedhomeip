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

#include <stdint.h>
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/zll-commissioning-common/zll-commissioning.h"
#include "rtos_bluetooth.h"
#include "gatt_db.h"

// Event control struct declarations
EmberEventControl identifyEventControl;
EmberEventControl networkEventControl;
EmberEventControl bleEventControl;
EmberEventControl zigbeeEventControl;
EmberEventControl bleTxEventControl;

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

#define MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH   70

#define ZIGBEE_TX_TEST_MAX_INFLIGHT         5

typedef struct {
  bool inUse;
  uint32_t startTime;
  uint8_t seqn;
} InflightInfo;

static struct {
  EmberApsOption apsOptions;
  EmberNodeId destination;
  uint8_t messageLength;
  uint16_t messageTotalCount;
  uint16_t messageRunningCount;
  uint16_t messageSuccessCount;
  uint8_t maxInFlight;
  uint8_t currentInFlight;
  uint16_t txDelayMs;
  uint32_t startTime;
  uint32_t minSendTimeMs;
  uint32_t maxSendTimeMs;
  uint32_t sumSendTimeMs;
  InflightInfo inflightInfoTable[ZIGBEE_TX_TEST_MAX_INFLIGHT];
} zigbeeTxTestParams;

static struct {
  uint16_t txDelayMs;
  uint8_t connHandle;
  uint16_t characteristicHandle;
  uint8_t size;
} bleTxTestParams;

static struct {
  bool on;
  bool disablePrint;
  uint8_t connectionHandle;
  uint16_t characteristic;
} bleReadParams = { false, true, 0, 0 };

static bool onOff = false;
static uint32_t identifyDurationMs = 0; // milliseconds
#define DEFAULT_IDENTIFY_DURATION_MS (3 * MILLISECOND_TICKS_PER_SECOND)
#define LED BOARDLED1

static void bleConnectionInfoTableInit(void);
static uint8_t bleConnectionInfoTableLookup(uint8_t connHandle);
static uint8_t bleConnectionInfoTableFindUnused(void);
static void bleConnectionInfoTablePrintEntry(uint8_t index);

static EmberStatus zigbeeSendTestMessage(EmberNodeId destination,
                                         uint8_t messageLength,
                                         uint8_t *message);

static void printBleAddress(uint8_t *address);

static void bleRead(void);
static void zigbeeTxTestStartCommand(void);
static void zigbeeTxTestStopCommand(void);
static void printZigBeeTxTestStats(void);
static void enableBleNotificationsCommand(void);
static void disableBleNotificationsCommand(void);
static void printBleConnectionTableCommand(void);
static void bleTxTestStartCommand(void);
static void bleTxTestStopCommand(void);
static void bleStartReadCommand(void);
static void bleStopReadCommand(void);

EmberCommandEntry emberAfCustomCommands[] = {
  emberCommandEntryAction("print-ble-connections",
                          printBleConnectionTableCommand,
                          "",
                          "Print BLE connections info"),
  emberCommandEntryAction("enable-ble-notifications",
                          enableBleNotificationsCommand,
                          "v",
                          "Enable BLE temperature notifications"),
  emberCommandEntryAction("disable-ble-notifications",
                          disableBleNotificationsCommand,
                          "",
                          "Disable BLE temperature notifications"),
  emberCommandEntryAction("start-zigbee-tx-test",
                          zigbeeTxTestStartCommand,
                          "uvvuvu",
                          "Start ZigBee TX test"),
  emberCommandEntryAction("stop-zigbee-tx-test",
                          zigbeeTxTestStopCommand,
                          "",
                          "Stop ZigBee TX test"),
  emberCommandEntryAction("start-ble-tx-test",
                          bleTxTestStartCommand,
                          "uvv",
                          "Start BLE TX test"),
  emberCommandEntryAction("stop-ble-tx-test",
                          bleTxTestStopCommand,
                          "",
                          "Stop BLE TX test"),
  emberCommandEntryAction("start-ble-rx-test",
                          bleStartReadCommand,
                          "uv",
                          "Start BLE RX test"),
  emberCommandEntryAction("stop-ble-rx-test",
                          bleStopReadCommand,
                          "",
                          "Stop BLE RX test"),
  emberCommandEntryTerminator(),
};

void identifyEventHandler(void)
{
  if (identifyDurationMs == 0) {
    if (onOff) {
      halSetLed(LED);
    } else {
      halClearLed(LED);
    }
    emberEventControlSetInactive(identifyEventControl);
  } else {
    halToggleLed(LED);
    if (identifyDurationMs >= MILLISECOND_TICKS_PER_QUARTERSECOND) {
      identifyDurationMs -= MILLISECOND_TICKS_PER_QUARTERSECOND;
    } else {
      identifyDurationMs = 0;
    }
    emberEventControlSetDelayMS(identifyEventControl,
                                MILLISECOND_TICKS_PER_QUARTERSECOND);
  }
}

void networkEventHandler(void)
{
  if (emberAfZllScanForUnusedPanId() == EMBER_SUCCESS) {
    emberEventControlSetInactive(networkEventControl);
    halPlayTune_P(waitTune, true);
  } else {
    emberAfZllResetToFactoryNew();
    emberEventControlSetActive(networkEventControl);
    halPlayTune_P(sadTune, true);
  }
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
  /*
     if (status == EMBER_NETWORK_UP) {
     emberEventControlSetInactive(networkEventControl);
     halPlayTune_P(happyTune, true);
     } else if (status == EMBER_NETWORK_DOWN
             && emberAfNetworkState() == EMBER_NO_NETWORK) {
     emberEventControlSetActive(networkEventControl);
     }
   */

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
  bleConnectionInfoTableInit();

  //emberEventControlSetActive(networkEventControl);
}

/** @brief On/off Cluster Server Post Init
 *
 * Following resolution of the On/Off state at startup for this endpoint, perform any
 * additional initialization needed; e.g., synchronize hardware state.
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
    if (emberAfReadServerAttribute(endpoint,
                                   ZCL_ON_OFF_CLUSTER_ID,
                                   ZCL_ON_OFF_ATTRIBUTE_ID,
                                   (uint8_t *)&onOff,
                                   sizeof(onOff))
        == EMBER_ZCL_STATUS_SUCCESS) {
      if (onOff) {
        halSetLed(LED);
      } else {
        halClearLed(LED);
      }
    }
  }
}

/** @brief Get Group Name
 *
 * This function returns the name of a group with the provided group ID,
 * should it exist.
 *
 * @param endpoint Endpoint  Ver.: always
 * @param groupId Group ID  Ver.: always
 * @param groupName Group Name  Ver.: always
 */
void emberAfPluginGroupsServerGetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
{
}

/** @brief Set Group Name
 *
 * This function sets the name of a group with the provided group ID.
 *
 * @param endpoint Endpoint  Ver.: always
 * @param groupId Group ID  Ver.: always
 * @param groupName Group Name  Ver.: always
 */
void emberAfPluginGroupsServerSetGroupNameCallback(uint8_t endpoint,
                                                   uint16_t groupId,
                                                   uint8_t *groupName)
{
}

/** @brief Group Names Supported
 *
 * This function returns whether or not group names are supported.
 *
 * @param endpoint Endpoint  Ver.: always
 */
bool emberAfPluginGroupsServerGroupNamesSupportedCallback(uint8_t endpoint)
{
  return false;
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
  return true;
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
  return true;
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
void emberAfPluginZllCommissioningCommonInitialSecurityStateCallback(EmberZllInitialSecurityState *securityState)
{
  // By default, the plugin will configure the stack for the certification
  // encryption algorithm.  Devices that are certified should instead use the
  // master encryption algorithm and set the appropriate encryption key and
  // pre-configured link key.
}

/** @brief Touch Link Complete
 *
 * This function is called by the ZLL Commissioning Common plugin when touch linking
 * completes.
 *
 * @param networkInfo The ZigBee and ZLL-specific information about the
 * network and target.  Ver.: always
 * @param deviceInformationRecordCount The number of sub-device information
 * records for the target.  Ver.: always
 * @param deviceInformationRecordList The list of sub-device information
 * records for the target.  Ver.: always
 */
void emberAfPluginZllCommissioningCommonTouchLinkCompleteCallback(const EmberZllNetwork *networkInfo,
                                                                  uint8_t deviceInformationRecordCount,
                                                                  const EmberZllDeviceInfoRecord *deviceInformationRecordList)
{
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
  halPlayTune_P(sadTune, true);
}

/** @brief Group Identifier Count
 *
 * This function is called by the ZLL Commissioning Server plugin to determine the
 * number of group identifiers in use by a specific endpoint on the device.
 * The total number of group identifiers on the device, which are shared by
 * all endpoints, is defined by ::EMBER_ZLL_GROUP_ADDRESSES.
 *
 * @param endpoint The endpoint for which the group identifier count is
 * requested.  Ver.: always
 */
uint8_t emberAfPluginZllCommissioningServerGroupIdentifierCountCallback(uint8_t endpoint)
{
  // Devices with multiple endpoints will have to distribute the group ids
  // available to the device among the individual endpoints.  This application
  // has one endpoint, so it uses all available group ids.
  return EMBER_ZLL_GROUP_ADDRESSES;
}

/** @brief Group Identifier
 *
 * This function is called by the ZLL Commissioning Server plugin to obtain
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
bool emberAfPluginZllCommissioningServerGroupIdentifierCallback(uint8_t endpoint,
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
 * This function is called by the ZLL Commissioning Server plugin to determine the
 * number of remote endpoints controlled by a specific endpoint on the local
 * device.
 *
 * @param endpoint The local endpoint for which the remote endpoint
 * information count is requested.  Ver.: always
 */
uint8_t emberAfPluginZllCommissioningServerEndpointInformationCountCallback(uint8_t endpoint)
{
  // Devices that control remote endpoints individually will have to maintain
  // a list of those endpoints.  This application does not control remote
  // endpoints individually.
  return 0x00;
}

/** @brief Endpoint Information
 *
 * This function is called by the ZLL Commissioning Server plugin to obtain
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
bool emberAfPluginZllCommissioningServerEndpointInformationCallback(uint8_t endpoint,
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
 * This function is called by the ZLL Commissioning Server plugin to notify the
 * application that it should take an action to identify itself.  This
 * typically occurs when an Identify Request is received via inter-PAN
 * messaging.
 *
 * @param durationS If the duration is zero, the device should exit identify
 * mode.  If the duration is 0xFFFF, the device should remain in identify mode
 * for the default time.  Otherwise, the duration specifies the length of time
 * in seconds that the device should remain in identify mode.  Ver.: always
 */
void emberAfPluginZllCommissioningServerIdentifyCallback(uint16_t durationS)
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
 * This function is called by the ZLL Commissioning Common plugin when a request to
 * reset to factory new is received.  The plugin will leave the network, reset
 * attributes managed by the framework to their default values, and clear the
 * group and scene tables.  The application should perform any other necessary
 * reset-related operations in this callback, including resetting any
 * externally-stored attributes.
 *
 */
void emberAfPluginZllCommissioningCommonResetToFactoryNewCallback(void)
{
}

/** @brief Join
 *
 * This callback is called by the ZLL Commissioning Network plugin when a joinable
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
bool emberAfPluginZllCommissioningNetworkJoinCallback(EmberZigbeeNetwork *networkFound,
                                                      uint8_t lqi,
                                                      int8_t rssi)
{
  return true;
}

/** @brief Off With Effect
 *
 * This callback is called by the ZLL On/Off Server plugin whenever an
 * OffWithEffect command is received.  The application should implement the
 * effect and variant requested in the command and return
 * ::EMBER_ZCL_STATUS_SUCCESS if successful or an appropriate error status
 * otherwise.
 *
 * @param endpoint   Ver.: always
 * @param effectId   Ver.: always
 * @param effectVariant   Ver.: always
 */
EmberAfStatus emberAfPluginZllOnOffServerOffWithEffectCallback(uint8_t endpoint,
                                                               uint8_t effectId,
                                                               uint8_t effectVariant)
{
  // This sample application is not capable of implementing the effects and
  // variants, so it simply returns SUCCESS.  A real device should perform the
  // requested effect and variant and return an appropriate status.
  return EMBER_ZCL_STATUS_SUCCESS;
}

struct {
  bool inUse;
  bool isMaster;
  uint8_t connectionHandle;
  uint8_t bondingHandle;
  uint8_t remoteAddress[6];
} bleConnectionTable[EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS];

static EmberStatus zigbeeSendTestMessage(EmberNodeId destination,
                                         uint8_t messageLength,
                                         uint8_t *message)
{
  EmberApsFrame apsFrame;
  uint8_t messageTag;

  apsFrame.sourceEndpoint = 0xFF;
  apsFrame.destinationEndpoint = 0xFF;
  apsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS;
  apsFrame.profileId = 0xFFFF;
  apsFrame.clusterId = 0x2000;

  return emAfSend(EMBER_OUTGOING_DIRECT,
                  destination,
                  &apsFrame,
                  messageLength,
                  message,
                  &messageTag,
                  0xFFFF,
                  0);
}

static void printZigBeeTxTestStats(void);

/** @brief Message Sent
 *
 * This function is called by the application framework from the message sent
 * handler, when it is informed by the stack regarding the message sent status.
 * All of the values passed to the emberMessageSentHandler are passed on to this
 * callback. This provides an opportunity for the application to verify that its
 * message has been sent successfully and take the appropriate action. This
 * callback should return a bool value of true or false. A value of true
 * indicates that the message sent notification has been handled and should not
 * be handled by the application framework.
 *
 * @param type   Ver.: always
 * @param indexOrDestination   Ver.: always
 * @param apsFrame   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 * @param status   Ver.: always
 */
bool emberAfMessageSentCallback(EmberOutgoingMessageType type,
                                uint16_t indexOrDestination,
                                EmberApsFrame* apsFrame,
                                uint16_t msgLen,
                                uint8_t* message,
                                EmberStatus status)
{
  // This is a message sent out as part of the ZigBee TX test
  if (apsFrame->profileId == 0x7F01 && apsFrame->clusterId == 0x0001) {
    uint32_t packetSendTimeMs = 0xFFFFFFFF;
    uint8_t i;

    zigbeeTxTestParams.currentInFlight--;

    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (zigbeeTxTestParams.inflightInfoTable[i].seqn == apsFrame->sequence) {
        zigbeeTxTestParams.inflightInfoTable[i].inUse = false;
        packetSendTimeMs =
          elapsedTimeInt32u(zigbeeTxTestParams.inflightInfoTable[i].startTime,
                            halCommonGetInt32uMillisecondTick());
        break;
      }
    }

    assert(packetSendTimeMs != 0xFFFFFFFF);

    if (status == EMBER_SUCCESS) {
      zigbeeTxTestParams.messageSuccessCount++;
      zigbeeTxTestParams.sumSendTimeMs += packetSendTimeMs;

      if (zigbeeTxTestParams.minSendTimeMs > packetSendTimeMs) {
        zigbeeTxTestParams.minSendTimeMs = packetSendTimeMs;
      }
      if (zigbeeTxTestParams.maxSendTimeMs < packetSendTimeMs) {
        zigbeeTxTestParams.maxSendTimeMs = packetSendTimeMs;
      }
    }

    if (zigbeeTxTestParams.currentInFlight == 0
        && zigbeeTxTestParams.messageRunningCount
        == zigbeeTxTestParams.messageTotalCount) {
      printZigBeeTxTestStats();
    }

    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
// BLE - Callbacks

/** @brief
 *
 * This function is called from the BLE stack to notify the application of a
 * stack event.
 */
void emberAfPluginBleEventCallback(struct gecko_cmd_packet* evt)
{
  switch (BGLIB_MSG_ID(evt->header)) {
    case gecko_evt_system_boot_id:
    {
      struct gecko_msg_system_hello_rsp_t *hello_rsp;
      struct gecko_msg_system_get_bt_address_rsp_t *get_address_rsp;

      // Call these two APIs upon boot for testing purposes.
      hello_rsp = gecko_cmd_system_hello();
      emberAfCorePrintln("BLE hello: %s",
                         (hello_rsp->result == bg_err_success) ? "success" : "error");

      get_address_rsp = gecko_cmd_system_get_bt_address();
      emberAfCorePrint("BLE address: ");
      printBleAddress(get_address_rsp->address.addr);
      emberAfCorePrintln("");
    }
    break;
    case gecko_evt_le_connection_opened_id:
    {
      struct gecko_msg_le_connection_opened_evt_t *conn_evt =
        (struct gecko_msg_le_connection_opened_evt_t*)&(evt->data);
      uint8_t index = bleConnectionInfoTableFindUnused();
      assert(index < 0xFF);

      bleConnectionTable[index].inUse = true;
      bleConnectionTable[index].isMaster = (conn_evt->master > 0);
      bleConnectionTable[index].connectionHandle = conn_evt->connection;
      bleConnectionTable[index].bondingHandle = conn_evt->bonding;
      memcpy(bleConnectionTable[index].remoteAddress, conn_evt->address.addr, 6);

      emberAfCorePrintln("BLE connection opened");
      bleConnectionInfoTablePrintEntry(index);
    }
    break;
    case gecko_evt_le_connection_closed_id:
    {
      struct gecko_msg_le_connection_closed_evt_t *conn_evt =
        (struct gecko_msg_le_connection_closed_evt_t*)&(evt->data);
      uint8_t index = bleConnectionInfoTableLookup(conn_evt->connection);
      assert(index < 0xFF);

      bleConnectionTable[index].inUse = false;

      emberAfCorePrintln("BLE connection closed, handle=0x%x, reason=0x%2x",
                         conn_evt->connection, conn_evt->reason);
    }
    break;
    case gecko_evt_le_gap_scan_response_id:
    {
      struct gecko_msg_le_gap_scan_response_evt_t *scan_evt =
        (struct gecko_msg_le_gap_scan_response_evt_t*)&(evt->data);

      emberAfCorePrint("Scan response, address type=0x%x, address: ",
                       scan_evt->address_type);
      printBleAddress(scan_evt->address.addr);
      emberAfCorePrintln("");
    }
    break;
    case gecko_evt_sm_list_bonding_entry_id:
    {
      struct gecko_msg_sm_list_bonding_entry_evt_t * bonding_entry_evt =
        (struct gecko_msg_sm_list_bonding_entry_evt_t*)&(evt->data);

      emberAfCorePrint("Bonding handle=0x%x, address type=0x%x, address: ",
                       bonding_entry_evt->bonding, bonding_entry_evt->address_type);
      printBleAddress(bonding_entry_evt->address.addr);
      emberAfCorePrintln("");
    }
    break;
    case gecko_evt_gatt_service_id:
    {
      struct gecko_msg_gatt_service_evt_t* service_evt =
        (struct gecko_msg_gatt_service_evt_t*)&(evt->data);
      uint8_t i;

      emberAfCorePrintln("GATT service: ");
      emberAfCorePrint(" - UUID=[");
      for (i = 0; i < service_evt->uuid.len; i++) {
        emberAfCorePrint("0x%x ", service_evt->uuid.data[i]);
      }
      emberAfCorePrintln("]");
      emberAfCorePrintln(" - conn_handle=0x%x\n - service_handle=0x%4x",
                         service_evt->connection, service_evt->service);
    }
    break;
    case gecko_evt_gatt_characteristic_id:
    {
      struct gecko_msg_gatt_characteristic_evt_t* char_evt =
        (struct gecko_msg_gatt_characteristic_evt_t*)&(evt->data);
      uint8_t i;

      emberAfCorePrintln("GATT characteristic:");
      emberAfCorePrint(" - UUID=[");
      for (i = 0; i < char_evt->uuid.len; i++) {
        emberAfCorePrint("0x%x ", char_evt->uuid.data[i]);
      }
      emberAfCorePrintln("]");
      emberAfCorePrintln(" - conn_handle=0x%x\n - char_handle=0x%2x\n - properties=0x%x",
                         char_evt->connection, char_evt->characteristic, char_evt->properties);
    }
    break;
    case gecko_evt_gatt_characteristic_value_id:
    {
      struct gecko_msg_gatt_characteristic_value_evt_t* char_evt =
        (struct gecko_msg_gatt_characteristic_value_evt_t*)&(evt->data);
      uint8_t i;

      if (char_evt->att_opcode == gatt_handle_value_indication) {
        gecko_cmd_gatt_send_characteristic_confirmation(char_evt->connection);
      }

      if (!(bleReadParams.on && bleReadParams.disablePrint)) {
        emberAfCorePrintln("GATT (client) characteristic value, handle=0x%x, characteristic=0x%2x, att_op_code=0x%x",
                           char_evt->connection,
                           char_evt->characteristic,
                           char_evt->att_opcode);
        emberAfCorePrint("value=[");
        for (i = 0; i < char_evt->value.len; i++) {
          emberAfCorePrint("0x%x ", char_evt->value.data[i]);
        }
        emberAfCorePrintln("]");
      }
    }
    break;
    case gecko_evt_gatt_server_attribute_value_id:
    {
      struct gecko_msg_gatt_server_attribute_value_evt_t* attr_evt =
        (struct gecko_msg_gatt_server_attribute_value_evt_t*)&(evt->data);
      EmberStatus status;
      uint8_t i;

      if (!(bleReadParams.on && bleReadParams.disablePrint)) {
        emberAfCorePrintln("GATT (server) attribute value, handle=0x%x, attribute=0x%2x, att_op_code=0x%x",
                           attr_evt->connection,
                           attr_evt->attribute,
                           attr_evt->att_opcode);
        emberAfCorePrint("value=[");
        for (i = 0; i < attr_evt->value.len; i++) {
          emberAfCorePrint("0x%x ", attr_evt->value.data[i]);
        }
        emberAfCorePrintln("]");
      }

      if (!bleReadParams.on) {
        // Forward the attribute over the ZigBee network.
        // TODO: for now we always send to the coordinator.
        status = zigbeeSendTestMessage(0x0000, // coordinator ID
                                       attr_evt->value.len,
                                       attr_evt->value.data);

        emberAfCorePrintln("sent 0x%x", status);
      }
    }
    break;
    case gecko_evt_le_connection_parameters_id:
    {
      struct gecko_msg_le_connection_parameters_evt_t* param_evt =
        (struct gecko_msg_le_connection_parameters_evt_t*)&(evt->data);
      emberAfCorePrintln("BLE connection parameters are updated, handle=0x%x, interval=0x%2x, latency=0x%2x, timeout=0x%2x, security=0x%x, txsize=0x%2x",
                         param_evt->connection,
                         param_evt->interval,
                         param_evt->latency,
                         param_evt->timeout,
                         param_evt->security_mode,
                         param_evt->txsize);
    }
    break;
    case gecko_evt_gatt_procedure_completed_id:
    {
      struct gecko_msg_gatt_procedure_completed_evt_t* proc_comp_evt =
        (struct gecko_msg_gatt_procedure_completed_evt_t*)&(evt->data);

      if (!(bleReadParams.on && bleReadParams.disablePrint)) {
        emberAfCorePrintln("BLE procedure completed, handle=0x%x, result=0x%2x",
                           proc_comp_evt->connection,
                           proc_comp_evt->result);
      }

      if (bleReadParams.on) {
        bleRead();
      }
    }
    break;
  }
}

//------------------------------------------------------------------------------
// BLE - Connection info table functions

static void bleConnectionInfoTableInit(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    bleConnectionTable[i].inUse = false;
  }
}

static uint8_t bleConnectionInfoTableLookup(uint8_t connHandle)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse
        && bleConnectionTable[i].connectionHandle == connHandle) {
      return i;
    }
  }

  return 0xFF;
}

static uint8_t bleConnectionInfoTableFindUnused(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (!bleConnectionTable[i].inUse) {
      return i;
    }
  }

  return 0xFF;
}

static void bleConnectionInfoTablePrintEntry(uint8_t index)
{
  assert(index < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS
         && bleConnectionTable[index].inUse);

  emberAfCorePrintln("**** Connection Info ****");
  emberAfCorePrintln("connection handle 0x%x",
                     bleConnectionTable[index].connectionHandle);
  emberAfCorePrintln("bonding handle = 0x%x",
                     bleConnectionTable[index].bondingHandle);
  emberAfCorePrintln("local node is %s",
                     (bleConnectionTable[index].isMaster) ? "master" : "slave");
  emberAfCorePrint("remote address: ");
  printBleAddress(bleConnectionTable[index].remoteAddress);
  emberAfCorePrintln("");
  emberAfCorePrintln("*************************");
}

static void printBleAddress(uint8_t *address)
{
  emberAfCorePrint("[%x %x %x %x %x %x]",
                   address[5], address[4], address[3],
                   address[2], address[1], address[0]);
}

//------------------------------------------------------------------------------
// BLE - Handler and helpers

// Convert mantissa & exponent values to IEEE float type
static inline uint32_t bg_uint32_to_float(uint32_t mantissa, int32_t exponent)
{
  return (mantissa & 0xffffff) | (uint32_t)(exponent << 24);
}

// Temperature measurement locations
enum bg_thermometer_temperature_type{
  bg_thermometer_temperature_type_armpit=1,
  bg_thermometer_temperature_type_body,
  bg_thermometer_temperature_type_ear,
  bg_thermometer_temperature_type_finger,
  bg_thermometer_temperature_type_gastro_intestinal_tract,
  bg_thermometer_temperature_type_mouth,
  bg_thermometer_temperature_type_rectum,
  bg_thermometer_temperature_type_toe,
  bg_thermometer_temperature_type_tympanum,
};

enum bg_thermometer_temperature_measurement_flag{
  bg_thermometer_temperature_measurement_flag_units    =0x1,
  bg_thermometer_temperature_measurement_flag_timestamp=0x2,
  bg_thermometer_temperature_measurement_flag_type     =0x4,
};

// Create temperature measurement value from IEEE float and temperature type flag
static inline void bg_thermometer_create_measurement(uint8_t* buffer,
                                                     uint32_t measurement,
                                                     int fahrenheit)
{
  buffer[0] = fahrenheit ? bg_thermometer_temperature_measurement_flag_units : 0;
  buffer[1] = measurement & 0xff;
  buffer[2] = (measurement >> 8) & 0xff;
  buffer[3] = (measurement >> 16) & 0xff;
  buffer[4] = (measurement >> 24) & 0xff;
}

static uint16_t bleNotificationsPeriodMs;
static uint8_t bleTemperatureCounter;

void bleEventHandler(void)
{
  uint8_t tempBuffer[5];

  bg_thermometer_create_measurement(tempBuffer,
                                    bg_uint32_to_float(bleTemperatureCounter, 0),
                                    0);

  gecko_cmd_gatt_server_send_characteristic_notification(0xff,
                                                         gattdb_temp_measurement,
                                                         5,
                                                         tempBuffer);

  if (++bleTemperatureCounter > 40) {
    bleTemperatureCounter = 20;
  }
  emberEventControlSetDelayMS(bleEventControl, bleNotificationsPeriodMs);
}

//------------------------------------------------------------------------------
// BLE - TX test macros and functions

#define BLE_TX_TEST_DATA_SIZE   6

void bleTxEventHandler(void)
{
  uint8_t txData[BLE_TX_TEST_DATA_SIZE];
  uint8_t i;

  for (i = 0; i < BLE_TX_TEST_DATA_SIZE; i++) {
    txData[i] = i;
  }

  gecko_cmd_gatt_write_characteristic_value_without_response(bleTxTestParams.connHandle,
                                                             bleTxTestParams.characteristicHandle,
                                                             BLE_TX_TEST_DATA_SIZE,
                                                             txData);

  emberEventControlSetDelayMS(bleTxEventControl, bleTxTestParams.txDelayMs);
}

static void bleTxTestStopCommand(void)
{
  emberEventControlSetInactive(bleTxEventControl);
}

static void bleTxTestStartCommand(void)
{
  bleTxTestParams.connHandle = emberUnsignedCommandArgument(0);
  bleTxTestParams.characteristicHandle = emberUnsignedCommandArgument(1);
  bleTxTestParams.txDelayMs = emberUnsignedCommandArgument(2);

  bleTxEventHandler();
}

//------------------------------------------------------------------------------
// For continuous read of a BLE characteristic

static void bleRead(void)
{
  struct gecko_msg_gatt_read_characteristic_value_rsp_t* rsp =
    gecko_cmd_gatt_read_characteristic_value(bleReadParams.connectionHandle,
                                             bleReadParams.characteristic);
  if (rsp->result != bg_err_success) {
    emberAfCorePrintln("read error: 0x%2x", rsp->result);
  }
}

void bleStartReadCommand(void)
{
  bleReadParams.connectionHandle = emberUnsignedCommandArgument(0);
  bleReadParams.characteristic = emberUnsignedCommandArgument(1);
  bleReadParams.on = true;
  bleRead();
}

void bleStopReadCommand(void)
{
  bleReadParams.on = false;
}

//------------------------------------------------------------------------------
// BLE - Custom CLI commands

static void printBleConnectionTableCommand(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse) {
      bleConnectionInfoTablePrintEntry(i);
    }
  }
}

static void enableBleNotificationsCommand(void)
{
  bleNotificationsPeriodMs = emberUnsignedCommandArgument(0);
  bleTemperatureCounter = 20;
  emberAfCorePrintln("BLE notifications enabled");
  bleEventHandler();
}

static void disableBleNotificationsCommand(void)
{
  emberAfCorePrintln("BLE notifications disabled");
  emberEventControlSetInactive(bleEventControl);
}

//------------------------------------------------------------------------------
// ZigBee TX test macros and functions

void zigbeeEventHandler(void)
{
  EmberApsFrame apsFrame;
  uint8_t messagePayload[MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH];
  uint8_t *messageTag = NULL;
  uint8_t i;

  // First byte is the message length
  messagePayload[0] = zigbeeTxTestParams.messageLength;
  // Second and third bytes are the message counter
  messagePayload[1] = LOW_BYTE(zigbeeTxTestParams.messageRunningCount);
  messagePayload[2] = HIGH_BYTE(zigbeeTxTestParams.messageRunningCount);

  // Init the the rest of the message payload with progressive byte values
  for (i = 3; i < zigbeeTxTestParams.messageLength; i++) {
    messagePayload[i] = i - 3;
  }

  if (zigbeeTxTestParams.maxInFlight > 0
      && zigbeeTxTestParams.currentInFlight >= zigbeeTxTestParams.maxInFlight) {
    // Max in flight reached, wait and try again.
    emberEventControlSetDelayMS(zigbeeEventControl, 1);
    return;
  }

  apsFrame.sourceEndpoint = 0xFF;
  apsFrame.destinationEndpoint = 0xFF;
  apsFrame.options = zigbeeTxTestParams.apsOptions;
  apsFrame.profileId = 0x7F01; // test profile ID
  apsFrame.clusterId = 0x0001; // counted packets cluster

  if (emAfSend(EMBER_OUTGOING_DIRECT,
               zigbeeTxTestParams.destination,
               &apsFrame,
               zigbeeTxTestParams.messageLength,
               messagePayload,
               messageTag,
               0xFFFF,
               0) == EMBER_SUCCESS) {
    zigbeeTxTestParams.messageRunningCount++;
    zigbeeTxTestParams.currentInFlight++;

    for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
      if (!zigbeeTxTestParams.inflightInfoTable[i].inUse) {
        zigbeeTxTestParams.inflightInfoTable[i].inUse = true;
        zigbeeTxTestParams.inflightInfoTable[i].seqn = apsFrame.sequence;
        zigbeeTxTestParams.inflightInfoTable[i].startTime =
          halCommonGetInt32uMillisecondTick();
        break;
      }
    }
    assert(i < ZIGBEE_TX_TEST_MAX_INFLIGHT);
  }

  if (zigbeeTxTestParams.messageRunningCount
      >= zigbeeTxTestParams.messageTotalCount) {
    emberEventControlSetInactive(zigbeeEventControl);
  } else {
    emberEventControlSetDelayMS(zigbeeEventControl,
                                zigbeeTxTestParams.txDelayMs);
  }
}

static void zigbeeTxTestStopCommand(void)
{
  emberEventControlSetInactive(zigbeeEventControl);
  zigbeeTxTestParams.messageTotalCount = 0;
}

static void zigbeeTxTestStartCommand(void)
{
  uint8_t i;
  zigbeeTxTestParams.messageLength = emberUnsignedCommandArgument(0);

  if (zigbeeTxTestParams.messageLength > MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH) {
    emberAfCorePrintln("Error: max allowed message payload is %d",
                       MAX_ZIGBEE_TX_TEST_MESSAGE_LENGTH);
    return;
  }

  zigbeeTxTestParams.maxInFlight = emberUnsignedCommandArgument(3);

  if (zigbeeTxTestParams.maxInFlight > ZIGBEE_TX_TEST_MAX_INFLIGHT) {
    emberAfCorePrintln("Error: max allowed inflight is %d",
                       ZIGBEE_TX_TEST_MAX_INFLIGHT);
    return;
  }

  zigbeeTxTestParams.messageTotalCount = emberUnsignedCommandArgument(1);

  if (zigbeeTxTestParams.messageTotalCount == 0) {
    emberAfCorePrintln("Error: message count is 0");
    return;
  }

  zigbeeTxTestParams.txDelayMs = emberUnsignedCommandArgument(2);
  zigbeeTxTestParams.destination = emberUnsignedCommandArgument(4);
  zigbeeTxTestParams.apsOptions = emberUnsignedCommandArgument(5);

  zigbeeTxTestParams.currentInFlight = 0;
  zigbeeTxTestParams.messageRunningCount = 0;
  zigbeeTxTestParams.messageSuccessCount = 0;
  zigbeeTxTestParams.startTime = halCommonGetInt32uMillisecondTick();
  zigbeeTxTestParams.minSendTimeMs = 0xFFFFFFFF;
  zigbeeTxTestParams.maxSendTimeMs = 0;
  zigbeeTxTestParams.sumSendTimeMs = 0;

  for (i = 0; i < ZIGBEE_TX_TEST_MAX_INFLIGHT; i++) {
    zigbeeTxTestParams.inflightInfoTable[i].inUse = false;
  }

  emberAfCorePrintln("ZigBee TX test started");

  zigbeeEventHandler();
}

static void printZigBeeTxTestStats(void)
{
  uint32_t totalSendTimeMs =
    elapsedTimeInt32u(zigbeeTxTestParams.startTime,
                      halCommonGetInt32uMillisecondTick());
  uint64_t throughput = (zigbeeTxTestParams.messageSuccessCount
                         * zigbeeTxTestParams.messageLength * 8);
  throughput = throughput * 1000;
  throughput = throughput / totalSendTimeMs;

  emberAfCorePrintln("ZigBee TX done");
  emberAfCorePrintln("Total time %ums", totalSendTimeMs);
  emberAfCorePrintln("Success messages: %d out of %d",
                     zigbeeTxTestParams.messageSuccessCount,
                     zigbeeTxTestParams.messageTotalCount);
  emberAfCorePrintln("Throughput: %u bits/s", throughput);
  if (zigbeeTxTestParams.messageSuccessCount > 0) {
    emberAfCorePrintln("Min packet send time: %u ms",
                       zigbeeTxTestParams.minSendTimeMs);
    emberAfCorePrintln("Max packet send time: %u ms",
                       zigbeeTxTestParams.maxSendTimeMs);
    emberAfCorePrintln("Avg packet send time: %u ms",
                       (zigbeeTxTestParams.sumSendTimeMs
                        / zigbeeTxTestParams.messageSuccessCount));
  }
}
