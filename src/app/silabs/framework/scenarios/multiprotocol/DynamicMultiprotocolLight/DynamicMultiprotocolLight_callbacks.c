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
#include <stdint.h>
#include <stdio.h>
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/plugin/network-creator-security/network-creator-security.h"
#include "app/framework/plugin/network-creator/network-creator.h"
#include "app/framework/plugin/reporting/reporting.h"
#include "rtos_bluetooth.h"
#include "gatt_db.h"
#ifdef EMBER_AF_PLUGIN_DMP_UI_DEMO
 #include "app/framework/plugin/dmp-ui-demo/dmp-ui.h"
#elif defined(EMBER_AF_PLUGIN_DMP_UI_DEMO_STUB)
 #include "app/framework/plugin/dmp-ui-demo-stub/dmp-ui-stub.h"
#endif

/* Write response codes*/
#define ES_WRITE_OK                         0
#define ES_ERR_CCCD_CONF                    0x81
#define ES_ERR_PROC_IN_PROGRESS             0x80
#define ES_NO_CONNECTION                    0xFF
// Advertisement data
#define UINT16_TO_BYTES(n)        ((uint8_t) (n)), ((uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)        ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)        ((uint8_t) ((n) >> 8))
// Ble TX test macros and functions
#define BLE_TX_TEST_DATA_SIZE   2
// We need to put the device name into a scan response packet,
// since it isn't included in the 'standard' beacons -
// I've included the flags, since certain apps seem to expect them
#define DEVNAME "DMP%02X%02X"
#define DEVNAME_LEN 8  // incl term null
#define UUID_LEN 16 // 128-bit UUID
#define SOURCE_ADDRESS_LEN 8

#define LE_GAP_MAX_DISCOVERABLE_MODE   0x04
#define LE_GAP_MAX_CONNECTABLE_MODE    0x03
#define LE_GAP_MAX_DISCOVERY_MODE      0x02
#define BLE_INDICATION_TIMEOUT         30000
#define BUTTON_LONG_PRESS_TIME_MSEC    3000

static void readLightState(uint8_t connection);
static void readTriggerSource(uint8_t connection);
static void readSourceAddress(uint8_t connection);
static void writeLightState(uint8_t connection, uint8array *writeValue);
static void enableBleAdvertisements(void);
static uint8_t bleConnectionInfoTableFindUnused(void);
static bool bleConnectionInfoTableIsEmpty(void);
static void bleConnectionInfoTablePrintEntry(uint8_t index);
static uint8_t bleConnectionInfoTableLookup(uint8_t connHandle);
static void printBleAddress(uint8_t *address);
static void setDefaultReportEntry(void);
static bool startPjoinAndIdentifying(uint16_t identifyTime);
static void startIdentifyOnAllChildNodes(uint16_t identifyTime);

// Event function forward declarations
void bleEventHandler(void);
void bleTxEventHandler(void);
void lcdMainMenuDisplayEventHandler(void);
void lcdPermitJoinEventHandler(void);
void buttonEventHandler(void);
void bleAdvertsEventHandler(void);

enum {
  DMP_LIGHT_OFF,
  DMP_LIGHT_ON
};
// Light state
enum {
  HANDLE_DEMO,
  HANDLE_IBEACON,
  HANDLE_EDDYSTONE
};
//advertisers handle

typedef enum {
  GAT_SERVER_CLIENT_CONFIG = 1,
  GAT_SERVER_CONFIRMATION,
} BLE_GAT_STATUS_FLAG;

typedef enum {
  GAT_DISABLED,
  GAT_RECEIVE_NOTIFICATION,
  GAT_RECEIVE_INDICATION,
} BLE_GAT_CLIENT_CONFIG_FLAG;

static BLE_GAT_CLIENT_CONFIG_FLAG ble_lightState_config = 0;
static BLE_GAT_CLIENT_CONFIG_FLAG ble_triggerSrc_config = 0;
static BLE_GAT_CLIENT_CONFIG_FLAG ble_bleSrc_config = 0;
static uint8_t ble_lightState = DMP_LIGHT_OFF;
static uint8_t ble_lastEvent = DMP_UI_DIRECTION_INVALID;
static uint8_t SourceAddress[SOURCE_ADDRESS_LEN];
static uint8_t SwitchEUI[SOURCE_ADDRESS_LEN];
static EmberEUI64 LightEUI;
static uint8_t ActiveBleConnections = 0;
static uint8_t lastButton;
static bool longPress = false;
static uint16_t bleNotificationsPeriodMs;
static DmpUiLightDirection_t lightDirection = DMP_UI_DIRECTION_INVALID;
// Event control struct declarations
EmberEventControl identifyEventControl;
EmberEventControl networkEventControl;
EmberEventControl bleEventControl;
EmberEventControl bleTxEventControl;
EmberEventControl zigbeeEventControl;
EmberEventControl lcdMainMenuDisplayEventControl;
EmberEventControl lcdPermitJoinEventControl;
EmberEventControl buttonEventControl;

struct {
  bool inUse;
  bool isMaster;
  uint8_t connectionHandle;
  uint8_t bondingHandle;
  uint8_t remoteAddress[6];
} bleConnectionTable[EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS];

/** GATT Server Attribute User Read Configuration.
 *  Structure to register handler functions to user read events. */
typedef struct {
  uint16 charId; /**< ID of the Characteristic. */
  void (*fctn)(uint8_t connection); /**< Handler function. */
} AppCfgGattServerUserReadRequest_t;

/** GATT Server Attribute Value Write Configuration.
 *  Structure to register handler functions to characteristic write events. */
