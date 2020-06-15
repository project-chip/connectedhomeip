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
 * @brief Unit tests for Gateway Relay CoAP plugin
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

#include "app/framework/include/af.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-cbor.h"
#include "app/framework/plugin-host/gateway-relay-coap/gateway-relay-coap-handle-read-attr-resp.h"
#include "app/framework/test/test-framework.h"

#define RETURN_STRING_LENGTH 100
static uint8_t returnString[RETURN_STRING_LENGTH] = { 0, };
static uint16_t returnCode = COAP_RESPONSE_OK;

static void setUpErrorCode(uint16_t error)
{
  returnCode = error;
  emAfPluginGatewayRelayCoapCborWriteInit(returnString, RETURN_STRING_LENGTH - 1);
}

static void printReturnInfo()
{
#if 0 // Set to 1 for additional debug output
  uint8_t returnLength = emAfPluginGatewayRelayCoapCborCurrentWriteDataLength();
  printf("Return code: %d\n", returnCode);
  printf("Length: %d\n", returnLength);
  for (uint8_t i = 0; i < returnLength; ++i) {
    printf("%02X ", returnString[i]);
  }
  printf("\n");
#endif
}

// -----------------------------------------------------------------------------
// Each of the following tests checks the conversion of a payload of a ZCL Read
// Attributes Response message (input[] array) into the corresponding
// CBOR-encoded representation for dotdot (expected[] array).
//
// The encoded result is a CBOR map of key/value pairs, each pair representing
// an attribute ID/value; the "value" of each pair is also a CBOR map,
// containing a single key/value pair, with key equal to "v" (signifying
// "value"), and value being the CBOR-encoded value of the attribute. The
// indefinite length form of CBOR map is used, thus each map will open with
// 0xBF (map of unspecified length) and close with 0xFF (break).
//
// Note that, except for text and binary string types, multi-byte data types
// (e.g. 32-bit integer) generally are in little endian order in a ZCL message
// payload, but in big endian order when encoded in CBOR.
// -----------------------------------------------------------------------------

