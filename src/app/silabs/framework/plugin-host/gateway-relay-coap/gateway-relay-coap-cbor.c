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
 * @brief CBOR definitions for the Gateway Relay COAP plugin.
 *******************************************************************************
   ******************************************************************************/

// ************** simple CBOR encoder/decoder ***************
#include PLATFORM_HEADER //compiler/micro specifics, types
#include "stack/include/error.h"
#include "gateway-relay-coap-cbor.h"

// Simple CBOR decoder for dotdot.  It assumes the arguments are in a CBOR map
static uint8_t *cborReadBuffer;
static uint16_t cborReadFinger;
static uint16_t cborReadBufferLength = 0;
static uint32_t cborNumArguments;
static uint8_t  cborArgFinger;

#define CBOR_TYPE_UNSIGNED_INT 0
#define CBOR_TYPE_NEGATIVE_INT 1
#define CBOR_TYPE_BYTE_STRING  2
#define CBOR_TYPE_TEXT_STRING  3
#define CBOR_TYPE_ARRAY        4
#define CBOR_TYPE_MAP          5

#define CBOR_EXTEND_1          24
#define CBOR_EXTEND_2          25
#define CBOR_EXTEND_4          26
#define CBOR_EXTEND_8          27

#define CBOR_FALSE  0xF4
#define CBOR_TRUE   0xF5
#define CBOR_NULL   0xF6
#define CBOR_FLOAT_HALF 0xF9
#define CBOR_FLOAT_SINGLE 0xFA
#define CBOR_FLOAT_DOUBLE 0xFB
#define CBOR_BREAK  0xff

#define CBOR_TYPE_SHIFT  5
#define CBOR_LENGTH_MASK 0x1f
#define CBOR_LENGTH_INFINITE 0x1f
#define CBOR_MAX_IMPLEMENTED_LENGTH 4

#define MAX_STRING_LENGTH 256

static uint8_t grabTypeFromByte(uint8_t byte)
{
  return (byte >> CBOR_TYPE_SHIFT);
}

static uint8_t grabCborType(void)
{
  return (grabTypeFromByte(cborReadBuffer[cborReadFinger]));
}

static uint8_t grabLengthFromByte(uint8_t byte)
{
  return (byte & CBOR_LENGTH_MASK);
}

static EmberStatus cborGrabNextByte(uint8_t *byte)
{
  if (cborReadFinger < cborReadBufferLength) {
    *byte = cborReadBuffer[cborReadFinger];
    cborReadFinger++;
  } else {
    printf("CBOR: error:  reading past end of buffer.\r\n");
    return EMBER_ERR_FATAL;
  }

  return EMBER_SUCCESS;
}

static uint32_t makeValue(uint8_t length)
{
  uint32_t value = 0;
  int i;

  for (i = 0; i < length; i++) {
    value <<= 8;
    value += cborReadBuffer[cborReadFinger];
    cborReadFinger++;
  }

  return value;
}

// Note:  even though CBOR supports up to uint64_t bytes, we only support 32
// bits here.  We will throw an error otherwise.
static uint32_t grabCborValue(uint8_t *size)
{
  uint8_t shortValue;
  uint32_t fullValue = 0;

  // first byte is the initial length.
  shortValue = grabLengthFromByte(cborReadBuffer[cborReadFinger]);
  cborReadFinger += 1;

  switch (shortValue) {
    case CBOR_EXTEND_1:
      fullValue = makeValue(1);

      if (size != NULL) {
        *size = 1;
      }
      break;
    case CBOR_EXTEND_2:
      fullValue = makeValue(2);
      if (size != NULL) {
        *size = 2;
      }
      break;
    case CBOR_EXTEND_4:
      fullValue = makeValue(4);
      if (size != NULL) {
        *size = 4;
      }
      break;
    case CBOR_EXTEND_8:
      fprintf(stderr, "CBOR Error:  don't support 64 bit value\r\n");
      cborReadFinger += 8;
      fullValue = 0;
      if (size != NULL) {
        *size = 8;
      }
      break;
    default:
      fullValue = (uint32_t) shortValue;
      if (size != NULL) {
        *size = 1;
      }
      break;
  }
  return fullValue;
}

