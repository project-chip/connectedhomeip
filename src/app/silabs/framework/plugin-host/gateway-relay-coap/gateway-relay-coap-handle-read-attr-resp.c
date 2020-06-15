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
 * @brief Read Attributes Response handling for Gateway Relay CoAP plugin
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include PLATFORM_HEADER //compiler/micro specifics, types

#include "app/framework/include/af.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-cbor.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-handle-read-attr-resp.h"

#define ATTRIBUTE_BUFFER_ATTRIBUTE_ID_LOW_BYTE 0
#define ATTRIBUTE_BUFFER_ATTRIBUTE_ID_HIGH_BYTE 1
#define ATTRIBUTE_BUFFER_SUCCESS_CODE 2
#define ATTRIBUTE_BUFFER_DATA_TYPE 3
#define ATTRIBUTE_BUFFER_DATA_START 4

// ----------------------------------------------------------------------------
// Forward declarations.
static uint64_t assembleValue64(const uint8_t *buffer, uint8_t len, bool isSigned);

// ----------------------------------------------------------------------------
// Translate incoming attributes read response to outgoing dotdot equivalent
// message.

static uint64_t assembleValue64(const uint8_t *buffer, uint8_t len, bool isSigned)
{
  // Construct a uint64_t integer result from a sequence of 1-8 little endian
  // ordered bytes. Although the return type is unsigned, it may represent
  // a signed integer value. The bool argument indicates if a signed integer
  // result is intended, in which case the proper sign extension will be
  // maintained.
  uint64_t result = (isSigned ? 0xFFFFFFFFFFFFFFFFL : 0L);
  for (uint8_t i = 0; i < len; ++i) {
    result = (result << 8) + buffer[len - i - 1];
  }
  return result;
}

void emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
  const uint8_t *buffer,
  uint8_t length,
  uint8_t *returnBuffer,
  uint8_t returnLength,
  void (*setUpErrorCodeFunction)(uint16_t coapStatusCode)
  )
{
  uint8_t counter = 0;
  uint16_t attributeId;
  uint8_t successCode, attributeType;
  uint16_t data16u;
  uint32_t data32u;
  uint64_t data64u;
  uint16_t blen;
  uint8_t boffset;
  bool isSigned;
  bool isLongString;
  bool isShortString;
  bool isTextString;

  emAfPluginGatewayRelayCoapCborWriteInit(returnBuffer, returnLength);

  if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
    setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
  }

  while (counter < length) {
    attributeId = HIGH_LOW_TO_INT(buffer[ATTRIBUTE_BUFFER_ATTRIBUTE_ID_HIGH_BYTE],
                                  buffer[ATTRIBUTE_BUFFER_ATTRIBUTE_ID_LOW_BYTE]);
    printf("Attribute Id %04x\r\n", attributeId);
    successCode = buffer[ATTRIBUTE_BUFFER_SUCCESS_CODE];

    if (successCode != EMBER_ZCL_STATUS_SUCCESS) {
      // Ignore / omit from CBOR encoding a non-SUCCESS attribute. The
      // attribute record will not include type/value fields, so increment
      // buffer/counter only by length of attr ID plus status code (2 + 1)
      // to align with the start of the next attribute record.
      buffer += 3;
      counter += 3;
    } else {
      attributeType = buffer[ATTRIBUTE_BUFFER_DATA_TYPE];
      buffer += ATTRIBUTE_BUFFER_DATA_START;
      counter += ATTRIBUTE_BUFFER_DATA_START;

      switch (attributeType) {
        case ZCL_DATA8_ATTRIBUTE_TYPE:
        case ZCL_DATA16_ATTRIBUTE_TYPE:
        case ZCL_DATA24_ATTRIBUTE_TYPE:
        case ZCL_DATA32_ATTRIBUTE_TYPE:
        case ZCL_DATA40_ATTRIBUTE_TYPE:
        case ZCL_DATA48_ATTRIBUTE_TYPE:
        case ZCL_DATA56_ATTRIBUTE_TYPE:
        case ZCL_DATA64_ATTRIBUTE_TYPE:
        case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
        case ZCL_BITMAP8_ATTRIBUTE_TYPE:
        case ZCL_BITMAP16_ATTRIBUTE_TYPE:
        case ZCL_BITMAP24_ATTRIBUTE_TYPE:
        case ZCL_BITMAP32_ATTRIBUTE_TYPE:
        case ZCL_BITMAP40_ATTRIBUTE_TYPE:
        case ZCL_BITMAP48_ATTRIBUTE_TYPE:
        case ZCL_BITMAP56_ATTRIBUTE_TYPE:
        case ZCL_BITMAP64_ATTRIBUTE_TYPE:
        case ZCL_INT8U_ATTRIBUTE_TYPE:
        case ZCL_INT16U_ATTRIBUTE_TYPE:
        case ZCL_INT32U_ATTRIBUTE_TYPE:
        case ZCL_INT24U_ATTRIBUTE_TYPE:
        case ZCL_INT40U_ATTRIBUTE_TYPE:
        case ZCL_INT48U_ATTRIBUTE_TYPE:
        case ZCL_INT56U_ATTRIBUTE_TYPE:
        case ZCL_INT64U_ATTRIBUTE_TYPE:
        case ZCL_INT8S_ATTRIBUTE_TYPE:
        case ZCL_INT16S_ATTRIBUTE_TYPE:
        case ZCL_INT24S_ATTRIBUTE_TYPE:
        case ZCL_INT32S_ATTRIBUTE_TYPE:
        case ZCL_INT40S_ATTRIBUTE_TYPE:
        case ZCL_INT48S_ATTRIBUTE_TYPE:
        case ZCL_INT56S_ATTRIBUTE_TYPE:
        case ZCL_INT64S_ATTRIBUTE_TYPE:
        case ZCL_ENUM8_ATTRIBUTE_TYPE:
        case ZCL_ENUM16_ATTRIBUTE_TYPE:
          // Signed ZCL types are a contiguous range, so easy to check...
          isSigned = (ZCL_INT8S_ATTRIBUTE_TYPE <= attributeType
                      && attributeType <= ZCL_INT64S_ATTRIBUTE_TYPE);
          // These ZCL type codes are structured such that the value of the
          // type's low 3 bits is one less than its size in bytes. E.g.,
          // DATA24 code is 0x0A -> low 3 bits 0x02 -> size 3 bytes == 24 bits.
          blen = (uint16_t)((attributeType & 0x07) + 1);
          data64u = assembleValue64(buffer, blen, isSigned);

          if (emAfPluginGatewayRelayCoapCborWriteUint16(attributeId)) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteString("v")) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (attributeType == ZCL_BOOLEAN_ATTRIBUTE_TYPE) {
            if (data64u == 0x00
                ? emAfPluginGatewayRelayCoapCborWriteFalse()
                : emAfPluginGatewayRelayCoapCborWriteTrue()) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          } else if (isSigned) {
            if (emAfPluginGatewayRelayCoapCborWriteInt64((int64_t)data64u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          } else { // unsigned equivalents
            if (emAfPluginGatewayRelayCoapCborWriteUint64(data64u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          }
          if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }

          buffer += blen;
          counter += blen;

          break;
        case ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE:
        case ZCL_CLUSTER_ID_ATTRIBUTE_TYPE:
        case ZCL_ATTRIBUTE_ID_ATTRIBUTE_TYPE:
          data16u = HIGH_LOW_TO_INT(buffer[1], buffer[0]);

          if (emAfPluginGatewayRelayCoapCborWriteUint16(attributeId)) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteString("v")) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (attributeType == ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE) {
            if (emAfPluginGatewayRelayCoapCborWriteFloatHalf(data16u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          } else { // unsigned 16-bit equivalents
            if (emAfPluginGatewayRelayCoapCborWriteUint16(data16u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          }
          if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }

          buffer += 2;
          counter += 2;

          break;
        case ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE:
        case ZCL_TIME_OF_DAY_ATTRIBUTE_TYPE:
        case ZCL_DATE_ATTRIBUTE_TYPE:
        case ZCL_UTC_TIME_ATTRIBUTE_TYPE:
        case ZCL_BACNET_OID_ATTRIBUTE_TYPE:
          data32u = (uint32_t)assembleValue64(buffer, 4, false);

          if (emAfPluginGatewayRelayCoapCborWriteUint16(attributeId)) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteString("v")) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (attributeType == ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE) {
            if (emAfPluginGatewayRelayCoapCborWriteFloatSingle(data32u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          } else { // unsigned 32-bit equivalents
            if (emAfPluginGatewayRelayCoapCborWriteUint32(data32u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          }
          if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }

          buffer += 4;
          counter += 4;

          break;
        case ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE:
        case ZCL_IEEE_ADDRESS_ATTRIBUTE_TYPE:
          data64u = assembleValue64(buffer, 8, false);

          if (emAfPluginGatewayRelayCoapCborWriteUint16(attributeId)) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteString("v")) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (attributeType == ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE) {
            if (emAfPluginGatewayRelayCoapCborWriteFloatDouble(data64u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          } else { // unsigned 64-bit equivalents
            if (emAfPluginGatewayRelayCoapCborWriteUint64(data64u)) {
              setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
            }
          }
          if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }

          buffer += 8;
          counter += 8;

          break;
        case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
        case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
        case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        case ZCL_SECURITY_KEY_ATTRIBUTE_TYPE:
          isLongString = (attributeType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE
                          || attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
          isShortString = (attributeType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE
                           || attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
          isTextString = (attributeType == ZCL_CHAR_STRING_ATTRIBUTE_TYPE
                          || attributeType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE);
          if (isLongString) {
            // 2-byte length prefix
            blen = HIGH_LOW_TO_INT(buffer[1], buffer[0]);
            if (blen == 0xFFFF) {
              // Treat "undefined" value for LONG string length to be zero.
              blen = 0;
            }
            boffset = 2;
          } else if (isShortString) {
            // 1-byte length prefix
            blen = *buffer;
            if (blen == 0x00FF) {
              // Treat "undefined" value for (short) string length to be zero.
              blen = 0;
            }
            boffset = 1;
          } else {
            // 16-byte security key
            blen = 16;
            boffset = 0;
          }

          if (emAfPluginGatewayRelayCoapCborWriteUint16(attributeId)) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteMapTag()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteString("v")) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteStringOfLength(
                (buffer + boffset),
                blen,
                isTextString)) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }

          // ZCL string value size: string length plus prefixed length byte.
          buffer += (blen + boffset);
          counter += (blen + boffset);

          break;
        default:
          printf("error:  unknown attribute type\r\n");
          if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
            setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
          }
          return;
          break;
      }
    }
  }
  if (emAfPluginGatewayRelayCoapCborWriteBreak()) {
    setUpErrorCodeFunction(COAP_RESPONSE_INTERNAL_ERROR);
  }
}