static void grcReadAttributesResponseBoolTest(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_BOOLEAN_ATTRIBUTE_TYPE,
    0x00, // false

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_BOOLEAN_ATTRIBUTE_TYPE,
    0x01, // true
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": false}
    0x00, 0xBF, 0x61, 0x76, 0xF4, 0xFF,
    // 1 : {"v": true}
    0x01, 0xBF, 0x61, 0x76, 0xF5, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseUns08Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT8U_ATTRIBUTE_TYPE,
    0x00,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT8U_ATTRIBUTE_TYPE,
    0x18,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x00, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x18, 0x18, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseUns16Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT16U_ATTRIBUTE_TYPE,
    0x00, 0x00,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT16U_ATTRIBUTE_TYPE,
    0x18, 0x00,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_INT16U_ATTRIBUTE_TYPE,
    0x61, 0x16,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x00, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x18, 0x18, 0xFF,
    // 2 : {"v": 0x1661}
    0x02, 0xBF, 0x61, 0x76, 0x19, 0x16, 0x61, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseUns32Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT32U_ATTRIBUTE_TYPE,
    0x00, 0x00, 0x00, 0x00,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT32U_ATTRIBUTE_TYPE,
    0x18, 0x00, 0x00, 0x00,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_INT32U_ATTRIBUTE_TYPE,
    0x61, 0x16, 0x00, 0x00,

    0x03, 0x00, // attribute 0x0003
    0x00, // success status
    ZCL_INT32U_ATTRIBUTE_TYPE,
    0x23, 0x00, 0x00, 0x32,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x00, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x18, 0x18, 0xFF,
    // 2 : {"v": 0x1661}
    0x02, 0xBF, 0x61, 0x76, 0x19, 0x16, 0x61, 0xFF,
    // 3 : {"v": 0x32000023}
    0x03, 0xBF, 0x61, 0x76, 0x1A, 0x32, 0x00, 0x00, 0x23, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseUns64Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT64U_ATTRIBUTE_TYPE,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT64U_ATTRIBUTE_TYPE,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_INT64U_ATTRIBUTE_TYPE,
    0x61, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x03, 0x00, // attribute 0x0003
    0x00, // success status
    ZCL_INT64U_ATTRIBUTE_TYPE,
    0x23, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00,

    0x04, 0x00, // attribute 0x0004
    0x00, // success status
    ZCL_INT64U_ATTRIBUTE_TYPE,
    0x46, 0x00, 0x00, 0x64, 0x46, 0x00, 0x00, 0x64,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x00, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x18, 0x18, 0xFF,
    // 2 : {"v": 0x1661}
    0x02, 0xBF, 0x61, 0x76, 0x19, 0x16, 0x61, 0xFF,
    // 3 : {"v": 0x32000023}
    0x03, 0xBF, 0x61, 0x76, 0x1A, 0x32, 0x00, 0x00, 0x23, 0xFF,
    // 4 : {"v": 0x6400004664000046}
    0x04, 0xBF, 0x61, 0x76, 0x1B, 0x64, 0x00, 0x00, 0x46,
    0x64, 0x00, 0x00, 0x46, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseNeg08Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT8S_ATTRIBUTE_TYPE,
    0xFF,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT8S_ATTRIBUTE_TYPE,
    0xE7,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x20, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x38, 0x18, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseNeg16Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT16S_ATTRIBUTE_TYPE,
    0xFF, 0xFF,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT16S_ATTRIBUTE_TYPE,
    0xE7, 0xFF,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_INT16S_ATTRIBUTE_TYPE,
    0x9E, 0xE9,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x20, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x38, 0x18, 0xFF,
    // 2 : {"v": 0x1661}
    0x02, 0xBF, 0x61, 0x76, 0x39, 0x16, 0x61, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseNeg32Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT32S_ATTRIBUTE_TYPE,
    0xFF, 0xFF, 0xFF, 0xFF,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT32S_ATTRIBUTE_TYPE,
    0xE7, 0xFF, 0xFF, 0xFF,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_INT32S_ATTRIBUTE_TYPE,
    0x9E, 0xE9, 0xFF, 0xFF,

    0x03, 0x00, // attribute 0x0003
    0x00, // success status
    ZCL_INT32S_ATTRIBUTE_TYPE,
    0xDC, 0xFF, 0xFF, 0xCD,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x20, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x38, 0x18, 0xFF,
    // 2 : {"v": 0x1661}
    0x02, 0xBF, 0x61, 0x76, 0x39, 0x16, 0x61, 0xFF,
    // 3 : {"v": 0x32000023}
    0x03, 0xBF, 0x61, 0x76, 0x3A, 0x32, 0x00, 0x00, 0x23, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseNeg64Test(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT64S_ATTRIBUTE_TYPE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT64S_ATTRIBUTE_TYPE,
    0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_INT64S_ATTRIBUTE_TYPE,
    0x9E, 0xE9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    0x03, 0x00, // attribute 0x0003
    0x00, // success status
    ZCL_INT64S_ATTRIBUTE_TYPE,
    0xDC, 0xFF, 0xFF, 0xCD, 0xFF, 0xFF, 0xFF, 0xFF,

    0x04, 0x00, // attribute 0x0004
    0x00, // success status
    ZCL_INT64S_ATTRIBUTE_TYPE,
    0xB9, 0xFF, 0xFF, 0x9B, 0xB9, 0xFF, 0xFF, 0x9B,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00}
    0x00, 0xBF, 0x61, 0x76, 0x20, 0xFF,
    // 1 : {"v": 0x18}
    0x01, 0xBF, 0x61, 0x76, 0x38, 0x18, 0xFF,
    // 2 : {"v": 0x1661}
    0x02, 0xBF, 0x61, 0x76, 0x39, 0x16, 0x61, 0xFF,
    // 3 : {"v": 0x32000023}
    0x03, 0xBF, 0x61, 0x76, 0x3A, 0x32, 0x00, 0x00, 0x23, 0xFF,
    // 4 : {"v": 0x6400004664000046}
    0x04, 0xBF, 0x61, 0x76, 0x3B, 0x64, 0x00, 0x00, 0x46,
    0x64, 0x00, 0x00, 0x46, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseUnsOddSizesTest(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_INT24U_ATTRIBUTE_TYPE,
    0x24, 0x24, 0x24,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_INT40U_ATTRIBUTE_TYPE,
    0x40, 0x40, 0x40, 0x40, 0x40,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_INT48U_ATTRIBUTE_TYPE,
    0x48, 0x48, 0x48, 0x48, 0x48, 0x48,

    0x03, 0x00, // attribute 0x0003
    0x00, // success status
    ZCL_INT56U_ATTRIBUTE_TYPE,
    0x56, 0x56, 0x56, 0x56, 0x56, 0x56, 0x56,
  };

  uint8_t expected[] = {
    0xBF,
    // 0 : {"v": 0x00242424}
    0x00, 0xBF, 0x61, 0x76, 0x1A, 0x00, 0x24, 0x24, 0x24, 0xFF,
    // 1 : {"v": 0x0000004040404040}
    0x01, 0xBF, 0x61, 0x76, 0x1B, 0x00, 0x00, 0x00, 0x40,
    0x40, 0x40, 0x40, 0x40, 0xFF,
    // 2 : {"v": 0x0000484848484848}
    0x02, 0xBF, 0x61, 0x76, 0x1B, 0x00, 0x00, 0x48, 0x48,
    0x48, 0x48, 0x48, 0x48, 0xFF,
    // 3 : {"v": 0x0056565656565656}
    0x03, 0xBF, 0x61, 0x76, 0x1B, 0x00, 0x56, 0x56, 0x56,
    0x56, 0x56, 0x56, 0x56, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseOctetStringTest(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_OCTET_STRING_ATTRIBUTE_TYPE,
    0x05, 0x0A, 0x01, 0x13, 0x0F, 0x0E,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE,
    0x05, 0x00, 0x0E, 0x0F, 0x13, 0x01, 0x0A,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_SECURITY_KEY_ATTRIBUTE_TYPE,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,

    0x03, 0x00, // attribute 0x0003
    0x00, // success status
    ZCL_OCTET_STRING_ATTRIBUTE_TYPE,
    0x00, // zero length string

    0x04, 0x00, // attribute 0x0004
    0x00, // success status
    ZCL_OCTET_STRING_ATTRIBUTE_TYPE,
    0xFF, // undefined string

    0x05, 0x00, // attribute 0x0005
    0x00, // success status
    ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE,
    0x00, 0x00, // zero length LONG string

    0x06, 0x00, // attribute 0x0006
    0x00, // success status
    ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE,
    0xFF, 0xFF, // undefined LONG string
  };

  uint8_t expected[] = {
    0xBF,
    // 0: {"v": 0x0A01130F0E}
    0x00, 0xBF, 0x61, 0x76, 0x45, 0x0A, 0x01, 0x13, 0x0F, 0x0E, 0xFF,
    // 1: {"v": 0x0E0F13010A}
    0x01, 0xBF, 0x61, 0x76, 0x45, 0x0E, 0x0F, 0x13, 0x01, 0x0A, 0xFF,
    // 2: {"v": 0x000102030405060708090A0B0C0D0E0F}
    0x02, 0xBF, 0x61, 0x76, 0x50, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
    // 3: {"v": <zero-length>}
    0x03, 0xBF, 0x61, 0x76, 0x40, 0xFF,
    // 4: {"v": <zero-length>}
    0x04, 0xBF, 0x61, 0x76, 0x40, 0xFF,
    // 5: {"v": <zero-length>}
    0x05, 0xBF, 0x61, 0x76, 0x40, 0xFF,
    // 6: {"v": <zero-length>}
    0x06, 0xBF, 0x61, 0x76, 0x40, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseTextStringTest(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
    0x03, 0x43, 0x61, 0x54, // "CaT"

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE,
    0x03, 0x00, 0x64, 0x50, 0x67, // "dOg"

    0x02, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
    0x00, // zero length string

    0x03, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_CHAR_STRING_ATTRIBUTE_TYPE,
    0xFF, // undefined string

    0x04, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE,
    0x00, 0x00, // zero length LONG string

    0x05, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE,
    0xFF, 0xFF, // undefined LONG string
  };

  uint8_t expected[] = {
    0xBF,
    // 0: {"v": "CaT"}
    0x00, 0xBF, 0x61, 0x76, 0x63, 0x43, 0x61, 0x54, 0xFF,
    // 1: {"v": "dOg"}
    0x01, 0xBF, 0x61, 0x76, 0x63, 0x64, 0x50, 0x67, 0xFF,
    // 2: {"v": <zero length string>}
    0x02, 0xBF, 0x61, 0x76, 0x60, 0xFF,
    // 3: {"v": <zero length string>}
    0x03, 0xBF, 0x61, 0x76, 0x60, 0xFF,
    // 4: {"v": <zero length string>}
    0x04, 0xBF, 0x61, 0x76, 0x60, 0xFF,
    // 5: {"v": <zero length string>}
    0x05, 0xBF, 0x61, 0x76, 0x60, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseFloatTest(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_FLOAT_SEMI_ATTRIBUTE_TYPE,
    0x00, 0x01,

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_FLOAT_SINGLE_ATTRIBUTE_TYPE,
    0x04, 0x05, 0x06, 0x07,

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_FLOAT_DOUBLE_ATTRIBUTE_TYPE,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  };

  uint8_t expected[] = {
    0xBF,
    // 0: {"v": 0x0100} half/semi precision
    0x00, 0xBF, 0x61, 0x76, 0xF9, 0x01, 0x00, 0xFF,
    // 1: {"v": 0x07060504} single precision
    0x01, 0xBF, 0x61, 0x76, 0xFA, 0x07, 0x06, 0x05, 0x04, 0xFF,
    // 2: {"v": 0x0F0E0D0C0B0A0908} double precision
    0x02, 0xBF, 0x61, 0x76, 0xFB, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseAttrErrorTest(void)
{
  uint8_t input[] = {
    0x00, 0x00, // attribute 0x0000
    0x86, // error status

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_BOOLEAN_ATTRIBUTE_TYPE,
    0x00, // false

    0x02, 0x00, // attribute 0x0002
    0x86, // error status

    0x03, 0x00, // attribute 0x0003
    0x00, // success status
    ZCL_BOOLEAN_ATTRIBUTE_TYPE,
    0x01, // true

    0x04, 0x00, // attribute 0x0004
    0x86, // error status
  };

  uint8_t expected[] = { // implementation ignores/omits errored attrs
    0xBF,
    // 0 : {"v": false}
    0x01, 0xBF, 0x61, 0x76, 0xF4, 0xFF,
    // 1 : {"v": true}
    0x03, 0xBF, 0x61, 0x76, 0xF5, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input,
    sizeof(input),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected, returnString, sizeof(expected)),
                "actual cbor failed to match expected");
  return;
}

static void grcReadAttributesResponseUnknownZclTypeTest(void)
{
  // Case 1: Unknown type is the only attr in the payload.
  // Implementation returns an empty map.
  uint8_t input1[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_UNKNOWN_ATTRIBUTE_TYPE,
    0x03, 0x43, 0x41, 0x54 // "CAT" w/ leading length byte 0x03
  };

  uint8_t expected1[] = {
    // empty map {}
    0xBF, 0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input1,
    sizeof(input1),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected1, returnString, sizeof(expected1)),
                "case 1 actual cbor failed to match expected");

  // Case 2: Unknown type is an intermediate attr in a multi-attr payload.
  // Implementation returns the attr(s) that precede the unknown attr.
  uint8_t input2[] = {
    0x00, 0x00, // attribute 0x0000
    0x00, // success status
    ZCL_BOOLEAN_ATTRIBUTE_TYPE,
    0x00, // false

    0x01, 0x00, // attribute 0x0001
    0x00, // success status
    ZCL_UNKNOWN_ATTRIBUTE_TYPE,
    0x03, 0x43, 0x41, 0x54, // "CAT" w/ leading length byte 0x03 (irrelevant)

    0x02, 0x00, // attribute 0x0002
    0x00, // success status
    ZCL_BOOLEAN_ATTRIBUTE_TYPE,
    0x01, // true
  };

  uint8_t expected2[] = {
    0xBF,
    // 0 : {"v": false}
    0x00, 0xBF, 0x61, 0x76, 0xF4, 0xFF,
    0xFF,
  };

  emAfPluginGatewayRelayCoapCreateCborDataFromReadAttributeResponse(
    input2,
    sizeof(input2),
    returnString,
    RETURN_STRING_LENGTH,
    setUpErrorCode
    );
  printReturnInfo();
  expectComparisonDecimal(returnCode, COAP_RESPONSE_OK, "expected rcode", "actual rcode");
  expectMessage(0 == memcmp(expected2, returnString, sizeof(expected2)),
                "case 2 actual cbor failed to match expected");
  return;
}

int main(int argc, char* argv[])
{
  const TestCase tests[] = {
    { "grc-read-attributes-response-bool-test", grcReadAttributesResponseBoolTest },
    { "grc-read-attributes-response-uns08-test", grcReadAttributesResponseUns08Test },
    { "grc-read-attributes-response-uns16-test", grcReadAttributesResponseUns16Test },
    { "grc-read-attributes-response-uns32-test", grcReadAttributesResponseUns32Test },
    { "grc-read-attributes-response-uns64-test", grcReadAttributesResponseUns64Test },
    { "grc-read-attributes-response-neg08-test", grcReadAttributesResponseNeg08Test },
    { "grc-read-attributes-response-neg16-test", grcReadAttributesResponseNeg16Test },
    { "grc-read-attributes-response-neg32-test", grcReadAttributesResponseNeg32Test },
    { "grc-read-attributes-response-neg64-test", grcReadAttributesResponseNeg64Test },
    { "grc-read-attributes-response-odd-sizes-test", grcReadAttributesResponseUnsOddSizesTest },
    { "grc-read-attributes-response-octet-string-test", grcReadAttributesResponseOctetStringTest },
    { "grc-read-attributes-response-text-string-test", grcReadAttributesResponseTextStringTest },
    { "grc-read-attributes-response-float-test", grcReadAttributesResponseFloatTest },
    { "grc-read-attributes-response-attr-error-test", grcReadAttributesResponseAttrErrorTest },
    { "grc-read-attributes-response-unknown-zcl-type-test", grcReadAttributesResponseUnknownZclTypeTest },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc, argv, "Gateway Relay CoAP", tests);
}
