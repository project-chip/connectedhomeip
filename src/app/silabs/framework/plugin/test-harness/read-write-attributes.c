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
 * @brief Attribute definitions for the Test Harness plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"

#include "test-harness.h"

//------------------------------------------------------------------------------
// Globals

EmberEventControl emberAfPluginTestHarnessReadWriteAttributesTimeoutEventControl;

typedef enum {
  READ_WRITE_TEST_STATE_NONE,
  READ_WRITE_TEST_STATE_READ_SENT,
  READ_WRITE_TEST_STATE_WRITE_SENT,
} ReadWriteState;

typedef enum {
  TEST_TYPE_READ_WRITE_ATTRIBUTES = 0,
  TEST_TYPE_BIG_ATTRIBUTE_READ    = 1,
} TestType;

#define TEST_TYPE_MAX TEST_TYPE_BIG_ATTRIBUTE_READ

static const char* testTypeStrings[] = {
  "Read Write Multiple attributes",
  "Big Attribute Read (requiring fragmentation)",
  NULL,
};

TestType testType = TEST_TYPE_READ_WRITE_ATTRIBUTES;

static ReadWriteState readWriteState = READ_WRITE_TEST_STATE_NONE;

static EmberNodeId destAddress = EMBER_NULL_NODE_ID;
static uint8_t sourceEndpoint = 1;
static uint8_t destEndpoint;
static uint16_t testClusterId;
static uint16_t attributeIdStart = 0;
static uint16_t attributeIdEnd = 0;
static bool clientToServer = true;

static uint16_t currentIndex;

void setDestinationCommand(void);

const char* setDestCommandArguments[] = {
  "dest-address",
  "dest-ep",
  NULL,
};

const char* startTestArguments[] = {
  "cluster-id",
  "attribute-id-start",
  "attribute-id-end",
  "client-to-server",
  NULL,
};

const char* optionCommandArguments[] = {
  "test-type",
  "timeout-in-sec",
  NULL,
};

static uint16_t timeoutSeconds = 3;

void startTestCommand(void);
void setOptionsCommand(void);
static void attributeTestStatusCommand(void);

EmberCommandEntry emAfReadWriteAttributeTestCommands[] = {
  emberCommandEntryActionWithDetails("set-dest",
                                     setDestinationCommand,
                                     "vu",
                                     "Set Attribute Test Destination address and endpoint",
                                     setDestCommandArguments),
  emberCommandEntryActionWithDetails("start-test",
                                     startTestCommand,
                                     "vvvu",
                                     "Start read-write attribute test",
                                     startTestArguments),
  emberCommandEntryActionWithDetails("options",
                                     setOptionsCommand,
                                     "uv",
                                     "Set test options",
                                     optionCommandArguments),
  emberCommandEntryAction("status",
                          attributeTestStatusCommand,
                          "",
                          "Print the current test parameters."),
  emberCommandEntryTerminator(),
};

#define UNKNOWN_RESULT 0xFF
typedef struct  {
  uint8_t readResult;
  uint8_t writeResult;
  uint8_t attributeType;
} SingleAttributeTestResults;

#define MAX_ATTRIBUTE_IDS 100
static SingleAttributeTestResults attributeResults[MAX_ATTRIBUTE_IDS];

typedef struct {
  uint8_t status;
  const char* string;
} ZclStatusToStringMap;