typedef struct {
  uint16 charId; /**< ID of the Characteristic. */
  /**< Handler function. */
  void (*fctn)(uint8_t connection, uint8array * writeValue);
} AppCfgGattServerUserWriteRequest_t;

// iBeacon structure and data
static struct {
  uint8_t flagsLen; /* Length of the Flags field. */
  uint8_t flagsType; /* Type of the Flags field. */
  uint8_t flags; /* Flags field. */
  uint8_t mandataLen; /* Length of the Manufacturer Data field. */
  uint8_t mandataType; /* Type of the Manufacturer Data field. */
  uint8_t compId[2]; /* Company ID field. */
  uint8_t beacType[2]; /* Beacon Type field. */
  uint8_t uuid[16]; /* 128-bit Universally Unique Identifier (UUID). The UUID is an identifier for the company using the beacon*/
  uint8_t majNum[2]; /* Beacon major number. Used to group related beacons. */
  uint8_t minNum[2]; /* Beacon minor number. Used to specify individual beacons within a group.*/
  uint8_t txPower; /* The Beacon's measured RSSI at 1 meter distance in dBm. See the iBeacon specification for measurement guidelines. */
} iBeaconData = {
/* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
  2, /* length  */
  0x01, /* type */
  0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */

/* Manufacturer specific data */
  26, /* length of field*/
  0xFF, /* type of field */

/* The first two data octets shall contain a company identifier code from
 * the Assigned Numbers - Company Identifiers document */
  { UINT16_TO_BYTES(0x004C) },

/* Beacon type */
/* 0x0215 is iBeacon */
  { UINT16_TO_BYTE1(0x0215), UINT16_TO_BYTE0(0x0215) },

/* 128 bit / 16 byte UUID - generated specially for the DMP Demo */
  { 0x00, 0x47, 0xe7, 0x0a, 0x5d, 0xc1, 0x47, 0x25, 0x87, 0x99, 0x83, 0x05, 0x44,
    0xae, 0x04, 0xf6 },

/* Beacon major number - not used for this application */
  { UINT16_TO_BYTE1(256), UINT16_TO_BYTE0(256) },

/* Beacon minor number  - not used for this application*/
  { UINT16_TO_BYTE1(0), UINT16_TO_BYTE0(0) },

/* The Beacon's measured RSSI at 1 meter distance in dBm */
/* 0xC3 is -61dBm */
// TBD: check?
  0xC3
};

static struct {
  uint8_t flagsLen; /**< Length of the Flags field. */
  uint8_t flagsType; /**< Type of the Flags field. */
  uint8_t flags; /**< Flags field. */
  uint8_t serLen; /**< Length of Complete list of 16-bit Service UUIDs. */
  uint8_t serType; /**< Complete list of 16-bit Service UUIDs. */
  uint8_t serviceList[2]; /**< Complete list of 16-bit Service UUIDs. */
  uint8_t serDataLength; /**< Length of Service Data. */
  uint8_t serDataType; /**< Type of Service Data. */
  uint8_t uuid[2]; /**< 16-bit Eddystone UUID. */
  uint8_t frameType; /**< Frame type. */
  uint8_t txPower; /**< The Beacon's measured RSSI at 0 meter distance in dBm. */
  uint8_t urlPrefix; /**< URL prefix type. */
  uint8_t url[10]; /**< URL. */
} eddystone_data = {
/* Flag bits - See Bluetooth 4.0 Core Specification , Volume 3, Appendix C, 18.1 for more details on flags. */
  2, /* length  */
  0x01, /* type */
  0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */
/* Service field length */
  0x03,
/* Service field type */
  0x03,
/* 16-bit Eddystone UUID */
  { UINT16_TO_BYTES(0xFEAA) },
/* Eddystone-TLM Frame length */
  0x10,
/* Service Data data type value */
  0x16,
/* 16-bit Eddystone UUID */
  { UINT16_TO_BYTES(0xFEAA) },
/* Eddystone-URL Frame type */
  0x10,
/* Tx power */
  0x00,
/* URL prefix - standard */
  0x00,
/* URL */
  { 's', 'i', 'l', 'a', 'b', 's', '.', 'c', 'o', 'm' }
};

static struct {
  uint16_t txDelayMs;
  uint8_t connHandle;
  uint16_t characteristicHandle;
  uint8_t size;
} bleTxTestParams;

struct responseData_t {
  uint8_t flagsLen; /**< Length of the Flags field. */
  uint8_t flagsType; /**< Type of the Flags field. */
  uint8_t flags; /**< Flags field. */
  uint8_t shortNameLen; /**< Length of Shortened Local Name. */
  uint8_t shortNameType; /**< Shortened Local Name. */
  uint8_t shortName[DEVNAME_LEN]; /**< Shortened Local Name. */
  uint8_t uuidLength; /**< Length of UUID. */
  uint8_t uuidType; /**< Type of UUID. */
  uint8_t uuid[UUID_LEN]; /**< 128-bit UUID. */
};

static struct responseData_t responseData = { 2, /* length (incl type) */
                                              0x01, /* type */
                                              0x04 | 0x02, /* Flags: LE General Discoverable Mode, BR/EDR is disabled. */
                                              DEVNAME_LEN + 1, // length of local name (incl type)
                                              0x08, // shortened local name
                                              { 'D', 'M', '0', '0', ':', '0', '0' },
                                              UUID_LEN + 1, // length of UUID data (incl type)
                                              0x06, // incomplete list of service UUID's
                                              // custom service UUID for silabs lamp in little-endian format
                                              { 0xc9, 0x1b, 0x80, 0x3d, 0x61, 0x50, 0x0c, 0x97, 0x8d, 0x45, 0x19,
                                                0x7d, 0x96, 0x5b, 0xe5, 0xba } };

