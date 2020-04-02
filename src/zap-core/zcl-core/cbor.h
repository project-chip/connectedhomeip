/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#ifndef CBOR_H
#define CBOR_H

// Dispatch bytes have a type field in the three high-order bits and
// an unsigned integer length in the low-order five bits.

#define CBOR_TYPE_MASK   0xE0
#define CBOR_LENGTH_MASK 0x1F

// Major types (there appear to be no minor types, so really these are
// just the types).

#define CBOR_UNSIGNED (0 << 5)
#define CBOR_NEGATIVE (1 << 5)
#define CBOR_BYTES    (2 << 5)
#define CBOR_TEXT     (3 << 5)
#define CBOR_ARRAY    (4 << 5)
#define CBOR_MAP      (5 << 5)
#define CBOR_TAG      (6 << 5)
#define CBOR_MISC     (7 << 5)  // floating points, atomic values, break

// Lengths 0 to 23 are encoded directly, the higher values have special
// meaning.
#define CBOR_MAX_LENGTH        23
#define CBOR_1_BYTE_LENGTH     24
#define CBOR_2_BYTE_LENGTH     25
#define CBOR_4_BYTE_LENGTH     26
#define CBOR_8_BYTE_LENGTH     27
#define CBOR_INDEFINITE_LENGTH 31

// Misc values
#define CBOR_FALSE    (CBOR_MISC | 20)
#define CBOR_TRUE     (CBOR_MISC | 21)
#define CBOR_NULL     (CBOR_MISC | 22)
#define CBOR_UNDEF    (CBOR_MISC | 23)
#define CBOR_EXTENDED (CBOR_MISC | 24)
#define CBOR_FLOAT16  (CBOR_MISC | 25)
#define CBOR_FLOAT32  (CBOR_MISC | 26)
#define CBOR_FLOAT64  (CBOR_MISC | 27)
#define CBOR_BREAK    (CBOR_MISC | 31)

// Tags
//  -- there are a lot - add as needed
#define CBOR_COSE_SIGN_TAG 98

//----------------
// Error status for CBOR parsing error and etc.

typedef enum {
  EM_ZCL_CORE_CBOR_VALUE_READ_SUCCESS               = 0x00,
  EM_ZCL_CORE_CBOR_VALUE_READ_ERROR                 = 0x01,
  EM_ZCL_CORE_CBOR_VALUE_READ_NOT_SUPPORTED         = 0x02,
  EM_ZCL_CORE_CBOR_VALUE_READ_INVALID_BOOLEAN_VALUE = 0x03,
  EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_LARGE       = 0x04,
  EM_ZCL_CORE_CBOR_VALUE_READ_VALUE_TOO_SMALL       = 0x05,
  EM_ZCL_CORE_CBOR_VALUE_READ_WRONG_TYPE            = 0x06,
} EmZclCoreCborValueReadStatus_t;

//----------------------------------------------------------------
// Struct that is used when writing or parsing CBOR data.

// How deeply we can nest arrays and maps.
#define MAX_MAP_ARRAY_NESTING 5

typedef struct {
  const uint8_t *start; // start of input or output buffer
  uint8_t *finger;      // pointer to next byte to be read or written
  const uint8_t *end;   // the end of the input or output buffer

  EmZclCoreCborValueReadStatus_t readStatus;   // status of last decode read operation

  // Not yet used - will be needed if we have nested structs
  // uint8_t *heapFinger;  // ditto, but for allocation
  // uint8_t *heapEnd;

  // Number of elements in maps and arrays, organized as a stack to
  // allow nesting.
  struct {
    uint32_t count;
    uint8_t *phead;
  } nestStack[MAX_MAP_ARRAY_NESTING];
  uint8_t nestDepth;

  // Pointer to a jmp_buf used to throw out when reading or writing past the
  // end of the array.  This is cast to a void * to avoid the need for
  // everyone to include setjmp.h.  We use a longjmp to avoid a lot of
  // status checking at intermediate calls.
  // Not yet used.
  // void *escape;
} CborState;

// Structure to allow apis to decode cbor field arrays.
typedef struct {
  uint8_t *ptr;                  // ptr to the (raw) cbor array payload header byte.
  uint16_t payloadSize;          // cbor array raw payload size (== bytes from array hdr to end of cbor payload). Used in decoding only.
  uint16_t numElementsToEncode;  // number of elements to write to cbor array. Used in encoding only.
  ZclipFieldData fieldData;      // holds field data in order for api to handle the cbor array decode.
} CborArray;

//----------------------------------------------------------------
// Encoding

// Get and set control to force definite length encoding.
bool emCborGetForceDefiniteLengthEncoding();
void emCborSetForceDefiniteLengthEncoding(bool force);

// Initialize 'state' for encoding into 'output'.
void emCborEncodeStart(CborState *state, uint8_t *output, uint16_t outputSize);

// Returns the number of bytes that have been encoded so far.
uint32_t emCborEncodeSize(const CborState *state);

// Encode one uint16_t value.
bool emCborEncodeKey(CborState *state, uint16_t key);