const ZclStatusToStringMap zclStatusToStringMap[] = {
  { EMBER_ZCL_STATUS_SUCCESS, "Success         " },
  { EMBER_ZCL_STATUS_FAILURE, "Failure         " },
  { EMBER_ZCL_STATUS_REQUEST_DENIED, "Request Denied  " },
  { EMBER_ZCL_STATUS_MULTIPLE_REQUEST_NOT_ALLOWED, "Mult Not Allowed" },
  { EMBER_ZCL_STATUS_INDICATION_REDIRECTION_TO_AP, "Indc Redir to AP" },
  { EMBER_ZCL_STATUS_PREFERENCE_DENIED, "Pref Denied     " },
  { EMBER_ZCL_STATUS_PREFERENCE_IGNORED, "Pref Ignored    " },
  { EMBER_ZCL_STATUS_NOT_AUTHORIZED, "Not Authorized  " },
  { EMBER_ZCL_STATUS_RESERVED_FIELD_NOT_ZERO, "Reserv Non-Zero " },
  { EMBER_ZCL_STATUS_MALFORMED_COMMAND, "Malformed Comm  " },
  { EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND, "Unsup Clust Cmd " },
  { EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND, "Unsup Gen Cmd   " },
  { EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND, "Uns MFG Clus Cmd" },
  { EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND, "Uns MFG Gen Cmd " },
  { EMBER_ZCL_STATUS_INVALID_FIELD, "Invalid Field   " },
  { EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE, "Unsupported Attr" },
  { EMBER_ZCL_STATUS_INVALID_VALUE, "Invalid Value   " },
  { EMBER_ZCL_STATUS_READ_ONLY, "Read Only       " },
  { EMBER_ZCL_STATUS_INSUFFICIENT_SPACE, "Insuff Space    " },
  { EMBER_ZCL_STATUS_DUPLICATE_EXISTS, "Duplicate Exists" },
  { EMBER_ZCL_STATUS_NOT_FOUND, "Not found       " },
  { EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE, "Unreportable Att" },
  { EMBER_ZCL_STATUS_INVALID_DATA_TYPE, "Invalid Data Typ" },
  { EMBER_ZCL_STATUS_INVALID_SELECTOR, "Invalid selector" },
  { EMBER_ZCL_STATUS_WRITE_ONLY, "Write Only      " },
  { EMBER_ZCL_STATUS_INCONSISTENT_STARTUP_STATE, "Inconsis Startup" },
  { EMBER_ZCL_STATUS_DEFINED_OUT_OF_BAND, "Def Out of Band " },
  { EMBER_ZCL_STATUS_ABORT, "Abort           " },
  { EMBER_ZCL_STATUS_INVALID_IMAGE, "Invalid Image   " },
  { EMBER_ZCL_STATUS_WAIT_FOR_DATA, "Wait for data   " },
  { EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE, "No image avail  " },
  { EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE, "Require more img" },
  { EMBER_ZCL_STATUS_HARDWARE_FAILURE, "Hardware failure" },
  { EMBER_ZCL_STATUS_SOFTWARE_FAILURE, "Software failure" },
  { EMBER_ZCL_STATUS_CALIBRATION_ERROR, "Calibration err " },

  { 0, NULL },  // terminator
};
static const char unknownErrorString[] =           "???             ";

typedef struct {
  uint8_t type;
  const char* string;
} ZclAttributeTypeToStringMap;