static const AppCfgGattServerUserReadRequest_t appCfgGattServerUserReadRequest[] =
{
  { gattdb_light_state, readLightState },
  { gattdb_trigger_source, readTriggerSource },
  { gattdb_source_address, readSourceAddress },
  { 0, NULL }
};

static const AppCfgGattServerUserWriteRequest_t appCfgGattServerUserWriteRequest[] =
{
  { gattdb_light_state, writeLightState },
  { 0, NULL }
};

size_t appCfgGattServerUserReadRequestSize = COUNTOF(appCfgGattServerUserReadRequest) - 1;
size_t appCfgGattServerUserWriteRequestSize = COUNTOF(appCfgGattServerUserWriteRequest) - 1;

/**
 * Custom CLI.  This command tree is executed by typing "custom <command>"
 * See app/util/serial/command-interpreter2.h for more detail on writing commands.
 **/
/*  Example sub-menu */
//  extern void doSomethingFunction(void);
//  static EmberCommandEntry customSubMenu[] = {
//    emberCommandEntryAction("do-something", doSomethingFunction, "", "Do something description"),
//    emberCommandEntryTerminator()
//  };
//  extern void actionFunction(void);
static void readLightState(uint8_t connection)
{
  emberAfCorePrintln("Light state = %d\r\n", ble_lightState);
  /* Send response to read request */
  gecko_cmd_gatt_server_send_user_read_response(connection,
                                                gattdb_light_state,
                                                0,
                                                sizeof(ble_lightState),
                                                &ble_lightState);
}

static void readTriggerSource(uint8_t connection)
{
  emberAfCorePrintln("Last event = %d\r\n", ble_lastEvent);
  /* Send response to read request */
  gecko_cmd_gatt_server_send_user_read_response(connection,
                                                gattdb_trigger_source,
                                                0,
                                                sizeof(ble_lastEvent),
                                                &ble_lastEvent);
}

static void readSourceAddress(uint8_t connection)
{
  emberAfCorePrintln("readSourceAddress");
  /* Send response to read request */
  gecko_cmd_gatt_server_send_user_read_response(connection,
                                                gattdb_source_address,
                                                0,
                                                sizeof(SourceAddress),
                                                SourceAddress);
}

