/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#include EMBER_AF_API_ZCL_CORE

// When true, the map/array "...Indefinite..." API functions will instead
// produce encodings with definite lengths. This is the default mode.
static bool forceDefiniteLengthEncoding = true;

//----------------------------------------------------------------
// Encoding

static uint32_t addCborHeader(CborState *state, uint8_t type, uint32_t length)
{
  uint8_t temp[5];
  uint8_t *finger = temp;

  if (state->finger == NULL || state->end <= state->finger) {
    return false;
  }

  if (length <= CBOR_MAX_LENGTH) {
    *finger++ = type | length;
  } else if (length < 1 << 8) {
    *finger++ = type | CBOR_1_BYTE_LENGTH;
    *finger++ = length;
  } else if (length < 1 << 16) {
    *finger++ = type | CBOR_2_BYTE_LENGTH;
    *finger++ = HIGH_BYTE(length);
    *finger++ = LOW_BYTE(length);
  } else {
    *finger++ = type | CBOR_4_BYTE_LENGTH;
    emberStoreHighLowInt32u(finger, length);
    finger += 4;
  }

  uint8_t bytes = finger - temp;
  if (state->finger + bytes <= state->end) {
    MEMCOPY(state->finger, temp, bytes);
    state->finger += bytes;
    return bytes;
  }

  return 0;
}

static uint32_t appendBytes(CborState *state,
                            uint8_t type,
                            const uint8_t *bytes,
                            uint16_t length)
{
  uint32_t len = addCborHeader(state, type, length);

  if (len && (state->finger + length < state->end)) {
    MEMCOPY(state->finger, bytes, length);
    state->finger += length;
    len += length;
  }
  return len;
}

static void incrementCount(CborState *state)
{
  if (state->nestDepth > 0) {
    if (state->nestStack[state->nestDepth - 1].phead != NULL) {
      // Indefinite - count up from zero until break.
      state->nestStack[state->nestDepth - 1].count++;
    } else {
      // Definite - count down from known starting count until zero.
      state->nestStack[state->nestDepth - 1].count--;
      if (state->nestStack[state->nestDepth - 1].count == 0) {
        // Done with this definite layer, pop stack
        state->nestDepth--;
        incrementCount(state);
      }
    }
  }
}

bool emCborGetForceDefiniteLengthEncoding()
{
  return forceDefiniteLengthEncoding;
}

void emCborSetForceDefiniteLengthEncoding(bool force)
{
  forceDefiniteLengthEncoding = force;
}

bool emCborEncodeKey(CborState *state, uint16_t key)
{
  if (addCborHeader(state, CBOR_UNSIGNED, key) != 0) {
    incrementCount(state);
    return true;
  }
  return false;
}

// Return true if value is invalid for the given ZCLIP type.
// Used for TLVs only. Revisit when optional parameter handling is implemented.
bool emCborZclipOutOfRangeValue(uint8_t type,
                                uint16_t valueLength,
                                const uint8_t *valueLoc)
{
  uint32_t value = 0;
  bool     skip = false;

  switch (type) {
    case EMBER_ZCLIP_TYPE_BOOLEAN:
    case EMBER_ZCLIP_TYPE_MISC:
      value = *((uint8_t *) valueLoc);
      if (value == 0xff) {
        skip = true;
      }
      break;

    case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
      value = emFetchInt32uValue(valueLoc, valueLength);
      switch (valueLength) {
        case 1:
          skip = (value == 0xff ? true : false);
          break;
        case 2:
          skip = (value == 0xffff ? true : false);
          break;
        case 4:
          skip = (value == 0xffffffff ? true : false);
          break;
        default:
          return false;
      }
      break;

    case EMBER_ZCLIP_TYPE_INTEGER:
      value = emFetchInt32uValue(valueLoc, valueLength);
      switch (valueLength) {
        case 1:
          skip = (value == 0x80 ? true : false);
          break;
        case 2:
          skip = (value == 0x8000 ? true : false);
          break;
        case 4:
          skip = (value == 0x80000000 ? true : false);
          break;
        default:
          return false;
      }
      break;

    case EMBER_ZCLIP_TYPE_BINARY: {
      EmberZclStringType_t *ezst = (EmberZclStringType_t *)((void *)valueLoc);

      if (ezst->ptr == NULL || ezst->length == 0) {
        skip = true;
      }
    }
    break;
    case EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY:
    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY:
      // Currently not used in TLVs
      break;

    case EMBER_ZCLIP_TYPE_STRING:
    case EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING:
      if (valueLoc == NULL ||  strlen((const char *) valueLoc) == 0) {
        skip = true;
      }
      break;
    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING: {
      EmberZclStringType_t *ezst = (EmberZclStringType_t *)((void *)valueLoc);
      if (ezst->ptr == NULL || ezst->length == 0) {
        skip = true;
      }
      break;
    }

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING:
      // Currently not used in TLVs
      break;
    default:
      break;
  }

  return skip;
}