ZclAttributeTypeToStringMap zclAttributeTypeToStringMap[] = {
  { ZCL_NO_DATA_ATTRIBUTE_TYPE, "No Data        " },
  { ZCL_DATA8_ATTRIBUTE_TYPE, "Data 8         " },
  { ZCL_DATA16_ATTRIBUTE_TYPE, "Data 16        " },
  { ZCL_DATA24_ATTRIBUTE_TYPE, "Data 24        " },
  { ZCL_DATA32_ATTRIBUTE_TYPE, "Data 32        " },
  { ZCL_DATA40_ATTRIBUTE_TYPE, "Data 40        " },
  { ZCL_DATA48_ATTRIBUTE_TYPE, "Data 48        " },
  { ZCL_DATA56_ATTRIBUTE_TYPE, "Data 56        " },
  { ZCL_DATA64_ATTRIBUTE_TYPE, "Data 64        " },
  { ZCL_BOOLEAN_ATTRIBUTE_TYPE, "Boolean        " },
  { ZCL_BITMAP8_ATTRIBUTE_TYPE, "Bitmap 8       " },
  { ZCL_BITMAP16_ATTRIBUTE_TYPE, "Bitmap 16      " },
  { ZCL_BITMAP24_ATTRIBUTE_TYPE, "Bitmap 24      " },
  { ZCL_BITMAP32_ATTRIBUTE_TYPE, "Bitmap 32      " },
  { ZCL_BITMAP40_ATTRIBUTE_TYPE, "Bitmap 40      " },
  { ZCL_BITMAP48_ATTRIBUTE_TYPE, "Bitmap 48      " },
  { ZCL_BITMAP56_ATTRIBUTE_TYPE, "Bitmap 56      " },
  { ZCL_BITMAP64_ATTRIBUTE_TYPE, "Bitmap 64      " },
  { ZCL_INT8U_ATTRIBUTE_TYPE, "Unsigned Int 8 " },
  { ZCL_INT16U_ATTRIBUTE_TYPE, "Unsigned Int 16" },
  { ZCL_INT24U_ATTRIBUTE_TYPE, "Unsigned Int 24" },
  { ZCL_INT32U_ATTRIBUTE_TYPE, "Unsigned Int 32" },
  { ZCL_INT40U_ATTRIBUTE_TYPE, "Unsigned Int 40" },
  { ZCL_INT48U_ATTRIBUTE_TYPE, "Unsigned Int 48" },
  { ZCL_INT56U_ATTRIBUTE_TYPE, "Unsigned Int 56" },
  { ZCL_INT64U_ATTRIBUTE_TYPE, "Unsigned Int 64" },
  { ZCL_INT8S_ATTRIBUTE_TYPE, "Signed Int 8   " },
  { ZCL_INT16S_ATTRIBUTE_TYPE, "Signed Int 16  " },
  { ZCL_INT24S_ATTRIBUTE_TYPE, "Signed Int 24  " },
  { ZCL_INT32S_ATTRIBUTE_TYPE, "Signed Int 32  " },
  { ZCL_INT40S_ATTRIBUTE_TYPE, "Signed Int 40  " },
  { ZCL_INT48S_ATTRIBUTE_TYPE, "Signed Int 48  " },
  { ZCL_INT56S_ATTRIBUTE_TYPE, "Signed Int 56  " },
  { ZCL_INT64S_ATTRIBUTE_TYPE, "Signed Int 64  " },
  { ZCL_ENUM8_ATTRIBUTE_TYPE, "Enum 8         " },
  { ZCL_ENUM16_ATTRIBUTE_TYPE, "Enum 16        " },
  { ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE, "Float semi     " },
  { ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE, "Float Single   " },
  { ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE, "Float Double   " },
  { ZCL_OCTET_STRING_ATTRIBUTE_TYPE, "Octet String   " },
  { ZCL_CHAR_STRING_ATTRIBUTE_TYPE, "Char String    " },
  { ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE, "Octet string   " },
  { ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE, "Long Char Str  " },
  { ZCL_ARRAY_ATTRIBUTE_TYPE, "Array Attribute" },
  { ZCL_STRUCT_ATTRIBUTE_TYPE, "Struct Attrib  " },
  { ZCL_SET_ATTRIBUTE_TYPE, "Set Attribute  " },
  { ZCL_BAG_ATTRIBUTE_TYPE, "Bag Attribute  " },
  { ZCL_TIME_OF_DAY_ATTRIBUTE_TYPE, "Time of Day Att" },
  { ZCL_DATE_ATTRIBUTE_TYPE, "Date Attribute " },
  { ZCL_UTC_TIME_ATTRIBUTE_TYPE, "UTC Time Attr  " },
  { ZCL_CLUSTER_ID_ATTRIBUTE_TYPE, "Cluster ID Attr" },
  { ZCL_ATTRIBUTE_ID_ATTRIBUTE_TYPE, "Attribute ID   " },
  { ZCL_BACNET_OID_ATTRIBUTE_TYPE, "Bacnet OID     " },
  { ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE, "Ieee Address   " },
  { ZCL_SECURITY_KEY_ATTRIBUTE_TYPE, "Security Key   " },
  { ZCL_UNKNOWN_ATTRIBUTE_TYPE, "Unknown        " },
  { 0, NULL }  // terminator
};
static const char unknownAttributeTypeString[] = "???            ";