static void writeLightState(uint8_t connection, uint8array *writeValue)
{
  emberAfCorePrintln("Light state write; %d\r\n", writeValue->data[0]);
  lightDirection = DMP_UI_DIRECTION_BLUETOOTH;

  emberAfWriteAttribute(emberAfPrimaryEndpoint(),
                        ZCL_ON_OFF_CLUSTER_ID,
                        ZCL_ON_OFF_ATTRIBUTE_ID,
                        CLUSTER_MASK_SERVER,
                        (int8u *) &writeValue->data[0],
                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  gecko_cmd_gatt_server_send_user_write_response(connection,
                                                 gattdb_light_state,
                                                 ES_WRITE_OK);

  uint8_t index = bleConnectionInfoTableLookup(connection);

  if (index != 0xFF) {
    memset(SourceAddress, 0, sizeof(SourceAddress));
    for (int i = 0; i < SOURCE_ADDRESS_LEN - 2; i++) {
      SourceAddress[2 + i] =
        bleConnectionTable[index].remoteAddress[5 - i];
    }
  }
}

static void notifyLight(uint8_t lightState)
{
  ble_lightState = lightState;
  if (ble_lightState_config == GAT_RECEIVE_INDICATION) {
    emberAfCorePrintln("notifyLight : Light state = %d\r\n", lightState);
    /* Send notification/indication data */
    for (int i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
      if (bleConnectionTable[i].inUse
          && bleConnectionTable[i].connectionHandle) {
        gecko_cmd_gatt_server_send_characteristic_notification(
          bleConnectionTable[i].connectionHandle,
          gattdb_light_state,
          sizeof(lightState),
          &lightState);
      }
    }
  }
}

static void notifyTriggerSource(uint8_t connection, uint8_t triggerSource)
{
  if (ble_triggerSrc_config == GAT_RECEIVE_INDICATION) {
    emberAfCorePrintln("notifyTriggerSource :Last event = %d\r\n",
                       triggerSource);
    /* Send notification/indication data */
    gecko_cmd_gatt_server_send_characteristic_notification(connection,
                                                           gattdb_trigger_source,
                                                           sizeof(triggerSource),
                                                           &triggerSource);
  }
}

static void notifySourceAddress(uint8_t connection)
{
  if (ble_triggerSrc_config == GAT_RECEIVE_INDICATION) {
    /* Send notification/indication data */
    gecko_cmd_gatt_server_send_characteristic_notification(connection,
                                                           gattdb_source_address,
                                                           sizeof(SourceAddress),
                                                           SourceAddress);
  }
}

static void toggleOnoffAttribute(void)
{
  EmberStatus status;
  int8u data;
  status = emberAfReadAttribute(emberAfPrimaryEndpoint(),
                                ZCL_ON_OFF_CLUSTER_ID,
                                ZCL_ON_OFF_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (int8u*) &data,
                                sizeof(data),
                                NULL);

  if (status == EMBER_ZCL_STATUS_SUCCESS) {
    if (data == 0x00) {
      data = 0x01;
    } else {
      data = 0x00;
    }

    lightDirection = DMP_UI_DIRECTION_SWITCH;
    emberAfGetEui64(LightEUI);
    for (int i = 0; i < SOURCE_ADDRESS_LEN; i++) {
      SourceAddress[i] = LightEUI[(SOURCE_ADDRESS_LEN - 1) - i];
    }
  } else {
    emberAfAppPrintln("read onoff attr: 0x%x", status);
  }

  status = emberAfWriteAttribute(emberAfPrimaryEndpoint(),
                                 ZCL_ON_OFF_CLUSTER_ID,
                                 ZCL_ON_OFF_ATTRIBUTE_ID,
                                 CLUSTER_MASK_SERVER,
                                 (int8u *) &data,
                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  emberAfAppPrintln("write to onoff attr: 0x%x", status);
}

static void setDefaultReportEntry(void)
{
  EmberAfPluginReportingEntry reportingEntry;
  emberAfClearReportTableCallback();
  reportingEntry.direction = EMBER_ZCL_REPORTING_DIRECTION_REPORTED;
  reportingEntry.endpoint = emberAfPrimaryEndpoint();
  reportingEntry.clusterId = ZCL_ON_OFF_CLUSTER_ID;
  reportingEntry.attributeId = ZCL_ON_OFF_ATTRIBUTE_ID;
  reportingEntry.mask = CLUSTER_MASK_SERVER;
  reportingEntry.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
  reportingEntry.data.reported.minInterval = 0x0001;
  reportingEntry.data.reported.maxInterval = 0x0002;
  reportingEntry.data.reported.reportableChange = 0; // onoff is boolean type so it is unused
  emberAfPluginReportingConfigureReportedAttribute(&reportingEntry);
}

static bool startPjoinAndIdentifying(uint16_t identifyTime)
{
  EmberStatus status;
  EmberEUI64 wildcardEui64 =
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, };
  EmberKeyData centralizedKey = { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41,
                                  0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39 };

  emberAddTransientLinkKey(wildcardEui64, &centralizedKey);
  status = emberPermitJoining(identifyTime);

  emberAfWriteServerAttribute(emberAfPrimaryEndpoint(),
                              ZCL_IDENTIFY_CLUSTER_ID,
                              ZCL_IDENTIFY_TIME_ATTRIBUTE_ID,
                              (uint8_t *) &identifyTime,
                              sizeof(identifyTime));

  startIdentifyOnAllChildNodes(identifyTime);

  return (status == EMBER_SUCCESS);
}

static void startIdentifyOnAllChildNodes(uint16_t identifyTime)
{
  // Attempt to start Identify on all connected child nodes.

  const uint8_t childTableSize = emberAfGetChildTableSize();
  uint8_t i;

  // Iterate through the child table and try to find the device's child data
  for (i = 0; i < childTableSize; ++i) {
    EmberChildData childData;
    if (emberAfGetChildData(i, &childData) == EMBER_SUCCESS) {
      // Write Identify Time attribute on child.
      uint8_t outgoingBuffer[9];
      uint8_t idx = 0;
      EmberApsFrame apsFrame = { 0 };

      apsFrame.options = EMBER_AF_DEFAULT_APS_OPTIONS;
      apsFrame.clusterId = ZCL_IDENTIFY_CLUSTER_ID;
      apsFrame.sourceEndpoint = 0x01;
      apsFrame.destinationEndpoint = 0x01;

      outgoingBuffer[idx++] = ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER;
      outgoingBuffer[idx++] = emberAfNextSequence();
      outgoingBuffer[idx++] = ZCL_WRITE_ATTRIBUTES_COMMAND_ID;
      outgoingBuffer[idx++] = LOW_BYTE(ZCL_IDENTIFY_TIME_ATTRIBUTE_ID);
      outgoingBuffer[idx++] = HIGH_BYTE(ZCL_IDENTIFY_TIME_ATTRIBUTE_ID);
      outgoingBuffer[idx++] = ZCL_INT16U_ATTRIBUTE_TYPE;
      outgoingBuffer[idx++] = LOW_BYTE(identifyTime);
      outgoingBuffer[idx++] = HIGH_BYTE(identifyTime);

      emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                         childData.id,
                         &apsFrame,
                         idx,
                         outgoingBuffer);
    }
  }
}

void buttonEventHandler(void)
{
  EmberStatus status;
  emberEventControlSetInactive(buttonEventControl);

  EmberNetworkStatus state = emberAfNetworkState();
  if (lastButton == BUTTON0) {
    toggleOnoffAttribute();
  } else if (lastButton == BUTTON1) {
    if (state != EMBER_JOINED_NETWORK) {
      dmpUiDisplayZigBeeState(DMP_UI_FORMING);
      status = emberAfPluginNetworkCreatorStart(true); // centralized
      emberAfCorePrintln("%p network %p: 0x%X", "Form centralized", "start", status);
    } else {
      // joined on NWK
      if (longPress == false) {
        if (emberGetPermitJoining()) {
          emberPermitJoining(0);
        } else {
          if (startPjoinAndIdentifying(180)) {
            dmpUiZigBeePjoin(true);
            emberAfAppPrintln("pJoin for 180 sec: 0x%x", status);
          }
        }
      } else {
        status = emberLeaveNetwork();
        emberClearBindingTable();
        emberAfAppPrintln("leave NWK: 0x%x", status);
      }
    }
  }
}

void lcdMainMenuDisplayEventHandler(void)
{
  emberEventControlSetInactive(lcdMainMenuDisplayEventControl);
  dmpUiLightOff();
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
  dmpUiDisplayZigBeeState(DMP_UI_STATE_UNKNOWN); // UI will work out and display the present Nwk state.

  switch (status) {
    case EMBER_NETWORK_UP:
      setDefaultReportEntry();
      dmpUiDisplayZigBeeState(DMP_UI_NETWORK_UP);
      if (startPjoinAndIdentifying(180)) {
        dmpUiZigBeePjoin(true);
        emberAfCorePrintln("%p network %p: 0x%X", "Open", "for joining", status);
      }
      break;
    case EMBER_NETWORK_DOWN:
      dmpUiDisplayZigBeeState(DMP_UI_NO_NETWORK);
      emberEventControlSetInactive(lcdPermitJoinEventControl);
      longPress = false;
      break;
    default:
      break;
  }

  return false;
}

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
  bleConnectionInfoTableInit();
}