bool emCborEncodeValue(CborState *state,
                       uint8_t type,
                       uint16_t valueLength,
                       const uint8_t *valueLoc)
{
  uint32_t appendedLen = 0;

  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  switch (type) {
    case EMBER_ZCLIP_TYPE_BOOLEAN:
      if (state->finger < state->end) {
        *state->finger++ = (*((uint8_t *) valueLoc) != 0
                            ? CBOR_TRUE
                            : CBOR_FALSE);
        appendedLen = -1;
      }
      break;

    case EMBER_ZCLIP_TYPE_MISC:
      if (state->finger < state->end) {
        *state->finger++ = *((uint8_t *) valueLoc);
        appendedLen = -1;
      }
      break;

    case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
      appendedLen = addCborHeader(state,
                                  CBOR_UNSIGNED,
                                  emFetchInt32uValue(valueLoc, valueLength));
      break;

    case EMBER_ZCLIP_TYPE_INTEGER: {
      int32_t n = emFetchInt32sValue(valueLoc, valueLength);
      if (n < 0) {
        appendedLen = addCborHeader(state, CBOR_NEGATIVE, -1 - n);
      } else {
        appendedLen = addCborHeader(state, CBOR_UNSIGNED, n);
      }
      break;
    }

    case EMBER_ZCLIP_TYPE_BINARY: {
      EmberZclStringType_t *ezst = (EmberZclStringType_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_BYTES, ezst->ptr, ezst->length);
      break;
    }

    case EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY:
      appendedLen = appendBytes(state, CBOR_BYTES, valueLoc, valueLength);
      break;

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY: {
      uint8_t *buffer = (uint8_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_BYTES, &buffer[1], emberZclStringLength(buffer));
      break;
    }

    case EMBER_ZCLIP_TYPE_STRING:
    case EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING: {
      appendedLen = appendBytes(state, CBOR_TEXT, valueLoc, strlen((const char *) valueLoc));
      break;
    }

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING: {
      EmberZclStringType_t *ezst = (EmberZclStringType_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_TEXT, ezst->ptr, ezst->length);
      break;
    }

    case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING: {
      uint8_t *buffer = (uint8_t *)((void *)valueLoc);
      appendedLen = appendBytes(state, CBOR_TEXT, &buffer[1], emberZclStringLength(buffer));
      break;
    }

    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING:
    case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING: {
      // TODO: Handle long ZigBee strings.
      appendedLen = 0;
      break;
    }

    default:
      appendedLen = 0;
      break;
  }
  if (appendedLen != 0) {
    incrementCount(state);
    return true;
  }
  return false;
}

void emCborEncodeStart(CborState *state, uint8_t *output, uint16_t outputSize)
{
  MEMSET(state, 0, sizeof(CborState));
  state->start = output;
  state->finger = output;
  state->end = output + outputSize;
}

uint32_t emCborEncodeSize(const CborState *state)
{
  return state->finger - state->start;
}

bool emCborEncodeStruct(CborState *state,
                        const ZclipStructSpec *structSpec,
                        const void *theStruct)
{
  ZclipStructData structData;
  uint16_t i;

  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  if (!emExpandZclipStructData(structSpec, &structData)) {
    return false;
  }
  emCborEncodeIndefiniteMap(state);

  for (i = 0; i < structData.fieldCount; i++) {
    ZclipFieldData fieldData;
    emGetNextZclipFieldData(&structData, &fieldData);
    const uint8_t *valueLoc = (uint8_t *)theStruct + fieldData.valueOffset;

    if (fieldData.isTlv) {
      // TLVs can be optional. An out of range value tells us that the TLV should be skipped
      if (emCborZclipOutOfRangeValue(fieldData.valueType, fieldData.valueSize, valueLoc)) {
        continue;
      }

      int32_t tag = -(fieldData.tlvTag + 1);    // ZCLIP uses -(tlvTag + 1) as the key
      emCborEncodeValue(state,
                        EMBER_ZCLIP_TYPE_INTEGER,
                        sizeof(tag),
                        (const uint8_t *)&tag);
    } else {
      if (fieldData.name == NULL) {
        emCborEncodeKey(state, i);
      } else {
        emCborEncodeValue(state,
                          EMBER_ZCLIP_TYPE_STRING,
                          0,     // value length - unused
                          (const uint8_t *)fieldData.name);
      }
    }

    if (!fieldData.isArray) {
      // Encode a single field value.
      emCborEncodeValue(state,
                        fieldData.valueType,
                        fieldData.valueSize,
                        valueLoc);
    } else {
      // We should encode an array of field values.
      CborArray *cborArray = (CborArray *)valueLoc;
      uint8_t *pArrayData = cborArray->ptr; // Get start of array data to encode.
      uint16_t count = cborArray->numElementsToEncode;  // Get array length to encode.
      emCborEncodeIndefiniteArray(state);
      for (uint8_t i = 0; i < count; ++i) {
        if (!emCborEncodeValue(state,
                               cborArray->fieldData.valueType,
                               cborArray->fieldData.valueSize,
                               pArrayData)) {
          return false;
        }
        pArrayData += cborArray->fieldData.valueSize;
      }
      emCborEncodeBreak(state);
    }
  }

  emCborEncodeBreak(state); // Close outer map.

  return true;
}

