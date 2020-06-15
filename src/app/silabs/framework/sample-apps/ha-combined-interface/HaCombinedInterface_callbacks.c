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
#include "app/framework/util/util.h"

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
      lightId = result->matchAddress;
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

  // if we are not part of the network, clear the light id; if not
  // already done so.
  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    if (lightId != EMBER_NULL_NODE_ID) {
      lightId = EMBER_NULL_NODE_ID;
      foundLight = false;
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

/** @brief Configure Reporting Response
 *
 * This function is called by the application framework when a Configure
 * Reporting Response command is received from an external device.  The
 * application should return true if the message was processed or false if it
 * was not.
 *
 * @param clusterId The cluster identifier of this response.
 * @param buffer Buffer containing the list of attribute status records.
 * @param bufLen The length in bytes of the list.
 */
bool emberAfConfigureReportingResponseCallback(EmberAfClusterId clusterId,
                                               uint8_t *buffer,
                                               uint16_t bufLen)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_REPORTING)
  uint16_t bufIndex = 0;
  emberAfReportingPrint("%p_RESP: ", "CFG_RPT");
  emberAfReportingDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfReportingPrintln("");
  emberAfReportingFlush();

  // Each record in the response has a one-byte status.  If the status is not
  // SUCCESS, the record will also contain a one-byte direction and a two-byte
  // attribute id.
  while (bufIndex + 1 <= bufLen) {
    EmberAfStatus status = (EmberAfStatus)emberAfGetInt8u(buffer,
                                                          bufIndex,
                                                          bufLen);
    bufIndex++;
    emberAfReportingPrintln(" - status:%x", status);
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      EmberAfReportingDirection direction;
      EmberAfAttributeId attributeId;
      direction =  (EmberAfReportingDirection)emberAfGetInt8u(buffer,
                                                              bufIndex,
                                                              bufLen);
      bufIndex++;
      attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                         bufIndex,
                                                         bufLen);
      bufIndex += 2;
      emberAfReportingPrintln("   direction:%x, attr:%2x",
                              direction,
                              attributeId);
    }
    emberAfReportingFlush();
  }
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_REPORTING
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Read Reporting Configuration Response
 *
 * This function is called by the application framework when a Read Reporting
 * Configuration Response command is received from an external device.  The
 * application should return true if the message was processed or false if it
 * was not.
 *
 * @param clusterId The cluster identifier of this response.
 * @param buffer Buffer containing the list of attribute reporting configuration records.
 * @param bufLen The length in bytes of the list.
 */