#define SOURCE_ENDPOINT 1

//------------------------------------------------------------------------------
// Forward Declarations

static void sendReadAttribute(void);
static void bigReadCommand(void);

//------------------------------------------------------------------------------
// Functions

void setOptionsCommand(void)
{
  uint8_t temp = (uint8_t)emberUnsignedCommandArgument(0);
  if (temp > TEST_TYPE_MAX) {
    uint8_t i;
    emberAfCorePrintln("Error: Invalid test type number.  Valid numbers are:");
    for (i = 0; i <= TEST_TYPE_MAX; i++) {
      emberAfCorePrintln("%d: %p", i, testTypeStrings[i]);
    }
    return;
  }

  timeoutSeconds = (uint16_t)emberUnsignedCommandArgument(1);
  testType = temp;
}

void setDestinationCommand(void)
{
  destAddress = (EmberNodeId)emberUnsignedCommandArgument(0);
  destEndpoint = (uint8_t)emberUnsignedCommandArgument(1);
}

static void readWriteAttributeTest(void)
{
  uint16_t count = attributeIdEnd - attributeIdStart + 1;
  if (count > MAX_ATTRIBUTE_IDS) {
    emberAfCorePrintln("Error: Too many attributes to test (%d > %d).",
                       count,
                       MAX_ATTRIBUTE_IDS);
    return;
  }
  MEMSET(attributeResults,
         UNKNOWN_RESULT,
         sizeof(SingleAttributeTestResults) * MAX_ATTRIBUTE_IDS);
  currentIndex = 0;
  emberAfCorePrintln("Testing cluster 0x%2X, attributes 0x%2X -> 0x%2X",
                     testClusterId,
                     attributeIdStart,
                     attributeIdEnd);
  sendReadAttribute();
}

void startTestCommand(void)
{
  testClusterId = (uint16_t)emberUnsignedCommandArgument(0);
  attributeIdStart = (uint16_t)emberUnsignedCommandArgument(1);
  attributeIdEnd = (uint16_t)emberUnsignedCommandArgument(2);
  clientToServer = (bool)emberUnsignedCommandArgument(3);
  if (attributeIdStart > attributeIdEnd) {
    emberAfCorePrintln("Error:  Start ID must be less than or equal to end ID.");
    return;
  }

  if (testType == TEST_TYPE_READ_WRITE_ATTRIBUTES) {
    readWriteAttributeTest();
  } else if (testType == TEST_TYPE_BIG_ATTRIBUTE_READ) {
    bigReadCommand();
  } else {
    emberAfCorePrintln("Error: Invalid test type %d", testType);
  }
}

static const char* gettAttributeTypeString(uint8_t attributeType)
{
  uint8_t i = 0;
  while (zclAttributeTypeToStringMap[i].string != NULL) {
    if (zclAttributeTypeToStringMap[i].type == attributeType) {
      return zclAttributeTypeToStringMap[i].string;
    }
    i++;
  }
  return unknownAttributeTypeString;
}

static const char* getErrorString(uint8_t status)
{
  uint8_t i = 0;
  while (zclStatusToStringMap[i].string != NULL) {
    if (zclStatusToStringMap[i].status == status) {
      return zclStatusToStringMap[i].string;
    }
    i++;
  }
  return unknownErrorString;
}

static void printResults(void)
{
  uint16_t i;
  uint16_t index = 0;
  emberAfCorePrintln("Cluster: 0x%2X\n", testClusterId);
  emberAfCorePrintln("Attr    Read                     Type                    Write");
  emberAfCorePrintln("-------------------------------------------------------------------------------");
  for (i = attributeIdStart; i <= attributeIdEnd; i++) {
    emberAfCorePrintln("0x%2X: 0x%X [%p]  0x%X [%p]  0x%X [%p]",
                       i,
                       attributeResults[index].readResult,
                       getErrorString(attributeResults[index].readResult),
                       attributeResults[index].attributeType,
                       gettAttributeTypeString(attributeResults[index].attributeType),
                       attributeResults[index].writeResult,
                       getErrorString(attributeResults[index].writeResult));
    index++;
  }
}