uint16_t emCborEncodeOneStruct(uint8_t *output,
                               uint16_t outputSize,
                               const ZclipStructSpec *structSpec,
                               const void *theStruct)
{
  CborState state;
  emCborEncodeStart(&state, output, outputSize);
  emCborEncodeStruct(&state, structSpec, theStruct);
  return emCborEncodeSize(&state);
}

// Maps

bool emCborEncodeMapStart(CborState *state,
                          uint8_t *output,
                          uint16_t outputSize,
                          uint16_t count)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeMap(state, count);
}

bool emCborEncodeIndefiniteMapStart(CborState *state,
                                    uint8_t *output,
                                    uint16_t outputSize)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeIndefiniteMap(state);
}

bool emCborEncodeDefinite(CborState *state, uint8_t valueType, uint16_t count)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || state->nestDepth >= MAX_MAP_ARRAY_NESTING) {
    return false;
  } else if (addCborHeader(state, valueType, count) != 0) {
    state->nestDepth++;
    state->nestStack[state->nestDepth - 1].count
      = (valueType == CBOR_MAP ? count * 2 : count);
    state->nestStack[state->nestDepth - 1].phead = NULL;
    return true;
  } else {
    return false;
  }
}

bool emCborEncodeIndefinite(CborState *state, uint8_t valueType)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || state->nestDepth >= MAX_MAP_ARRAY_NESTING) {
    return false;
  } else if (2 <= state->end - state->finger) {
    state->nestDepth++;
    state->nestStack[state->nestDepth - 1].count = 0;
    state->nestStack[state->nestDepth - 1].phead = state->finger;
    *state->finger++ = valueType | CBOR_INDEFINITE_LENGTH;
    return true;
  } else {
    return false;
  }
}

bool emCborEncodeBreak(CborState *state)
{
  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  if (1 <= state->end - state->finger) {
    if (forceDefiniteLengthEncoding) {
      if (state->nestDepth > 0
          && state->nestStack[state->nestDepth - 1].phead != NULL) {
        // update array/map header with count
        uint8_t *phead = state->nestStack[state->nestDepth - 1].phead;
        uint8_t type = (*phead & CBOR_TYPE_MASK);
        uint32_t count = state->nestStack[state->nestDepth - 1].count;
        if (type == CBOR_MAP) {
          // TODO: What if error produced odd-valued count?
          count >>= 1; // counted each map key and value; half that for entries
        }
        if (count <= CBOR_MAX_LENGTH) {
          // Just patch the additionalInfo bits of the map/array header
          *phead = type | (count & CBOR_LENGTH_MASK);
        } else {
          // Shift map/array content forward to create a length field
          // that is sufficiently large (either 1 or 2 bytes) to hold the count.
          if (count < 1 << 8) {
            MEMMOVE(phead + 2, phead + 1, (state->finger - phead + 1));
            state->finger += 1;
            *phead = (type | CBOR_1_BYTE_LENGTH);
            *(phead + 1) = count;
          } else if (2 <= state->end - state->finger) {
            // Assume count of 256+ is within representation of 16 bits.
            // Any count that requires more than that would have exhausted
            // the buffer while encoding the content.
            MEMMOVE(phead + 3, phead + 1, (state->finger - phead + 1));
            state->finger += 2;
            *phead = (type | CBOR_2_BYTE_LENGTH);
            *(phead + 1) = HIGH_BYTE(count);
            *(phead + 2) = LOW_BYTE(count);
          } else {
            // Insufficient space for 16-bit count
            return false;
          }
        }
      }
    } else {
      *state->finger++ = CBOR_BREAK;
    }
    // pop stack, and increment count for next layer up, if any
    state->nestDepth--;
    incrementCount(state);
    return true;
  } else {
    return false;
  }
}

bool emCborEncodeMapEntry(CborState *state,
                          uint16_t key,
                          uint8_t valueType,
                          uint16_t valueSize,
                          const uint8_t *valueLoc)
{
  return (emCborEncodeKey(state, key)
          && emCborEncodeValue(state, valueType, valueSize, valueLoc));
}

// Arrays

bool emCborEncodeArrayStart(CborState *state,
                            uint8_t *output,
                            uint16_t outputSize,
                            uint16_t count)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeArray(state, count);
}

bool emCborEncodeIndefiniteArrayStart(CborState *state,
                                      uint8_t *output,
                                      uint16_t outputSize)
{
  emCborEncodeStart(state, output, outputSize);
  return emCborEncodeIndefiniteArray(state);
}

//----------------------------------------------------------------
// Decoding

void emCborDecodeStart(CborState *state,
                       const uint8_t *input,
                       uint16_t inputSize)
{
  MEMSET(state, 0, sizeof(CborState));
  state->start = (uint8_t *) input;
  state->finger = (uint8_t *) input;
  state->end = (uint8_t *) input + inputSize;
}

