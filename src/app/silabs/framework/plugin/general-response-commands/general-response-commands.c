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
 * @brief Routines for the General Response Commands plugin.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/util.h"

bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                           uint8_t *buffer,
                                           uint16_t bufLen)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEBUG)
  uint16_t bufIndex = 0;
  emberAfDebugPrint("%p_RESP: ", "READ_ATTR");
  emberAfDebugDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfDebugPrintln("");
  emberAfDebugFlush();

  // Each record in the response has a two-byte attribute id and a one-byte
  // status.  If the status is SUCCESS, there will also be a one-byte type and
  // variable-length data.
  while (bufIndex + 3 <= bufLen) {
    EmberAfAttributeId attributeId;
    EmberAfStatus status;
    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                       bufIndex,
                                                       bufLen);
    bufIndex += 2;
    status = (EmberAfStatus)emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;
    emberAfDebugPrintln(" - attr:%2x, status:%x", attributeId, status);
    if (status == EMBER_ZCL_STATUS_SUCCESS) {
      uint8_t dataType;
      uint16_t dataSize;
      dataType = emberAfGetInt8u(buffer, bufIndex, bufLen);
      bufIndex++;

      dataSize = emberAfAttributeValueSize(dataType, buffer + bufIndex);

      emberAfDebugPrint("   type:%x, val:", dataType);
      if (dataSize <= bufLen - bufIndex) {
        if (emberAfIsStringAttributeType(dataType)) {
          emberAfDebugPrintString(buffer + bufIndex);
        } else if (emberAfIsLongStringAttributeType(dataType)) {
          emberAfDebugDebugExec(emberAfPrintLongString(EMBER_AF_PRINT_DEBUG, buffer + bufIndex));
        } else {
          emberAfDebugPrintBuffer(buffer + bufIndex, dataSize, false);
        }
        emberAfDebugPrintln("");
        emberAfDebugFlush();
        bufIndex += dataSize;
      } else {
        // dataSize exceeds buffer length, terminate loop
        emberAfDebugPrintln("ERR: attr:%2x size exceeds buffer size %d", attributeId, dataSize);
        emberAfDebugFlush();
        break; // while
      }
    }
  }
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_DEBUG
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfWriteAttributesResponseCallback(EmberAfClusterId clusterId,
                                            uint8_t *buffer,
                                            uint16_t bufLen)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEBUG)
  uint16_t bufIndex = 0;
  emberAfDebugPrint("%p_RESP: ", "WRITE_ATTR");
  emberAfDebugDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfDebugPrintln("");
  emberAfDebugFlush();

  // Each record in the response has a one-byte status.  If the status is not
  // SUCCESS, the record will also contain a two-byte attribute id.
  while (bufIndex + 1 <= bufLen) {
    EmberAfStatus status = (EmberAfStatus)emberAfGetInt8u(buffer,
                                                          bufIndex,
                                                          bufLen);
    bufIndex++;
    emberAfDebugPrintln(" - status:%x", status);
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
      EmberAfAttributeId attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                                            bufIndex,
                                                                            bufLen);
      bufIndex += 2;
      emberAfDebugPrintln("   attr:%2x", attributeId);
    }
    emberAfDebugFlush();
  }
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_DEBUG
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

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

    emberAfReportingPrintln(" - attr:%2x", attributeId);
    emberAfReportingPrint("   type:%x, val:", dataType);
    if (dataSize <= bufLen - bufIndex) {
      if (emberAfIsStringAttributeType(dataType)) {
        emberAfReportingPrintString(buffer + bufIndex);
      } else if (emberAfIsLongStringAttributeType(dataType)) {
        emberAfReportingDebugExec(emberAfPrintLongString(EMBER_AF_PRINT_REPORTING, buffer + bufIndex));
      } else {
        emberAfReportingPrintBuffer(buffer + bufIndex, dataSize, false);
      }
      emberAfReportingPrintln("");
      emberAfReportingFlush();
      bufIndex += dataSize;
    } else {
      // dataSize exceeds buffer length, terminate loop
      emberAfDebugPrintln("ERR: attr:%2x size %d exceeds buffer size", attributeId, dataSize);
      emberAfReportingFlush();
      break; // while
    }
  }
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_REPORTING
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}

bool emberAfDefaultResponseCallback(EmberAfClusterId clusterId,
                                    uint8_t commandId,
                                    EmberAfStatus status)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEBUG)

  emberAfDebugPrint("%p_RESP: ", "DEFAULT");
  emberAfDebugDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfDebugPrintln(" cmd %x status %x", commandId, status);
  emberAfDebugFlush();
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_DEBUG
  return true;
}

bool emberAfDiscoverAttributesResponseCallback(EmberAfClusterId clusterId,
                                               bool discoveryComplete,
                                               uint8_t *buffer,
                                               uint16_t bufLen,
                                               bool extended)
{
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_DEBUG)
  uint16_t bufIndex = 0;

  emberAfDebugPrint("%p%p_RESP: ", "DISC_ATTR", (extended ? "_EXT" : ""));
  emberAfDebugDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfDebugPrintln(" comp %pDONE", discoveryComplete ? "" : "NOT_");
  emberAfDebugFlush();

  // Each record in the response has a two-byte attribute id and a one-byte
  // type.
  while (bufIndex + 3 <= bufLen) {
    EmberAfAttributeId attributeId;
    uint8_t dataType;
    uint8_t accessControl;
    attributeId = (EmberAfAttributeId)emberAfGetInt16u(buffer,
                                                       bufIndex,
                                                       bufLen);
    bufIndex += 2;
    dataType = emberAfGetInt8u(buffer, bufIndex, bufLen);
    bufIndex++;
    if (extended) {
      accessControl = emberAfGetInt8u(buffer, bufIndex, bufLen);
      bufIndex++;
      emberAfDebugPrintln(" - attr:%2x, type:%x ac:%x", attributeId, dataType, accessControl);
    } else {
      emberAfDebugPrintln(" - attr:%2x, type:%x", attributeId, dataType);
    }
    emberAfDebugFlush();
  }
#endif //EMBER_AF_PRINT_ENABLE && EMBER_AF_PRINT_DEBUG
  emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
  return true;
}