bool checkTestComplete(void)
{
  if ((attributeIdStart + currentIndex) > attributeIdEnd) {
    emberAfCorePrintln("Done.\n");
    printResults();
    emberEventControlSetInactive(emberAfPluginTestHarnessReadWriteAttributesTimeoutEventControl);
    return true;
  }
  return false;
}

static void sendCommand(bool read)
{
  emberAfSetCommandEndpoints(sourceEndpoint, destEndpoint);
  EmberStatus status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, destAddress);
  emberAfCorePrintln("%p Attr 0x%2X",
                     (read
                      ? "Read "
                      : "Write"),
                     attributeIdStart + currentIndex);
  if (status == EMBER_SUCCESS) {
    emberEventControlSetDelayQS(emberAfPluginTestHarnessReadWriteAttributesTimeoutEventControl,
                                timeoutSeconds << 2);
  } else {
    emberAfCorePrintln("Error: Failed to send command (0x%X)", status);
  }
}

static void sendReadAttribute(void)
{
  uint8_t temp[2];
  temp[0] = LOW_BYTE(attributeIdStart + currentIndex);
  temp[1] = HIGH_BYTE(attributeIdStart + currentIndex);
  emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND
                             | (clientToServer
                                ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)),
                            testClusterId,
                            ZCL_READ_ATTRIBUTES_COMMAND_ID,
                            "b",
                            temp,
                            2);
  sendCommand(true);
  readWriteState = READ_WRITE_TEST_STATE_READ_SENT;
}

static void sendWriteAttribute(uint8_t type,
                               uint8_t* data,
                               uint16_t dataLen)
{
  emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND
                             | (clientToServer
                                ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)),
                            testClusterId, \
                            ZCL_WRITE_ATTRIBUTES_COMMAND_ID,
                            "b",
                            data,
                            dataLen);

  sendCommand(false);  // read?
  readWriteState = READ_WRITE_TEST_STATE_WRITE_SENT;
}

void emberAfPluginTestHarnessReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                                            uint8_t * buffer,
                                                            uint16_t bufLen)
{
  if (clusterId == testClusterId
      && readWriteState == READ_WRITE_TEST_STATE_READ_SENT) {
    uint8_t result = buffer[2];
    uint16_t attributeId = (buffer[0] + (buffer[1] << 8));
    if (attributeId != (currentIndex + attributeIdStart)) {
      // Might be an APS retry, ignore.
      return;
    }
    emberEventControlSetInactive(emberAfPluginTestHarnessReadWriteAttributesTimeoutEventControl);
    attributeResults[currentIndex].readResult = result;
    if (result == EMBER_ZCL_STATUS_SUCCESS) {
      uint8_t type = buffer[3];
      attributeResults[currentIndex].attributeType = type;
      // A very crude way to change the attribute data without knowing
      // what we are writing.  We change the 0th bit of the first byte.
      // This is unlikely to be outside the valid range for the attribute
      // and yet is different than the current value.

      // The read attributes response is:
      //   Attribute ID (2-bytes)
      //   Status (1-byte)
      //   Type (1-byte)
      //   Value (variable)

      // We need to change it into a write attributes request:
      //   Attribute ID (2-bytes)
      //   Type (1-byte)
      //   Value (variable)

      // So we just shift the Attribute ID forward 1-byte to
      // avoid an overlapping MEMMOVE().  The size of the value
      // may be quite large, so better to let it stay put.

      buffer[1] = LOW_BYTE(attributeId);
      buffer[2] = HIGH_BYTE(attributeId);
      buffer[4] = (buffer[4] & 1
                   ? (buffer[4] & 0xFE)
                   : (buffer[4] | 0x01));
      sendWriteAttribute(type, &buffer[1], bufLen - 1);
    } else {
      currentIndex++;
      if (!checkTestComplete()) {
        sendReadAttribute();
      }
    }
  }
}