static bool peekOrReadCborHeaderLength(CborState *state,
                                       uint8_t b0,
                                       uint32_t *result,
                                       bool read)
{
  uint8_t length = b0 & CBOR_LENGTH_MASK;
  *result = 0;

  uint8_t *finger = state->finger;
  if (finger == NULL) {
    return false;
  }

  if (!read) {
    finger += 1; // skip over b0
  }

  if (length == CBOR_INDEFINITE_LENGTH) {
    *result = -1;
  } else if (length <= CBOR_MAX_LENGTH) {
    *result = length;
  } else if (length == CBOR_1_BYTE_LENGTH) {
    *result = *finger;
    finger += 1;
  } else if (length == CBOR_2_BYTE_LENGTH) {
    *result = HIGH_LOW_TO_INT(finger[0], finger[1]);
    finger += 2;
  } else {
    *result = emberFetchHighLowInt32u(finger);
    finger += 4;
  }

  if (read) {
    state->finger = finger;
  }

  return (finger <= state->end);
}

static bool readCborHeaderLength(CborState *state,
                                 uint8_t b0,
                                 uint32_t *result)
{
  return peekOrReadCborHeaderLength(state, b0, result, true);
}

static const uint32_t uintMasks[] = {
  0x000000FF,
  0x0000FFFF,
  0,
  0xFFFFFFFF
};

static uint8_t zclipTypeToCborType[] = {
  0,             // BOOLEAN - entry for booleans is not used
  CBOR_UNSIGNED, // INTEGER - can be CBOR_NEGATIVE as well
  CBOR_UNSIGNED, // UNSIGNED_INTEGER
  CBOR_BYTES,    // BINARY
  CBOR_BYTES,    // FIXED_LENGTH_BINARY
  CBOR_TEXT,     // STRING
  CBOR_TEXT,     // MAX_LENGTH_STRING
  CBOR_TEXT,     // UINT8_LENGTH_STRING
  CBOR_TEXT,     // UINT16_LENGTH_STRING
  CBOR_BYTES,    // UINT8_LENGTH_PREFIXED_BINARY
  CBOR_BYTES,    // UINT16_LENGTH_PREFIXED_BINARY
  CBOR_TEXT,     // UINT8_LENGTH_PREFIXED_STRING
  CBOR_TEXT      // UINT16_LENGTH_PREFIXED_STRING
};