/** @brief On/off Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 * @param attributeId Attribute that changed  Ver.: always
 */
void emberAfOnOffClusterServerAttributeChangedCallback(int8u endpoint,
                                                       EmberAfAttributeId attributeId)
{
  EmberStatus status;
  int8u data;

  if (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) {
    status = emberAfReadAttribute(endpoint,
                                  ZCL_ON_OFF_CLUSTER_ID,
                                  ZCL_ON_OFF_ATTRIBUTE_ID,
                                  CLUSTER_MASK_SERVER,
                                  (int8u*) &data,
                                  sizeof(data),
                                  NULL);

    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      if (data == 0x00) {
        halClearLed(BOARDLED0);
        halClearLed(BOARDLED1);
        dmpUiLightOff();
        notifyLight(DMP_LIGHT_OFF);
      } else {
        halSetLed(BOARDLED0);
        halSetLed(BOARDLED1);
        notifyLight(DMP_LIGHT_ON);
        dmpUiLightOn();
      }
      if ((lightDirection == DMP_UI_DIRECTION_BLUETOOTH)
          || (lightDirection == DMP_UI_DIRECTION_SWITCH)) {
        dmpUiUpdateDirection(lightDirection);
      } else {
        lightDirection = DMP_UI_DIRECTION_ZIGBEE;
        dmpUiUpdateDirection(lightDirection);
        for (int i = 0; i < SOURCE_ADDRESS_LEN; i++) {
          SourceAddress[i] = SwitchEUI[(SOURCE_ADDRESS_LEN - 1) - i];
        }
      }
      ble_lastEvent = lightDirection;
      lightDirection = DMP_UI_DIRECTION_INVALID;
    }
  }
}

/** @brief
 *
 * This function is called from the BLE stack to notify the application of a
 * stack event.
 */