void emberAfPluginTestHarnessWriteAttributesResponseCallback(EmberAfClusterId clusterId,
                                                             uint8_t * buffer,
                                                             uint16_t bufLen)
{
  if (clusterId == testClusterId
      && readWriteState == READ_WRITE_TEST_STATE_WRITE_SENT) {
    uint16_t attributeId = (buffer[1] + (buffer[2] << 8));
    if (attributeId != (currentIndex + attributeIdStart)) {
      return;
    }
    emberEventControlSetInactive(emberAfPluginTestHarnessReadWriteAttributesTimeoutEventControl);
    attributeResults[currentIndex].writeResult = buffer[0];
    currentIndex++;
    if (!checkTestComplete()) {
      sendReadAttribute();
    }
  }
}

void emberAfPluginTestHarnessReadWriteAttributesTimeoutEventHandler(void)
{
  if (readWriteState == READ_WRITE_TEST_STATE_WRITE_SENT
      || readWriteState == READ_WRITE_TEST_STATE_READ_SENT) {
    emberAfCorePrintln("Timeout %p attribute",
                       (readWriteState == READ_WRITE_TEST_STATE_WRITE_SENT
                        ? "writing"
                        : "reading"));
    currentIndex++;
  }

  if (!checkTestComplete()) {
    sendReadAttribute();
  }
}

static void attributeTestStatusCommand(void)
{
  emberAfCorePrintln("Test Type: %p\n", testTypeStrings[testType]);
  emberAfCorePrintln("Test Target Destination Node ID: 0x%2X", destAddress);
  emberAfCorePrintln("Test Target Destination EP:      %d", destEndpoint);
}

static void bigReadCommand(void)
{
  uint16_t currentAttributeId;
  EmberStatus status;

  #if !defined(EMBER_AF_PLUGIN_FRAGMENTATION)
  emberAfCorePrintln("Warning:  Fragmentation plugin not enabled.  Will not be able to fit many attributes.");
  #endif

  if (emAfZclBufferLen < (EMBER_AF_ZCL_OVERHEAD
                          + ((attributeIdEnd - attributeIdStart) << 1))) {
    emberAfCorePrintln("Error:  Too many attributes (%d) to fit into super buffer (max attributes: %d)",
                       (attributeIdEnd + 1 - attributeIdStart),
                       // 3 bytes = Overhead
                       //   ZCL Frame control (1-byte)
                       //   ZCL Command ID (1-byte)
                       //   Sequence Number
                       (emAfZclBufferLen - EMBER_AF_ZCL_OVERHEAD) >> 1);  // each attribute is 2-bytes
    return;
  }

  emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND
                             | (clientToServer
                                ? ZCL_FRAME_CONTROL_CLIENT_TO_SERVER
                                : ZCL_FRAME_CONTROL_SERVER_TO_CLIENT)),
                            testClusterId,
                            ZCL_READ_ATTRIBUTES_COMMAND_ID,
                            "");
  for (currentAttributeId = attributeIdStart;
       currentAttributeId <= attributeIdEnd
       && *emAfResponseLengthPtr < emAfZclBufferLen;
       currentAttributeId++) {
    emAfZclBuffer[(*emAfResponseLengthPtr)++] = LOW_BYTE(currentAttributeId);
    emAfZclBuffer[(*emAfResponseLengthPtr)++] = HIGH_BYTE(currentAttributeId);
  }

  emberAfSetCommandEndpoints(SOURCE_ENDPOINT, destEndpoint);
  status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                     destAddress);
  emberAfCorePrintln("Sent read attributes for cluster 0x%2X, attributes: %d, status: 0x%X",
                     testClusterId,
                     attributeIdEnd - attributeIdStart,
                     status);
}