bool emberAfReadReportingConfigurationResponseCallback(EmberAfClusterId clusterId,
                                                       uint8_t *buffer,
                                                       uint16_t bufLen)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_REPORTING)
  uint16_t bufIndex = 0;
  emberAfReportingPrint("%p_RESP: ", "READ_RPT_CFG");
  emberAfReportingDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfReportingPrintln("");
  emberAfReportingFlush();

  // Each record in the response has a one-byte status, a one-byte direction,
  // and a two-byte attribute id.  If the status is SUCCESS, the record will
  // contain additional fields.
  while (bufIndex + 4 <= bufLen) {
    EmberAfAttributeId attributeId;
    EmberAfStatus status;
    EmberAfReportingDirection direction;
    status = (EmberAfStatus)emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;
    direction = (EmberAfReportingDirection)emberAfGetInt8u(buffer,
                                                           bufIndex,
                                                           bufLen);
    bufIndex++;
    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                       bufIndex,
                                                       bufLen);
    bufIndex += 2;
    emberAfReportingPrintln(" - status:%x, direction:%x, attr:%2x",
                            status,
                            direction,
                            attributeId);
    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      // If the direction indicates the attribute is reported, the record will
      // contain a one-byte type and two two-byte intervals.  If the type is
      // analog, the record will contain a reportable change of the same data
      // type.  If the direction indicates reports of the attribute are
      // received, the record will contain a two-byte timeout.
      switch (direction) {
        case EMBER_ZCL_REPORTING_DIRECTION_REPORTED:
        {
          uint16_t minInterval, maxInterval;
          uint8_t dataType;
          dataType = emberAfGetInt8u(buffer, bufIndex, bufLen);
          bufIndex++;
          minInterval = emberAfGetInt16u(buffer, bufIndex, bufLen);
          bufIndex += 2;
          maxInterval = emberAfGetInt16u(buffer, bufIndex, bufLen);
          bufIndex += 2;
          emberAfReportingPrintln("   type:%x, min:%2x, max:%2x",
                                  dataType,
                                  minInterval,
                                  maxInterval);
          if (emberAfGetAttributeAnalogOrDiscreteType(dataType)
              == EMBER_AF_DATA_TYPE_ANALOG) {
            uint8_t dataSize = emberAfGetDataSize(dataType);
            emberAfReportingPrint("   change:");
            emberAfReportingPrintBuffer(buffer + bufIndex, dataSize, false);
            emberAfReportingPrintln("");
            bufIndex += dataSize;
          }
          break;
        }
        case EMBER_ZCL_REPORTING_DIRECTION_RECEIVED:
        {
          uint16_t timeout = emberAfGetInt16u(buffer, bufIndex, bufLen);
          bufIndex += 2;
          emberAfReportingPrintln("   timeout:%2x", timeout);
          break;
        }
        default:
          emberAfReportingPrintln("ERR: unknown direction %x", direction);
          emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
          return true;
      }
    }
    emberAfReportingFlush();
  }
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_REPORTING
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

/** @brief Report Attributes
 *
 * This function is called by the application framework when a Report
 * Attributes command is received from an external device.  The application
 * should return true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this command.
 * @param buffer Buffer containing the list of attribute report records.
 * @param bufLen The length in bytes of the list.
 */
bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_REPORTING)
  uint16_t bufIndex = 0;
  emberAfReportingPrint("RPT_ATTR: ");
  emberAfReportingDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfReportingPrintln("");

  // Each record in the response has a two-byte attribute id, a one-byte
  // type, and variable-length data.
  while (bufIndex + 3 < bufLen) {
    EmberAfAttributeId attributeId;
    uint8_t dataType;
    uint16_t dataSize;
    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                       bufIndex,
                                                       bufLen);
    bufIndex += 2;
    dataType = emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;

    dataSize = emberAfAttributeValueSize(dataType, buffer + bufIndex);

    if (dataSize <= bufLen - bufIndex) {
      emberAfReportingPrintln(" - attr:%2x", attributeId);
      emberAfReportingPrint("   type:%x, val:", dataType);
      if (emberAfIsStringAttributeType(dataType)) {
        emberAfReportingPrintString(buffer + bufIndex);
      } else if (emberAfIsLongStringAttributeType(dataType)) {
        emberAfReportingDebugExec(emberAfPrintLongString(EMBER_AF_PRINT_REPORTING, buffer + bufIndex));
      } else {
        emberAfReportingPrintBuffer(buffer + bufIndex, dataSize, false);
      }
      emberAfReportingPrintln("");
      // display specific message for on/off cluster report
      if ((clusterId == ZCL_ON_OFF_CLUSTER_ID)
          && (attributeId == ZCL_ON_OFF_ATTRIBUTE_ID)) {
        uint8_t lightValue = emberAfGetInt8u(buffer, bufIndex, bufLen);
        emberAfReportingPrintln("************************");
        emberAfReportingPrintln(" Light is now %p", (lightValue > 0 ? "ON" : "OFF"));
        emberAfReportingPrintln("************************");
      }
      emberAfReportingFlush();
      bufIndex += dataSize;
    } else {
      // dataSize exceeds buffer length, terminate loop
      emberAfReportingPrintln("ERR: attr:%2x size %d exceeds buffer size", attributeId, dataSize);
      break;
    }
  }
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_REPORTING
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
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
}