// Encode one (non-struct) value.
bool emCborEncodeValue(CborState *state,
                       uint8_t valueType,
                       uint16_t valueSize,
                       const uint8_t *valueLoc);

// Encode one struct value.
bool emCborEncodeStruct(CborState *state,
                        const ZclipStructSpec *structSpec,
                        const void *theStruct);

// Maps and arrays can either specify the number of elements up front or
// use a terminating 'break' marker.

#define emCborEncodeMap(state, count) \
  (emCborEncodeDefinite((state), CBOR_MAP, (count)))

#define emCborEncodeArray(state, count) \
  (emCborEncodeDefinite((state), CBOR_ARRAY, (count)))

#define emCborEncodeIndefiniteMap(state) \
  (emCborEncodeIndefinite((state), CBOR_MAP))

#define emCborEncodeIndefiniteArray(state) \
  (emCborEncodeIndefinite((state), CBOR_ARRAY))

bool emCborEncodeDefinite(CborState *state, uint8_t valueType, uint16_t count);
bool emCborEncodeIndefinite(CborState *state, uint8_t valueType);

// Ends the current indefinite map or array.
bool emCborEncodeBreak(CborState *state);

//----------------
// Utility functions that are wrappers around the above procedures.

// emCborEncodeStart() + emCborEncodeArray()
bool emCborEncodeArrayStart(CborState *state,
                            uint8_t *output,
                            uint16_t outputSize,
                            uint16_t count);

// emCborEncodeStart() + emCborEncodeIndefiniteArray()
bool emCborEncodeIndefiniteArrayStart(CborState *state,
                                      uint8_t *output,
                                      uint16_t outputSize);

// emCborEncodeStart() + emCborEncodeMap()
bool emCborEncodeMapStart(CborState *state,
                          uint8_t *output,
                          uint16_t outputSize,
                          uint16_t count);

// emCborEncodeStart() + emCborEncodeIndefiniteMap()
bool emCborEncodeIndefiniteMapStart(CborState *state,
                                    uint8_t *output,
                                    uint16_t outputSize);

// emCborEncodeKey() + emCborEncodeValue()
bool emCborEncodeMapEntry(CborState *state,
                          uint16_t key,
                          uint8_t valueType,
                          uint16_t valueSize,
                          const uint8_t *valueLoc);

// emCborEncodeStart() + emCborEncodeStruct() + emCborEncodeSize()
uint16_t emCborEncodeOneStruct(uint8_t *output,
                               uint16_t outputSize,
                               const ZclipStructSpec *structSpec,
                               const void *theStruct);

//----------------------------------------------------------------
// Decoding

// Initialize 'state' for decoding.
void emCborDecodeStart(CborState *state,
                       const uint8_t *input,
                       uint16_t inputSize);

// These return true for success and false if the end of the current array
// or map has been reached.
bool emCborDecodeStruct(CborState *state,
                        const ZclipStructSpec *structSpec,
                        void *theStruct);

bool emCborDecodeSequence(CborState *state, uint8_t valueType);

#define emCborDecodeMap(state) \
  (emCborDecodeSequence((state), CBOR_MAP))

#define emCborDecodeArray(state) \
  (emCborDecodeSequence((state), CBOR_ARRAY))

bool emCborDecodeValue(CborState *state,
                       uint8_t valueType,
                       uint16_t valueSize,
                       uint8_t *valueLoc);

bool emCborDecodeItem(CborState *state,
                      uint8_t type,
                      uint8_t **contentsLoc,
                      uint32_t *contentsLengthLoc);
bool emCborDecodeSequenceEnd(CborState *state);
bool emCborDecodeTag(CborState *state, uint32_t *value);

// Returns 0xFFFF if there are no remaining keys in the current map.
uint16_t emCborDecodeKey(CborState *state);

// Returns the type of the next value.
uint8_t emCborDecodePeek(CborState *state, uint32_t *length);
// Returns true if at the end of the current sequence.
bool emCborPeekSequenceEnd(CborState *state);

// Skips over the next value, returns false if we are at then of a sequence
// and there is no next value.
bool emCborDecodeSkipValue(CborState *state);

// Return false if CBOR payload is empty, otherwise true
bool emCborNonEmpty(CborState *state);

// Remove an unsigned integer of a given length from the end of CBOR payload
bool emCborEraseUnsignedInt(CborState *state, uint32_t length);

// Decodes cbor field array elements from the raw cbor payload (in *cborArray)
// into the specified output buffer.
uint16_t emCborDecodeFieldArrayIntoBuffer(CborArray *cborArray,
                                          uint8_t *buffer,
                                          uint16_t bufferSize,
                                          size_t bufferValueSize);

//----------------
// Wrapper functions that implement the old interface

bool emCborDecodeOneStruct(const uint8_t *input,
                           uint16_t inputSize,
                           const ZclipStructSpec *structSpec,
                           void *theStruct);

//----------------
// Misc defines

#define CBOR_TEXT_KEY_MAX_LENGTH  8

uint16_t emCborToString(const uint8_t *cbor,
                        uint16_t cborLength,
                        uint8_t *buf,
                        uint16_t bufLength);

#endif // CBOR_H