// returns whether read is successful.
static EmZclCoreCborValueReadStatus_t realReadCborValue(CborState *state, ZclipFieldData *fieldData, uint8_t *valueLocation)
{
  if (state->finger == NULL
      || state->end <= state->finger) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
  }

  uint8_t b0 = *state->finger++;

  uint32_t length = 0;

  // The maximum type we can handle.
  if (EMBER_ZCLIP_START_MARKER <= fieldData->valueType) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE;
  }
  uint8_t cborType = zclipTypeToCborType[fieldData->valueType];

  if (fieldData->valueType == EMBER_ZCLIP_TYPE_BOOLEAN) {
    if (b0 == CBOR_TRUE || b0 == CBOR_FALSE) {
      *valueLocation = (b0 == CBOR_TRUE);
      return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
    } else {
      return EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE;
    }
  } else if (fieldData->valueType == EMBER_ZCLIP_TYPE_MISC) {
    if (b0 & CBOR_MISC) {
      *valueLocation = b0;
      return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
    } else {
      return EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE;
    }
  } else if (!readCborHeaderLength(state, b0, &length)) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
  } else if (!(((b0 & CBOR_TYPE_MASK) == cborType)
               || (fieldData->valueType == EMBER_ZCLIP_TYPE_INTEGER
                   && (b0 & CBOR_TYPE_MASK) == CBOR_NEGATIVE))) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE;
  } else if ((cborType == CBOR_BYTES
              || cborType == CBOR_TEXT)
             && (state->end - state->finger) < length) {
    return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
  } else {
    switch (fieldData->valueType) {
      case EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER:
        if ((length & uintMasks[fieldData->valueSize - 1]) == length) {
          emStoreInt32uValue(valueLocation, length, fieldData->valueSize);  // (length==the decoded value).
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_INTEGER:
        if ((length & (uintMasks[fieldData->valueSize - 1] >> 1)) == length) {
          emStoreInt32uValue(valueLocation,
                             ((b0 & CBOR_TYPE_MASK) == CBOR_UNSIGNED
                              ? length
                              : -1 - (int32_t) length),
                             fieldData->valueSize);
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }
      case EMBER_ZCLIP_TYPE_BINARY:     // Same max length as strings.
      case EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING:
        if (length <= EMBER_ZCL_STRING_LENGTH_MAX) {
          EmberZclStringType_t *ezst =
            (EmberZclStringType_t *) ((void *) valueLocation);
          ezst->ptr = state->finger;
          ezst->length = length;
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY:
        if (length == fieldData->valueSize) {
          MEMCOPY(valueLocation, state->finger, length);
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else if (length < fieldData->valueSize) {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY:
      case EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING:
        if (length <= EMBER_ZCL_STRING_LENGTH_MAX) {
          uint8_t *buffer = (uint8_t *) ((void *) valueLocation);
          buffer[0] = length;
          MEMCOPY(&buffer[1], state->finger, length);
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_STRING:
      case EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING:
        if (length + 1 <= fieldData->valueSize) {
          MEMCOPY(valueLocation, state->finger, length);
          valueLocation[length] = 0;
          state->finger += length;
          return EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS;
        } else {
          return EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE;
        }

      case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY:
      case EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING:
      case EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING:
        // TODO: Handle long ZigBee strings.
        return EM_ZCL_CORE_CBOR_VALUE_READ_NOT_SUPPORTED;

      default:
        return EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
    }
  }
}

static EmZclCoreCborValueReadStatus_t readCborValue(CborState *state,
                                                    ZclipFieldData *fieldData,
                                                    uint8_t *valueLocation)
{
  state->readStatus = realReadCborValue(state, fieldData, valueLocation);
  return state->readStatus;
}

static bool cborDecodeStruct(CborState *state,
                             const ZclipStructSpec *structSpec,
                             void *theStruct)
{
  if ((state->finger == NULL) || (state->end <= state->finger)) {
    return false;
  }

  ZclipStructData structData;
  if (!emExpandZclipStructData(structSpec, &structData)) {
    return false;
  }

  uint32_t fieldCount = 0;
  uint8_t b0 = *state->finger++;
  if (((b0 & CBOR_TYPE_MASK) != CBOR_MAP)
      || (!readCborHeaderLength(state, b0, &fieldCount))) {
    return false;
  }

  ZclipFieldData fieldData;

  // Track (by field index) the number of mandatory fields we expect in the structure.
  // Only mandatory fields among the first 32 fields are tracked.
  uint32_t mandatoryFieldBitmap = 0;
  emResetZclipFieldData(&structData);
  for (uint16_t i = 0; i < structData.fieldCount; i++) {
    emGetNextZclipFieldData(&structData, &fieldData);
    if (fieldData.isMandatory && i < sizeof(mandatoryFieldBitmap) * 8) {
      mandatoryFieldBitmap |= (1 << i); // Set bit for each mandatory field
    }
  }

  for (uint16_t i = 0; i < fieldCount || fieldCount == ((uint32_t) -1); i++) {
    uint8_t type = emCborDecodePeek(state, NULL);
    if (type == CBOR_BREAK) {
      return (fieldCount == ((uint32_t) -1));
    }

    uint16_t keyIndex = 0;
    uint8_t keyName[8];
    int32_t keyTlv = 0;
    if (type == CBOR_UNSIGNED) {
      emCborDecodeValue(state,
                        EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                        sizeof(keyIndex),
                        (uint8_t *)&keyIndex);
    } else if (type == CBOR_NEGATIVE) {
      emCborDecodeValue(state,
                        EMBER_ZCLIP_TYPE_INTEGER,
                        sizeof(keyTlv),
                        (uint8_t *)&keyTlv);

      keyTlv = -(keyTlv + 1); // ZCLIP uses -tlvTag as the key

      // Enforce range for TLV Tags
      if (keyTlv > TLV_TAG_MASK) {
        return false;
      }
    } else if (type == CBOR_TEXT) {
      emCborDecodeValue(state,
                        EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,
                        sizeof(keyName),
                        keyName);
    } else {
      // Skip the key and value.
      emCborDecodeSkipValue(state);
      emCborDecodeSkipValue(state);
      continue;
    }

    uint16_t j;
    emResetZclipFieldData(&structData);
    for (j = 0; j < structData.fieldCount; j++) {
      emGetNextZclipFieldData(&structData, &fieldData);
      if (fieldData.isTlv) {
        if (type == CBOR_NEGATIVE && keyTlv == fieldData.tlvTag) {
          break;
        }
      } else {
        if ((type == CBOR_UNSIGNED && fieldData.name == NULL && keyIndex == j)
            || (type == CBOR_TEXT
                && fieldData.name != NULL
                && strcmp((const char *)keyName, fieldData.name) == 0)) {
          break;
        }
      }
    }

    if (j == structData.fieldCount) {
      emCborDecodeSkipValue(state);
    } else {
      uint8_t *valueLoc = (uint8_t *)theStruct + fieldData.valueOffset;

      if (!fieldData.isArray) {
        // Decode a single (i.e. non-array) field value.
        if (readCborValue(state, &fieldData, valueLoc)
            != EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS) {
          return false;
        }
      } else {
        // The field consists of an array of cbor values- at this stage we only
        // check that the cbor array header is valid (full cbor array decode is
        // performed by apis by accessing the field's CborArray ptr, for detail
        // see emCborDecodeFieldArrayIntoBuffer().
        if (emCborDecodePeek(state, NULL) != CBOR_ARRAY) {
          return false;
        }

        // For an array field valueLoc points to a CborArray struct- update
        // this struct with current decode info.
        CborArray *cborArray = (CborArray *)valueLoc;
        cborArray->ptr = state->finger;
        cborArray->payloadSize = state->end - state->finger;
        cborArray->fieldData = fieldData;

        emCborDecodeSkipValue(state); // skip the array ready for next field.
      }

      if (j < sizeof(mandatoryFieldBitmap) * 8) {
        mandatoryFieldBitmap &= ~(1 << j); // Always clear bit in decoded mandatory field bitmap.
      }
    }
  }

  // Check that all expected mandatory fields are present.
  if (mandatoryFieldBitmap != 0) { // All bits set for mandatory fields should have been cleared
    return false;
  }

  return true;
}

bool emCborDecodeStruct(CborState *state,
                        const ZclipStructSpec *structSpec,
                        void *theStruct)
{
  return cborDecodeStruct(state, structSpec, theStruct);
}

bool emCborDecodeOneStruct(const uint8_t *input,
                           uint16_t inputSize,
                           const ZclipStructSpec *structSpec,
                           void *theStruct)
{
  CborState state;
  emCborDecodeStart(&state, input, inputSize);
  return cborDecodeStruct(&state, structSpec, theStruct);
}

uint16_t emCborDecodeFieldArrayIntoBuffer(CborArray *cborArray,
                                          uint8_t *buffer,
                                          uint16_t bufferSize,
                                          size_t bufferValueSize)
{
  // Decodes cbor array elements into the specified output buffer.
  // On entry *cborArray must point to a valid CborArray.
  // Return: the number of array elements written to the decode output buffer.

  CborState state;
  emCborDecodeStart(&state, cborArray->ptr, cborArray->payloadSize);

  // Decode the cbor array header and get the cbor array length.
  uint32_t arrayLength = 0;
  uint8_t b0 = *state.finger++;
  if (((b0 & CBOR_TYPE_MASK) != CBOR_ARRAY)
      || (!readCborHeaderLength(&state, b0, &arrayLength))) {
    return 0;
  }
  uint16_t cborArrayCount = arrayLength; // (cborArrayCount=0xFFFF for indeterminate length cbor array).
  uint16_t maxBufferIdx = bufferSize / bufferValueSize;
  uint8_t *valueLoc = buffer;

  // Modify the cCborArray valueSize to reflect the real size of the buffer
  // elements (presently valueSize represents the size of the CborArray
  // struct itself).
  cborArray->fieldData.valueSize = bufferValueSize;

  // Fill the output buffer with the cbor values as decoded from the array.
  uint16_t idx;
  for (idx = 0; (idx < cborArrayCount) && (idx < maxBufferIdx); ++idx) {
    if ((cborArrayCount == 0xFFFF)
        && (emCborDecodePeek(&state, NULL) == CBOR_BREAK)) {
      break;  // found end of indeterminate length cbor array.
    }
    if (readCborValue(&state, &cborArray->fieldData, valueLoc)
        != EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS) {
      return 0;
    }
    valueLoc += bufferValueSize;
  }

  return (idx);
}

// Decoding arrays and maps

// Decrement the number of array or map entries.  If we know the number of
// elements we decrement the count.  If it reaches zero we pop off the
// innermost count.  If there are an indefinite number of elements we
// peek to see if the next thing is a break, in which case we pop off the
// innermost count.

enum {
  SEQUENCE_NOT_DONE,
  SEQUENCE_DONE,
  SEQUENCE_ERROR
};

static int decrementCount(CborState *state)
{
  if (state->finger == NULL
      || state->end < state->finger) {
    return SEQUENCE_ERROR;
  }

  if (state->nestDepth != 0) {
    uint32_t count = state->nestStack[state->nestDepth - 1].count;
    // fprintf(stderr, "[pop depth %d count %d]\n",
    //         state->nestDepth,
    //         count);
    if (count == (uint32_t) -1) {          // ends with a break
      if (*state->finger == CBOR_BREAK) {
        state->nestDepth -= 1;
        state->finger += 1;
        // fprintf(stderr, "[break]\n");
        return SEQUENCE_DONE;
      }
    } else if (count == 0) {
      state->nestDepth -= 1;
      return SEQUENCE_DONE;
    } else {
      state->nestStack[state->nestDepth - 1].count -= 1;
    }
  }
  return SEQUENCE_NOT_DONE;
}

bool emCborDecodeSequence(CborState *state, uint8_t valueType)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || decrementCount(state) != SEQUENCE_NOT_DONE) {
    return false;
  }

  if (state->nestDepth == MAX_MAP_ARRAY_NESTING) {
    return false;
  }

  uint8_t b0 = *state->finger++;

  if ((b0 & CBOR_TYPE_MASK) != valueType) {
    return false;
  }

  uint32_t count = 0;
  bool status = readCborHeaderLength(state, b0, &count);
  if (!status) {
    return false;
  }

  state->nestStack[state->nestDepth].count =
    ((count != (uint32_t) -1
      && valueType == CBOR_MAP)
     ? count * 2        // maps have two values (key+value) for each item
     : count);
  state->nestDepth += 1;
  return true;
}

uint16_t emCborDecodeKey(CborState *state)
{
  uint16_t key;
  if (emCborDecodeValue(state,
                        EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                        2,
                        (uint8_t *) &key)) {
    return key;
  } else {
    return -1;
  }
}

bool emCborDecodeValue(CborState *state,
                       uint8_t valueType,
                       uint16_t valueSize,
                       uint8_t *valueLoc)
{
  if (decrementCount(state)  != SEQUENCE_NOT_DONE) {
    state->readStatus = EM_ZCL_CORE_CBOR_VALUE_READ_ERROR;
    return false;
  } else {
    ZclipFieldData fieldData;
    fieldData.valueType = valueType;
    fieldData.valueSize = valueSize;

    if (readCborValue(state, &fieldData, valueLoc) == EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS) {
      return true;
    } else {
      return false;
    }
  }
}

bool emCborDecodeItem(CborState *state,
                      uint8_t type,
                      uint8_t **contentsLoc,
                      uint32_t *contentsLengthLoc)
{
  uint8_t b0 = *state->finger++;
  uint32_t length = 0;
  if (decrementCount(state) == SEQUENCE_NOT_DONE
      && readCborHeaderLength(state, b0, &length)
      && (b0 & CBOR_TYPE_MASK) == type
      && length != (uint32_t) -1) {  // can't handle indefinite lengths
    *contentsLoc = state->finger;
    *contentsLengthLoc = length;
    state->finger += length;
    return true;
  } else {
    return false;
  }
}

bool emCborDecodeSequenceEnd(CborState *state)
{
  return decrementCount(state) == SEQUENCE_DONE;
}

bool emCborPeekSequenceEnd(CborState *state)
{
  if (state->nestDepth == 0) {
    return false;
  } else {
    uint32_t count = state->nestStack[state->nestDepth - 1].count;
    return (count == (uint32_t) -1           // ends with a break
            ? *state->finger == CBOR_BREAK
            : count == 0);
  }
}

// Returns the type of the next value.

uint8_t emCborDecodePeek(CborState *state, uint32_t *length)
{
  // TODO: There might be a better value to return here / at least we are not
  // crashing.
  if (state->finger == NULL
      || state->end <= state->finger) {
    return CBOR_BREAK;
  }

  uint8_t b0 = *state->finger;

  if (state->nestDepth != 0) {
    uint32_t count = state->nestStack[state->nestDepth - 1].count;
    if ((count == (uint32_t) -1
         && b0 == CBOR_BREAK)
        || count == 0) {
      return CBOR_BREAK;
    }
  }

  if (length != NULL && !peekOrReadCborHeaderLength(state, b0, length, false)) {
    return CBOR_BREAK;
  }

  if ((b0 & CBOR_TYPE_MASK) == CBOR_MISC) {
    return b0;
  } else {
    return b0 & CBOR_TYPE_MASK;
  }
}

bool emCborDecodeTag(CborState *state, uint32_t *value)
{
  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  }

  uint8_t b0 = *state->finger;

  if ((b0 & CBOR_TYPE_MASK) == CBOR_TAG) {
    state->finger += 1;         // read past b0
    readCborHeaderLength(state, b0, value);
    return true;
  } else {
    return false;
  }
}

bool emCborDecodeSkipValue(CborState *state)
{
  if (state->finger == NULL
      || state->end <= state->finger
      || decrementCount(state)  != SEQUENCE_NOT_DONE) {
    return false;
  }

  uint32_t needed = 1;
  uint32_t saved[16];
  uint8_t depth = 0;

  while (true) {
    uint8_t b0 = *state->finger++;
    if (needed != (uint32_t) -1) {
      needed -= 1;
      if (needed == 0 && depth > 0) {
        depth -= 1;
        needed = saved[depth];
      }
    }
    if ((b0 & CBOR_TYPE_MASK) == CBOR_MISC) {
      switch (b0) {
        case CBOR_EXTENDED: state->finger += 1; break;
        case CBOR_FLOAT16:  state->finger += 2; break;
        case CBOR_FLOAT32:  state->finger += 4; break;
        case CBOR_FLOAT64:  state->finger += 8; break;
        case CBOR_BREAK:
          if (needed == (uint32_t) -1) {
            depth -= 1;
            needed = saved[depth];
          }
          break;
      }
    } else {
      uint32_t length = 0;
      bool status = readCborHeaderLength(state, b0, &length);

      if (!status) {
        return false;
      }

      switch (b0 & CBOR_TYPE_MASK) {
        case CBOR_UNSIGNED:
        case CBOR_NEGATIVE:
          // nothing to do
          break;
        case CBOR_BYTES:
        case CBOR_TEXT:
          state->finger += length;
          break;
        case CBOR_MAP:
          if (length != (uint32_t) -1) {
            length <<= 1;
          }
        // fall through
        case CBOR_ARRAY:
          if (depth >= 16) {
            return false;
          }
          saved[depth] = needed;
          depth += 1;
          needed = length;
          break;
        case CBOR_TAG:
          if (needed != (uint32_t) -1) {
            needed += 1;
          }
          break;
      }
    }
    if (needed == 0) {
      return true;
    }
  }
}

bool emCborNonEmpty(CborState *state)
{
  if (state->finger == NULL
      || state->end <= state->finger) {
    return false;
  } else {
    return true;
  }
}

bool emCborEraseUnsignedInt(CborState *state, uint32_t length)
{
  if (state->finger == NULL || state->end <= state->finger) {
    return false;
  }

  decrementCount(state);

  // Reverse what's done to state->finger in addCborHeader()
  if (length <= CBOR_MAX_LENGTH) {
    state->finger--;
  } else if (length < 1 << 8) {
    state->finger -= 2;
  } else if (length < 1 << 16) {
    state->finger -= 3;
  } else {
    state->finger -= 5;
  }

  return true;
}

// Handy utility to simplify keeping track of how space is left.

typedef struct {
  char *finger;
  char *limit;
} PrintBuffer;

static void addChars(PrintBuffer *buffer,
                     const char *format,
                     ...)
{
  va_list argPointer;
  va_start(argPointer, format);
  int length = vsnprintf(buffer->finger,
                         (buffer->finger < buffer->limit
                          ? buffer->limit - buffer->finger
                          : 0),
                         format,
                         argPointer);
  va_end(argPointer);
  assert(0 <= length);
  buffer->finger += length;
}

// For printing nested arrays and maps.

typedef struct {
  uint32_t count;       // number of values printed so far
  uint32_t needed;      // total number of values, or -1 if indefinite
  bool map;             // true if we are printing a map
} Nesting_t;

// In order to reduce the serial traffic this does not print any spaces.

uint16_t emCborToString(const uint8_t *cbor,
                        uint16_t cborLength,
                        uint8_t *buf,
                        uint16_t bufLength)
{
  PrintBuffer buffer;
  buffer.finger = (char *)buf;
  buffer.limit = (char *)(buf + bufLength);

  CborState state;
  emCborDecodeStart(&state, cbor, cborLength);

  Nesting_t stack[16];
  Nesting_t *top = stack;
  Nesting_t *end = top + 16;
  top->needed = 1;
  top->count = 0;
  top->map = false;

  while (state.finger < state.end) {
    if (0 < top->count) {
      addChars(&buffer, "%c", (top->map && (top->count & 1)
                               ? ':'
                               : ','));
    }
    top->count += 1;
    uint8_t b0 = *state.finger++;
    if ((b0 & CBOR_TYPE_MASK) == CBOR_MISC) {
      switch (b0) {
        case CBOR_FALSE: addChars(&buffer, "false"); break;
        case CBOR_TRUE:  addChars(&buffer, "true");  break;
        case CBOR_NULL:  addChars(&buffer, "null");  break;
        case CBOR_UNDEF: addChars(&buffer, "undef"); break;
        // -> Just skip over these for now.
        case CBOR_EXTENDED: state.finger += 1; break;
        case CBOR_FLOAT16:  state.finger += 2; break;
        case CBOR_FLOAT32:  state.finger += 4; break;
        case CBOR_FLOAT64:  state.finger += 8; break;
        // <-
        case CBOR_BREAK: addChars(&buffer, "break"); break;
      }
    } else {
      uint32_t length = 0;
      bool status = readCborHeaderLength(&state, b0, &length);

      if (!status) {
        return 0;
      }

      switch (b0 & CBOR_TYPE_MASK) {
        case CBOR_NEGATIVE:
          if (length != 0) {
            // In CBOR negative numbers are off by 1
            addChars(&buffer, "-%d", length + 1);
            break;
          } // else fall through to print '0'
        case CBOR_UNSIGNED:
          addChars(&buffer, "%u", length);
          break;
        case CBOR_BYTES: {
          uint16_t i;
          // Both the square brackets and the use of hexidecimal is not
          // standard JSON.  JSON uses base64 for bytes which is not
          // helpful for debugging.
          addChars(&buffer, "[");
          for (i = 0; i < length; i++) {
            addChars(&buffer, "%02X", state.finger[i]);
          }
          addChars(&buffer, "]");
          state.finger += length;
          break;
        }
        case CBOR_TEXT: {
          uint16_t i;
          addChars(&buffer, "\"");
          for (i = 0; i < length; i++) {
            addChars(&buffer, "%c", state.finger[i]);
          }
          state.finger += length;
          addChars(&buffer, "\"");
          break;
        }
        case CBOR_MAP:
        case CBOR_ARRAY:
          top += 1;
          if (top == end) {
            return 0;
          }

          top->count = 0;
          if ((b0 & CBOR_TYPE_MASK) == CBOR_MAP) {
            top->needed = (length == (uint32_t) -1
                           ? length
                           : length << 1);
            top->map = true;
            addChars(&buffer, "{");
          } else {
            top->needed = length;
            top->map = false;
            addChars(&buffer, "[");
          }
          break;
        case CBOR_TAG:
          top->count -= 1;      // didn't print anything
          break;
      }
    }

    // Close off maps and arrays.
    while (top != stack
           && (top->needed == top->count
               || (top->needed == (uint32_t) -1
                   && *state.finger == CBOR_BREAK))) {
      if (top->needed == (uint32_t) -1) {
        state.finger += 1;
      }
      if (top->map) {
        addChars(&buffer, "}");
      } else {
        addChars(&buffer, "]");
      }
      top -= 1;
    }

    if (top == stack && top->count == 1) {
      break;
    }
  }
  return (uint8_t *)buffer.finger - buf;
}