void emberAfPluginBleEventCallback(struct gecko_cmd_packet* evt)
{
  switch (BGLIB_MSG_ID(evt->header)) {
    /* This event indicates that a remote GATT client is attempting to read a value of an
     *  attribute from the local GATT database, where the attribute was defined in the GATT
     *  XML firmware configuration file to have type="user". */

    case gecko_evt_gatt_server_user_read_request_id:
      for (int i = 0; i < appCfgGattServerUserReadRequestSize; i++) {
        if ((appCfgGattServerUserReadRequest[i].charId
             == evt->data.evt_gatt_server_user_read_request.characteristic)
            && (appCfgGattServerUserReadRequest[i].fctn)) {
          appCfgGattServerUserReadRequest[i].fctn(
            evt->data.evt_gatt_server_user_read_request.connection);
        }
      }
      break;

    /* This event indicates that a remote GATT client is attempting to write a value of an
     * attribute in to the local GATT database, where the attribute was defined in the GATT
     * XML firmware configuration file to have type="user".  */

    case gecko_evt_gatt_server_user_write_request_id:
      for (int i = 0; i < appCfgGattServerUserWriteRequestSize; i++) {
        if ((appCfgGattServerUserWriteRequest[i].charId
             == evt->data.evt_gatt_server_characteristic_status.characteristic)
            && (appCfgGattServerUserWriteRequest[i].fctn)) {
          appCfgGattServerUserWriteRequest[i].fctn(
            evt->data.evt_gatt_server_user_read_request.connection,
            &(evt->data.evt_gatt_server_attribute_value.value));
        }
      }
      break;

    case gecko_evt_system_boot_id: {
      struct gecko_msg_system_hello_rsp_t *hello_rsp;
      struct gecko_msg_system_get_bt_address_rsp_t *get_address_rsp;

      // Call these two APIs upon boot for testing purposes.
      hello_rsp = gecko_cmd_system_hello();
      get_address_rsp = gecko_cmd_system_get_bt_address();
      emberAfCorePrintln("BLE hello: %s",
                         (hello_rsp->result == bg_err_success) ? "success" : "error");
      emberAfCorePrint("BLE address: ");
      printBleAddress(get_address_rsp->address.addr);
      emberAfCorePrintln("");
      // start advertising
      enableBleAdvertisements();
    }
    break;
    case gecko_evt_gatt_server_characteristic_status_id: {
      bool isTimerRunning = 0;
      struct gecko_msg_gatt_server_characteristic_status_evt_t *StatusEvt =
        (struct gecko_msg_gatt_server_characteristic_status_evt_t*) &(evt->data);
      if (StatusEvt->status_flags == GAT_SERVER_CONFIRMATION) {
        emberAfCorePrintln(
          "characteristic= %d , GAT_SERVER_CLIENT_CONFIG_FLAG = %d\r\n",
          StatusEvt->characteristic, StatusEvt->client_config_flags);
        if (StatusEvt->characteristic == gattdb_light_state) {
          notifyTriggerSource(StatusEvt->connection, ble_lastEvent);
        } else if (StatusEvt->characteristic == gattdb_trigger_source) {
          notifySourceAddress(StatusEvt->connection);
        }
      } else if (StatusEvt->status_flags == GAT_SERVER_CLIENT_CONFIG) {
        if (StatusEvt->characteristic == gattdb_light_state) {
          ble_lightState_config = StatusEvt->client_config_flags;
        } else if (StatusEvt->characteristic == gattdb_trigger_source) {
          ble_triggerSrc_config = StatusEvt->client_config_flags;
        } else if (StatusEvt->characteristic == gattdb_source_address) {
          ble_bleSrc_config = StatusEvt->client_config_flags;
        }
        emberAfCorePrintln(
          "SERVER : ble_lightState_config= %d , ble_triggerSrc_config = %d , ble_bleSrc_config = %d\r\n",
          ble_lightState_config,
          ble_triggerSrc_config,
          ble_bleSrc_config);
      }
    }
    break;
    case gecko_evt_le_connection_opened_id: {
      emberAfCorePrintln("gecko_evt_le_connection_opened_id \n");
      struct gecko_msg_le_connection_opened_evt_t *conn_evt =
        (struct gecko_msg_le_connection_opened_evt_t*) &(evt->data);
      uint8_t index = bleConnectionInfoTableFindUnused();
      if (index == 0xFF) {
        emberAfCorePrintln("MAX active BLE connections");
        assert(index < 0xFF);
      } else {
        bleConnectionTable[index].inUse = true;
        bleConnectionTable[index].isMaster = (conn_evt->master > 0);
        bleConnectionTable[index].connectionHandle = conn_evt->connection;
        bleConnectionTable[index].bondingHandle = conn_evt->bonding;
        memcpy(bleConnectionTable[index].remoteAddress,
               conn_evt->address.addr, 6);

        ActiveBleConnections++;
        gecko_cmd_le_connection_set_phy(conn_evt->connection, 2);
        enableBleAdvertisements();
        emberAfCorePrintln("BLE connection opened");
        bleConnectionInfoTablePrintEntry(index);
        emberAfCorePrintln("%d active BLE connection",
                           ActiveBleConnections);
      }
    }
    break;
    case gecko_evt_le_connection_phy_status_id: {
      // indicate the PHY that has been selected
      emberAfCorePrintln("now using the %dMPHY\r\n",
                         evt->data.evt_le_connection_phy_status.phy);
    }
    break;
    case gecko_evt_le_connection_closed_id: {
      struct gecko_msg_le_connection_closed_evt_t *conn_evt =
        (struct gecko_msg_le_connection_closed_evt_t*) &(evt->data);
      uint8_t index = bleConnectionInfoTableLookup(conn_evt->connection);
      assert(index < 0xFF);

      bleConnectionTable[index].inUse = false;

      --ActiveBleConnections;
      // restart advertising, set connectable
      enableBleAdvertisements();
      if (bleConnectionInfoTableIsEmpty()) {
        dmpUiBluetoothConnected(false);
      }
      emberAfCorePrintln(
        "BLE connection closed, handle=0x%x, reason=0x%2x : [%d] active BLE connection",
        conn_evt->connection, conn_evt->reason, ActiveBleConnections);
    }
    break;
    case gecko_evt_le_gap_scan_response_id: {
      struct gecko_msg_le_gap_scan_response_evt_t *scan_evt =
        (struct gecko_msg_le_gap_scan_response_evt_t*) &(evt->data);
      emberAfCorePrint("Scan response, address type=0x%x, address: ",
                       scan_evt->address_type);
      printBleAddress(scan_evt->address.addr);
      emberAfCorePrintln("");
    }
    break;
    case gecko_evt_sm_list_bonding_entry_id: {
      struct gecko_msg_sm_list_bonding_entry_evt_t * bonding_entry_evt =
        (struct gecko_msg_sm_list_bonding_entry_evt_t*) &(evt->data);
      emberAfCorePrint("Bonding handle=0x%x, address type=0x%x, address: ",
                       bonding_entry_evt->bonding,
                       bonding_entry_evt->address_type);
      printBleAddress(bonding_entry_evt->address.addr);
      emberAfCorePrintln("");
    }
    break;
    case gecko_evt_gatt_service_id: {
      struct gecko_msg_gatt_service_evt_t* service_evt =
        (struct gecko_msg_gatt_service_evt_t*) &(evt->data);
      uint8_t i;
      emberAfCorePrintln(
        "GATT service, conn_handle=0x%x, service_handle=0x%4x",
        service_evt->connection, service_evt->service);
      emberAfCorePrint("UUID=[");
      for (i = 0; i < service_evt->uuid.len; i++) {
        emberAfCorePrint("0x%x ", service_evt->uuid.data[i]);
      }
      emberAfCorePrintln("]");
    }
    break;
    case gecko_evt_gatt_characteristic_id: {
      struct gecko_msg_gatt_characteristic_evt_t* char_evt =
        (struct gecko_msg_gatt_characteristic_evt_t*) &(evt->data);
      uint8_t i;
      emberAfCorePrintln(
        "GATT characteristic, conn_handle=0x%x, char_handle=0x%2x, properties=0x%x",
        char_evt->connection,
        char_evt->characteristic,
        char_evt->properties);
      emberAfCorePrint("UUID=[");
      for (i = 0; i < char_evt->uuid.len; i++) {
        emberAfCorePrint("0x%x ", char_evt->uuid.data[i]);
      }
      emberAfCorePrintln("]");
    }
    break;
    case gecko_evt_gatt_characteristic_value_id: {
      struct gecko_msg_gatt_characteristic_value_evt_t* char_evt =
        (struct gecko_msg_gatt_characteristic_value_evt_t*) &(evt->data);
      uint8_t i;

      if (char_evt->att_opcode == gatt_handle_value_indication) {
        gecko_cmd_gatt_send_characteristic_confirmation(
          char_evt->connection);
      }
      emberAfCorePrintln(
        "GATT (client) characteristic value, handle=0x%x, characteristic=0x%2x, att_op_code=0x%x",
        char_evt->connection,
        char_evt->characteristic,
        char_evt->att_opcode);
      emberAfCorePrint("value=[");
      for (i = 0; i < char_evt->value.len; i++) {
        emberAfCorePrint("0x%x ", char_evt->value.data[i]);
      }
      emberAfCorePrintln("]");
    }
    break;
    case gecko_evt_gatt_server_attribute_value_id: {
      struct gecko_msg_gatt_server_attribute_value_evt_t* attr_evt =
        (struct gecko_msg_gatt_server_attribute_value_evt_t*) &(evt->data);
      EmberStatus status;
      uint8_t i;
      emberAfCorePrintln(
        "GATT (server) attribute value, handle=0x%x, attribute=0x%2x, att_op_code=0x%x",
        attr_evt->connection,
        attr_evt->attribute,
        attr_evt->att_opcode);
      emberAfCorePrint("value=[");
      for (i = 0; i < attr_evt->value.len; i++) {
        emberAfCorePrint("0x%x ", attr_evt->value.data[i]);
      }
      emberAfCorePrintln("]");
      // Forward the attribute over the ZigBee network.
      emberAfWriteAttribute(emberAfPrimaryEndpoint(),
                            ZCL_ON_OFF_CLUSTER_ID,
                            ZCL_ON_OFF_ATTRIBUTE_ID,
                            CLUSTER_MASK_SERVER,
                            (int8u *) attr_evt->value.data,
                            ZCL_BOOLEAN_ATTRIBUTE_TYPE);

      lightDirection = DMP_UI_DIRECTION_BLUETOOTH;
    }
    break;
    case gecko_evt_le_connection_parameters_id: {
      struct gecko_msg_le_connection_parameters_evt_t* param_evt =
        (struct gecko_msg_le_connection_parameters_evt_t*) &(evt->data);
      emberAfCorePrintln(
        "BLE connection parameters are updated, handle=0x%x, interval=0x%2x, latency=0x%2x, timeout=0x%2x, security=0x%x, txsize=0x%2x",
        param_evt->connection,
        param_evt->interval,
        param_evt->latency,
        param_evt->timeout,
        param_evt->security_mode,
        param_evt->txsize);
      dmpUiBluetoothConnected(true);
    }
    break;
    case gecko_evt_gatt_procedure_completed_id: {
      struct gecko_msg_gatt_procedure_completed_evt_t* proc_comp_evt =
        (struct gecko_msg_gatt_procedure_completed_evt_t*) &(evt->data);
      emberAfCorePrintln("BLE procedure completed, handle=0x%x, result=0x%2x",
                         proc_comp_evt->connection, proc_comp_evt->result);
    }
    break;
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
void emberAfHalButtonIsrCallback(int8u button, int8u state)
{
  static uint16_t buttonPressTime;
  uint16_t currentTime = 0;
  if (state == BUTTON_PRESSED) {
    if (button == BUTTON1) {
      buttonPressTime = halCommonGetInt16uMillisecondTick();
    }
  } else if (state == BUTTON_RELEASED) {
    if (button == BUTTON1) {
      currentTime = halCommonGetInt16uMillisecondTick();

      if ((currentTime - buttonPressTime) > BUTTON_LONG_PRESS_TIME_MSEC) {
        longPress = true;
      }
    }
    lastButton = button;
    emberEventControlSetActive(buttonEventControl);
  }
}

/** @brief Pre Command Received
 *
 * This callback is the second in the Application Framework's message processing
 * chain. At this point in the processing of incoming over-the-air messages, the
 * application has determined that the incoming message is a ZCL command. It
 * parses enough of the message to populate an EmberAfClusterCommand struct. The
 * Application Framework defines this struct value in a local scope to the
 * command processing but also makes it available through a global pointer
 * called emberAfCurrentCommand, in app/framework/util/util.c. When command
 * processing is complete, this pointer is cleared.
 *
 * @param cmd   Ver.: always
 */
boolean emberAfPreCommandReceivedCallback(EmberAfClusterCommand* cmd)
{
  if ((cmd->commandId == ZCL_ON_COMMAND_ID)
      || (cmd->commandId == ZCL_OFF_COMMAND_ID)
      || (cmd->commandId == ZCL_TOGGLE_COMMAND_ID)) {
    memset(SwitchEUI, 0, SOURCE_ADDRESS_LEN);
    emberLookupEui64ByNodeId(cmd->source, SwitchEUI);
    emberAfCorePrintln(
      "SWITCH ZCL toggle/on/off EUI [%x %x %x %x %x %x %x %x]",
      SwitchEUI[7],
      SwitchEUI[6],
      SwitchEUI[5],
      SwitchEUI[4],
      SwitchEUI[3],
      SwitchEUI[2],
      SwitchEUI[1],
      SwitchEUI[0]);
  }
  return false;
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or ezspTrustCenterJoinHandler.
 * This callback provides the same arguments passed to the
 * TrustCenterJoinHandler. For more information about the TrustCenterJoinHandler
 * please see documentation included in stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  if (status == EMBER_DEVICE_LEFT) {
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++) {
      EmberBindingTableEntry entry;
      emberGetBinding(i, &entry);
      if ((entry.type == EMBER_UNICAST_BINDING)
          && (entry.clusterId == ZCL_ON_OFF_CLUSTER_ID)
          && ((MEMCOMPARE(entry.identifier, newNodeEui64, EUI64_SIZE)
               == 0))) {
        emberDeleteBinding(i);
        emberAfAppPrintln("deleted binding entry: %d", i);
        break;
      }
    }
  }
}

void bleEventHandler(void)
{
  emberEventControlSetDelayMS(bleEventControl, bleNotificationsPeriodMs);
}

#ifdef ENABLE_CUSTOM_COMMANDS
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
#if defined(DMP_DEBUG)
  emberAfCorePrintln("BLE notifications enabled");
#endif //DMP_DEBUG
  bleEventHandler();
}