EmberStatus emAfPluginGatewayRelayCoapCborReadInit(uint8_t *buffer,
                                                   uint16_t bufferLength)
{
  uint8_t cborType;

  if (bufferLength == 0) {
    cborReadBufferLength = 0;
    return EMBER_SUCCESS;
  }

  cborReadBufferLength = bufferLength;
  cborReadBuffer = buffer;
  cborReadFinger = 0;

  cborType = grabTypeFromByte(cborReadBuffer[cborReadFinger]);

  if (cborType != CBOR_TYPE_MAP) {
    printf("ERROR:  wrong CBOR type %d\r\n", cborType);
    return EMBER_BAD_ARGUMENT;
  }

  cborNumArguments = grabCborValue(NULL);
  cborArgFinger = 0;

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborGetUnsignedInteger(uint32_t *value,
                                                             uint8_t *size)
{
  if (cborArgFinger >= cborNumArguments
      || cborReadFinger >= cborReadBufferLength) {
    printf("CBOR: buffer overrun1\r\n");
    return EMBER_BAD_ARGUMENT;
  }

  if (grabCborType() != CBOR_TYPE_UNSIGNED_INT) {
    printf("CBOR:  expected unsigned int.  Got %d\r\n", grabCborType());
    return EMBER_BAD_ARGUMENT;
  }

  *value = grabCborValue(size);

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborGetSignedInteger(int32_t *value,
                                                           uint8_t *size)
{
  uint8_t cborType = grabCborType();

  if (cborArgFinger >= cborNumArguments
      || cborReadFinger >= cborReadBufferLength) {
    return EMBER_BAD_ARGUMENT;
  }

  if (cborType != CBOR_TYPE_UNSIGNED_INT
      && cborType != CBOR_TYPE_NEGATIVE_INT) {
    return EMBER_BAD_ARGUMENT;
  }

  *value = grabCborValue(size);

  if (cborType == CBOR_TYPE_NEGATIVE_INT) {
    *value = *value * -1;
    *value -= 1;
  }

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborGetByteStringDefiniteLength(
  uint8_t *buffer,
  uint16_t bufferLength)
{
  uint16_t i;
  uint32_t stringLength;
  uint8_t size;

  stringLength = grabCborValue(&size);

  if (size > CBOR_MAX_IMPLEMENTED_LENGTH || stringLength > bufferLength) {
    printf("CBOR:  error:  string length too long\r\n");
    return EMBER_BAD_ARGUMENT;
  }

  for (i = 0; i < stringLength; i++) {
    if (cborGrabNextByte(buffer)) {
      return EMBER_ERR_FATAL;
    }
    buffer++;
  }
  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborGetByteStringIndefiniteLength(
  uint8_t *buffer,
  uint16_t bufferLength)
{
  // Note:  this is a placeholder function for now.  CBOR standard allows an
  // indefinite lenght string.  I have chosen not to implement this yet
  // because I don't think we need it yet.
  return EMBER_ERR_FATAL;
}

EmberStatus emAfPluginGatewayRelayCoapCborGetBooleanValue(bool *value)
{
  uint8_t byte;

  if (cborGrabNextByte(&byte)) {
    return EMBER_ERR_FATAL;
  }

  if (byte == CBOR_TRUE) {
    *value = true;
  } else if (byte == CBOR_FALSE) {
    *value = false;
  } else {
    printf("CBOR: error:  expected boolen.  Got %02x\r\n", byte);
    return EMBER_BAD_ARGUMENT;
  }
  return EMBER_SUCCESS;
}

// ************** simple CBOR encoder ***************
// Simple CBOR encoder for Thread.  It assumes the arguments are in a CBOR map
static uint8_t *cborWriteBuffer;
static uint16_t cborWriteBufferLength;
static uint16_t cborWriteFinger;

EmberStatus emAfPluginGatewayRelayCoapCborWriteInit(uint8_t *buffer,
                                                    uint16_t bufferLength)
{
  cborWriteBuffer = buffer;
  cborWriteBufferLength = bufferLength;
  cborWriteFinger = 0;

  return EMBER_SUCCESS;
}

static EmberStatus cborWriteBufferByte(uint8_t byte)
{
  if (cborWriteFinger < cborWriteBufferLength) {
    cborWriteBuffer[cborWriteFinger] = byte;
    cborWriteFinger++;

    return EMBER_SUCCESS;
  } else {
    printf("ERROR:  CBOR buffer out of space\r\n");
    return EMBER_BAD_ARGUMENT;
  }
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteMapTag(void)
{
  // write array tag with indefinite length (31)
  return cborWriteBufferByte((CBOR_TYPE_MAP << CBOR_TYPE_SHIFT)
                             + CBOR_LENGTH_INFINITE);
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteArrayTag(void)
{
  // write array tag with indefinite length (31)
  return cborWriteBufferByte((CBOR_TYPE_ARRAY << CBOR_TYPE_SHIFT)
                             + CBOR_LENGTH_INFINITE);
}

uint16_t emAfPluginGatewayRelayCoapCborCurrentWriteDataLength(void)
{
  return cborWriteFinger;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteStringOfLength(const uint8_t *string,
                                                              uint16_t length,
                                                              bool isText)
{
  uint8_t coapStringType = (isText ? CBOR_TYPE_TEXT_STRING : CBOR_TYPE_BYTE_STRING);
  uint8_t i;
  EmberStatus status;

  // Note:  does not handle strings greater than 256, which shouldn't be a
  // problem
  if (length < CBOR_EXTEND_1) {
    cborWriteBufferByte((coapStringType << CBOR_TYPE_SHIFT) + (uint8_t)length);
  } else if (length < MAX_STRING_LENGTH) {
    cborWriteBufferByte((coapStringType << CBOR_TYPE_SHIFT) + CBOR_EXTEND_1);
    cborWriteBufferByte((uint8_t)length);
  } else {
    return EMBER_BAD_ARGUMENT;
  }

  for (i = 0; i < length; i++) {
    status = cborWriteBufferByte(string[i]);
    if (status > 0) {
      return status;
    }
  }
  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteString(char *string)
{
  return emAfPluginGatewayRelayCoapCborWriteStringOfLength(
    (uint8_t *)string,
    strlen(string),
    true); // text string, not binary string
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteBreak(void)
{
  return cborWriteBufferByte(CBOR_BREAK);
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteTrue(void)
{
  return cborWriteBufferByte(CBOR_TRUE);
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteFalse(void)
{
  return cborWriteBufferByte(CBOR_FALSE);
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteInt8(int8_t signedData8)
{
  int64_t signedData64 = (int64_t) signedData8;

  emAfPluginGatewayRelayCoapCborWriteInt64(signedData64);

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteInt16(int16_t signedData16)
{
  int64_t signedData64 = (int64_t) signedData16;

  emAfPluginGatewayRelayCoapCborWriteInt32(signedData64);

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteInt32(int32_t signedData32)
{
  int64_t signedData64 = (int64_t) signedData32;

  emAfPluginGatewayRelayCoapCborWriteInt64(signedData64);

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteUint8(uint8_t data8)
{
  uint64_t data64 = (uint64_t) data8;

  emAfPluginGatewayRelayCoapCborWriteUint64(data64);

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteUint16(uint16_t data16)
{
  uint64_t data64 = (uint64_t) data16;

  emAfPluginGatewayRelayCoapCborWriteUint64(data64);

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteUint32(uint32_t data32)
{
  uint64_t data64 = (uint64_t) data32;

  emAfPluginGatewayRelayCoapCborWriteUint64(data64);

  return EMBER_SUCCESS;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteInteger(uint64_t data64, uint8_t cborType)
{
  uint8_t byteValue;
  EmberStatus status;

  if (data64 < CBOR_EXTEND_1) {
    // compress into CBOR additionalInfo subfield
    byteValue = (uint8_t) data64;
    status = cborWriteBufferByte((cborType << CBOR_TYPE_SHIFT) + byteValue);
  } else if (data64 < 0x100) {
    // CBOR 1-byte extension
    cborWriteBufferByte((cborType << CBOR_TYPE_SHIFT) + CBOR_EXTEND_1);
    byteValue = (uint8_t) data64;
    status = cborWriteBufferByte(byteValue);
  } else if (data64 < 0x10000) {
    // CBOR 2-byte extension
    cborWriteBufferByte((cborType << CBOR_TYPE_SHIFT) + CBOR_EXTEND_2);
    byteValue = (uint8_t) (data64 >> 8);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 & 0xff);
    status = cborWriteBufferByte(byteValue);
  } else if (data64 < 0x100000000) {
    // CBOR 4-byte extension
    cborWriteBufferByte((cborType << CBOR_TYPE_SHIFT) + CBOR_EXTEND_4);
    byteValue = (uint8_t) (data64 >> 24);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 16);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 8);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 & 0xff);
    status = cborWriteBufferByte(byteValue);
  } else {
    // CBOR 8-byte extension
    cborWriteBufferByte((cborType << CBOR_TYPE_SHIFT) + CBOR_EXTEND_8);
    byteValue = (uint8_t) (data64 >> 56);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 48);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 40);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 32);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 24);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 16);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 >> 8);
    cborWriteBufferByte(byteValue);
    byteValue = (uint8_t) (data64 & 0xff);
    status = cborWriteBufferByte(byteValue);
  }
  return status;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteInt64(int64_t signedData64)
{
  uint64_t data64;
  uint8_t cborType = CBOR_TYPE_UNSIGNED_INT;

  if (signedData64 < 0) {
    signedData64 += 1;
    signedData64 *= -1;
    cborType = CBOR_TYPE_NEGATIVE_INT;
  }

  data64 = (uint64_t) signedData64;

  return emAfPluginGatewayRelayCoapCborWriteInteger(data64, cborType);
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteUint64(uint64_t data64)
{
  return emAfPluginGatewayRelayCoapCborWriteInteger(data64, CBOR_TYPE_UNSIGNED_INT);
}

static EmberStatus emAfPluginGatewayRelayCoapCborWriteFloat(uint64_t data64, uint8_t cborFloatType)
{
  uint8_t byteValue;
  EmberStatus status;

  cborWriteBufferByte(cborFloatType);

  switch (cborFloatType) {
    case CBOR_FLOAT_DOUBLE:
      byteValue = (uint8_t) (data64 >> 56);
      cborWriteBufferByte(byteValue);
      byteValue = (uint8_t) (data64 >> 48);
      cborWriteBufferByte(byteValue);
      byteValue = (uint8_t) (data64 >> 40);
      cborWriteBufferByte(byteValue);
      byteValue = (uint8_t) (data64 >> 32);
      cborWriteBufferByte(byteValue);
    // fall through

    case CBOR_FLOAT_SINGLE:
      byteValue = (uint8_t) (data64 >> 24);
      cborWriteBufferByte(byteValue);
      byteValue = (uint8_t) (data64 >> 16);
      cborWriteBufferByte(byteValue);
    // fall through

    case CBOR_FLOAT_HALF:
      byteValue = (uint8_t) (data64 >> 8);
      cborWriteBufferByte(byteValue);
      byteValue = (uint8_t) (data64 >> 0);
      status = cborWriteBufferByte(byteValue);
      break;

    default:
      status = EMBER_BAD_ARGUMENT;
      break;
  }

  return status;
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteFloatHalf(uint16_t data16)
{
  uint64_t data64 = (uint64_t)data16;
  return emAfPluginGatewayRelayCoapCborWriteFloat(data64, CBOR_FLOAT_HALF);
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteFloatSingle(uint32_t data32)
{
  uint64_t data64 = (uint64_t)data32;
  return emAfPluginGatewayRelayCoapCborWriteFloat(data64, CBOR_FLOAT_SINGLE);
}

EmberStatus emAfPluginGatewayRelayCoapCborWriteFloatDouble(uint64_t data64)
{
  return emAfPluginGatewayRelayCoapCborWriteFloat(data64, CBOR_FLOAT_DOUBLE);
}