static void disableBleNotificationsCommand(void)
{
#if defined(DMP_DEBUG)
  emberAfCorePrintln("BLE notifications disabled");
#endif //DMP_DEBUG
  emberEventControlSetInactive(bleEventControl);
}
#endif

void bleTxEventHandler(void);

EmberCommandEntry emberAfCustomCommands[] = {
#ifdef ENABLE_CUSTOM_COMMANDS
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
#endif //ENABLE_CUSTOM_COMMANDS
  emberCommandEntryTerminator(),
};

//------------------------------------------------------------------------------

// BLE connection info table functions
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

static bool bleConnectionInfoTableIsEmpty(void)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS; i++) {
    if (bleConnectionTable[i].inUse) {
      return false;
    }
  }
  return true;
}

static void bleConnectionInfoTablePrintEntry(uint8_t index)
{
  assert(index < EMBER_AF_PLUGIN_BLE_MAX_CONNECTIONS
         && bleConnectionTable[index].inUse);
  emberAfCorePrintln("**** Connection Info index[%d]****", index);
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
  emberAfCorePrint("[%x %x %x %x %x %x]", address[5], address[4], address[3],
                   address[2], address[1], address[0]);
}

//------------------------------------------------------------------------------

void bleTxEventHandler(void)
{
  uint8_t txData[BLE_TX_TEST_DATA_SIZE];
  uint8_t i;

  for (i = 0; i < BLE_TX_TEST_DATA_SIZE; i++) {
    txData[i] = i;
  }

  gecko_cmd_gatt_write_characteristic_value_without_response(
    bleTxTestParams.connHandle,
    bleTxTestParams.characteristicHandle,
    BLE_TX_TEST_DATA_SIZE,
    txData);

  emberEventControlSetDelayMS(bleTxEventControl, bleTxTestParams.txDelayMs);
}

static void BeaconAdvertisements(uint16_t devId)
{
  static uint8_t *advData;
  static uint8_t advDataLen;

  iBeaconData.minNum[0] = UINT16_TO_BYTE1(devId);
  iBeaconData.minNum[1] = UINT16_TO_BYTE0(devId);

  advData = (uint8_t*) &iBeaconData;
  advDataLen = sizeof(iBeaconData);
  /* Set custom advertising data */
  gecko_cmd_le_gap_bt5_set_adv_data(HANDLE_IBEACON, 0, advDataLen, advData);
  gecko_cmd_le_gap_bt5_set_adv_parameters(HANDLE_IBEACON, 160, 160, 7, 0);
  gecko_cmd_le_gap_bt5_set_mode(HANDLE_IBEACON,
                                le_gap_user_data,
                                le_gap_non_connectable,
                                0,
                                le_gap_non_resolvable);

  advData = (uint8_t*) &eddystone_data;
  advDataLen = sizeof(eddystone_data);
  /* Set custom advertising data */
  gecko_cmd_le_gap_bt5_set_adv_data(HANDLE_EDDYSTONE, 0, advDataLen, advData);
  gecko_cmd_le_gap_bt5_set_adv_parameters(HANDLE_EDDYSTONE, 160, 160, 7, 0);
  gecko_cmd_le_gap_bt5_set_mode(HANDLE_EDDYSTONE,
                                le_gap_user_data,
                                le_gap_non_connectable,
                                0,
                                le_gap_non_resolvable);
}

static void enableBleAdvertisements(void)
{
  /* set transmit power to 0 dBm */
  gecko_cmd_system_set_tx_power(0);

  /* Create the device name based on the 16-bit device ID */
  uint16_t devId;
  struct gecko_msg_system_get_bt_address_rsp_t* btAddr;
  btAddr = gecko_cmd_system_get_bt_address();
  devId = *((uint16*) (btAddr->address.addr));

  // Copy to the local GATT database - this will be used by the BLE stack
  // to put the local device name into the advertisements, but only if we are
  // using default advertisements
  static char devName[DEVNAME_LEN];
  snprintf(devName, DEVNAME_LEN, DEVNAME, devId >> 8, devId & 0xff);
  emberAfCorePrintln("devName = %s", devName);
  gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name,
                                              0,
                                              strlen(devName),
                                              (uint8_t *) devName);
  dmpUiSetBleDeviceName(devName);

  // Copy the shortened device name to the response data, overwriting
  // the default device name which is set at compile time
  MEMCOPY(((uint8_t*) &responseData) + 5, devName, 8);

  // Set the response data
  struct gecko_msg_le_gap_bt5_set_adv_data_rsp_t *rsp;
  rsp = gecko_cmd_le_gap_bt5_set_adv_data(HANDLE_DEMO,
                                          0,
                                          sizeof(responseData),
                                          (uint8_t*) &responseData);

  // Set nominal 100ms advertising interval, so we just get
  // a single beacon of each type
  gecko_cmd_le_gap_bt5_set_adv_parameters(HANDLE_DEMO, 160, 160, 7, 1);
  /* Start advertising in user mode and enable connections*/
  gecko_cmd_le_gap_bt5_set_mode(HANDLE_DEMO,
                                le_gap_user_data,
                                le_gap_undirected_connectable,
                                0,
                                le_gap_identity_address);

  emberAfCorePrintln("BLE custom advertisements enabled");
  BeaconAdvertisements(devId);
}
