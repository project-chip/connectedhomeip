/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include "core/ember-stack.h"
#include "zclip-struct.h"
#include "cbor.h"
#include "zcl-core-types.h"

typedef struct {
  bool     field0;
  uint8_t  field1;
  uint16_t field2;
  uint32_t field3;
  int8_t   field4;
  int16_t  field5;
  int32_t  field6;
  uint8_t  field7[7];
  char     field8[7];
  EmberZclStringType_t field9;
  EmberZclStringType_t field10;
} ZclipTestStruct;

#define EMBER_ZCLIP_STRUCT ZclipTestStruct

ZclipStructSpec zclipTestStructSpec[] = {
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                     11,        // fieldCount
                     NULL),     // names
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_BOOLEAN, field0),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, field1),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, field2),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, field3),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_INTEGER, field4),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_INTEGER, field5),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_INTEGER, field6),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY, field7),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING, field8),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING, field9),
  EMBER_ZCLIP_FIELD(EMBER_ZCLIP_TYPE_BINARY, field10)
};

#undef EMBER_ZCLIP_STRUCT

typedef struct {
  bool field0;
  bool field1;
} MixedKeysTestStruct;
#define EMBER_ZCLIP_STRUCT MixedKeysTestStruct
ZclipStructSpec mixedKeysTestStructSpec[] = {
  EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                     2,        // fieldCount
                     NULL),     // names
  EMBER_ZCLIP_FIELD_INDEXED(EMBER_ZCLIP_TYPE_BOOLEAN, field0),
  EMBER_ZCLIP_FIELD_NAMED(EMBER_ZCLIP_TYPE_BOOLEAN, field1, "e"),
};
#undef EMBER_ZCLIP_STRUCT

// Prints encoded CBOR to stderr
boolean debug = false;

static void printBuffer(uint8_t * buffer, uint16_t length);

static void compareTestStructs(ZclipTestStruct *testStruct,
                               ZclipTestStruct *decodedStruct)
{
  // Check field9 separately because it is a pointer.
  // printBuffer(testStruct->field9.ptr, testStruct->field9.length);
  // printBuffer(decodedStruct->field9.ptr, decodedStruct->field9.length);
  assert(memcmp(testStruct->field9.ptr,
                decodedStruct->field9.ptr,
                testStruct->field9.length)
         == 0);
// Check field10 separately because it is a pointer.
  assert(memcmp(testStruct->field10.ptr,
                decodedStruct->field10.ptr,
                testStruct->field10.length)
         == 0);
  // Fix it so that the following memcmp() won'd die on fields 9 and 10.
  decodedStruct->field9.ptr = testStruct->field9.ptr;
  decodedStruct->field10.ptr = testStruct->field10.ptr;
  assert(memcmp(testStruct, decodedStruct, sizeof(ZclipTestStruct)) == 0);
  fputc('.', stderr);
}

// These are the CBOR types used to encode the test struct.
static const uint8_t structTypes[] = {
  CBOR_TRUE,
  CBOR_UNSIGNED,
  CBOR_UNSIGNED,
  CBOR_UNSIGNED,
  CBOR_NEGATIVE,
  CBOR_NEGATIVE,
  CBOR_NEGATIVE,
  CBOR_BYTES,
  CBOR_TEXT,
  CBOR_TEXT,
  CBOR_BYTES
};

// Test that the encoded bytes represent the CBOR map with key and
// value types that correspond to the structTypes hardcoded above
static void peekTest(uint8_t *encoded, uint16_t encodedLength)
{
  CborState state;
  uint32_t length;
  uint32_t *lengthPointer = NULL;
  uint8_t i;

  while (true) {
    // Skip the whole thing.
    emCborDecodeStart(&state, encoded, encodedLength);
    assert(emCborDecodeSkipValue(&state));
    assert(state.finger == state.end);

    // Peek at and then skip the individual elements.
    emCborDecodeStart(&state, encoded, encodedLength);
    assert(emCborDecodePeek(&state, lengthPointer) == CBOR_MAP);
    assert(emCborDecodeMap(&state));
    for (i = 0; i < sizeof(structTypes); i++) {
      assert(emCborDecodePeek(&state, lengthPointer) == CBOR_UNSIGNED);    // key
      assert(emCborDecodeSkipValue(&state));
      assert(emCborDecodePeek(&state, lengthPointer) == structTypes[i]);   // value
      assert(emCborDecodeSkipValue(&state));
    }
    assert(emCborDecodePeek(&state, lengthPointer) == CBOR_BREAK);
    assert(!emCborDecodeSkipValue(&state));
    assert(emCborDecodePeek(&state, lengthPointer) == CBOR_BREAK);

    if (lengthPointer == NULL) {
      lengthPointer = &length;
    } else {
      break;
    }
  }
}

static bool enableDumpCborState = false;
static void dumpCborState(CborState *state)
{
  int i;
  if (enableDumpCborState && state != NULL) {
    fprintf(stderr, "start     : 0x%08x\n", (uint32_t)state->start);
    fprintf(stderr, "finger    : 0x%08x\n", (uint32_t)state->finger);
    fprintf(stderr, "end       : 0x%08x\n", (uint32_t)state->end);
    fprintf(stderr, "nestDepth : %d\n", state->nestDepth);
    for (i = 0; i < MAX_MAP_ARRAY_NESTING; ++i) {
      fprintf(stderr, "  nest %d count %d phead 0x%08x\n",
              i + 1,
              state->nestStack[i].count,
              (uint32_t)(state->nestStack[i].phead));
    }
  }
}

static void printBuffer(uint8_t * buffer, uint16_t length)
{
  uint16_t i;
  fprintf(stderr, "\n%d bytes\n", length);
  for (i = 0; i < length; i++) {
    fprintf(stderr, " %02X", buffer[i]);
  }
  fprintf(stderr, "\n");
}

// Test values for variable length text and binary strings.
static const EmberZclStringType_t field9Value = {
  .ptr = "zxcvb",
  .length = 5
};
static uint8_t field10Binary[] = { 0x0f, 0x0a, 0x0c, 0x0a, 0x0d, 0x0e };
static const EmberZclStringType_t field10Value = {
  .ptr = field10Binary,
  .length = 5
};

int main(int argc, char *argv[])
{
  ZclipStructData structData;
  ZclipFieldData fieldData;
  uint16_t i;

  fprintf(stderr, "[%s ", argv[0]);

  if (!emExpandZclipStructData(zclipTestStructSpec, &structData)) {
    fprintf(stderr, "... struct data init failed at %s, line %d.", __FILE__, __LINE__);
  }

  CborState state;
  ZclipTestStruct testStruct;

  memset(&testStruct, 0, sizeof(testStruct));

  testStruct.field0 = true;
  testStruct.field1 = 0xFF;
  testStruct.field2 = 12345;
  testStruct.field3 = 1234567;
  testStruct.field4 = -128;
  testStruct.field5 = -12345;
  testStruct.field6 = -1234567;
  MEMCOPY(&testStruct.field7, "qwertyu", 7);
  MEMCOPY(&testStruct.field8, "asdf", 5);       // 5 to include the null
  MEMCOPY(&testStruct.field9, &field9Value, sizeof(EmberZclStringType_t));
  MEMCOPY(&testStruct.field10, &field10Value, sizeof(EmberZclStringType_t));

  uint8_t buffer[3][1000];

  // Encode the test struct.
  emCborEncodeStart(&state, buffer[0], sizeof(buffer[0]));
  emCborEncodeStruct(&state, zclipTestStructSpec, (uint8_t *) &testStruct);
  uint16_t length = emCborEncodeSize(&state);

  fputc('.', stderr);

  // Prints out the encoding so that it can be tested using the web
  // app at http://cbor.me/.
  if (debug) {
    printBuffer(buffer[0], length);
    dumpCborState(&state);
  }

  peekTest(buffer[0], length);

  fputc('.', stderr);

  ZclipTestStruct decodedStruct;
  memset(&decodedStruct, 0, sizeof(decodedStruct));

  // Decode it an check that we got the same values back again.
  emCborDecodeStart(&state, buffer[0], sizeof(buffer[0]));
  assert(emCborDecodeStruct(&state,
                            zclipTestStructSpec,
                            (uint8_t *) &decodedStruct));
  compareTestStructs(&testStruct, &decodedStruct);

  // Same again, but encode and decode as if the fields were attributes.
  // We do it thrice, first with fixed length maps and arrays, second
  // with indefinite lengths forced to definite length, and third with
  // indefinite lengths. The results of the first two of these are compared
  // and should be identical (both are definite length encoding).

  uint8_t indefinite;
  for (indefinite = 0; indefinite < 3; indefinite++) {
    emCborSetForceDefiniteLengthEncoding(indefinite == 1);
    emCborEncodeStart(&state, buffer[indefinite], sizeof(buffer[indefinite]));
    if (!emExpandZclipStructData(zclipTestStructSpec, &structData)) {
      fprintf(stderr, "... struct data init failed at %s, line %d.", __FILE__, __LINE__);
    }

    if (indefinite) {
      emCborEncodeIndefiniteMap(&state);
    } else {
      emCborEncodeMap(&state, structData.fieldCount);
    }

    for (i = 0; i < structData.fieldCount; i++) {
      emGetNextZclipFieldData(&structData, &fieldData);
      emCborEncodeMapEntry(&state,
                           100 + i,
                           fieldData.valueType,
                           fieldData.valueSize,
                           ((uint8_t *) &testStruct) + fieldData.valueOffset);
    }

    if (indefinite) {
      emCborEncodeBreak(&state);
    }

    length = state.finger - buffer[indefinite];

    fputc('.', stderr);

    if (debug) {
      printBuffer(buffer[indefinite], length);
      dumpCborState(&state);
    }

    peekTest(buffer[indefinite], length);

    emResetZclipFieldData(&structData);
    memset(&decodedStruct, 0, sizeof(decodedStruct));
    emCborDecodeStart(&state, buffer[indefinite], length);
    assert(emCborDecodeMap(&state));

    for (i = 0;; i++) {
      uint16_t key = emCborDecodeKey(&state);
      if (key == 0xFFFF) {
        assert(i == structData.fieldCount);
        break;
      }
      assert(key == 100 + i);
      emGetNextZclipFieldData(&structData, &fieldData);
      emCborDecodeValue(&state,
                        fieldData.valueType,
                        fieldData.valueSize,
                        ((uint8_t *) &decodedStruct) + fieldData.valueOffset);
    }

    // For certain struct field types, decoding can modify the encoded buffer
    // content inline, so compare definite and forced definite buffers after
    // both have been decoded / similarly modified (instead of immediately after
    // encoding -- but before decoding -- the forced indefinite pass; as is done
    // in most other test cases below.
    if (indefinite == 1) {
      assert(memcmp(buffer[0], buffer[1], length) == 0);
    }

    compareTestStructs(&testStruct, &decodedStruct);
  }

  // Arrays

  for (indefinite = 0; indefinite < 3; indefinite++) {
    emCborSetForceDefiniteLengthEncoding(indefinite == 1);
    emCborEncodeStart(&state, buffer[indefinite], sizeof(buffer[indefinite]));
    if (indefinite) {
      emCborEncodeIndefiniteArray(&state);
    } else {
      emCborEncodeArray(&state, 5);
    }
    for (i = 0; i < 5; i++) {
      uint8_t j;
      if (indefinite) {
        emCborEncodeIndefiniteArray(&state);
      } else {
        emCborEncodeArray(&state, 5 - i);
      }
      for (j = 0; j < 5 - i; j++) {
        uint16_t v = 100 + j;
        emCborEncodeValue(&state,
                          EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                          sizeof(v),
                          (uint8_t *) &v);
      }
      if (indefinite) {
        emCborEncodeBreak(&state);
      }
    }

    if (indefinite) {
      emCborEncodeBreak(&state);
    }

    length = state.finger - buffer[indefinite];

    fputc('.', stderr);

    if (debug) {
      printBuffer(buffer[indefinite], length);
      dumpCborState(&state);
    }

    if (indefinite == 1) {
      assert(memcmp(buffer[0], buffer[1], length) == 0);
    }

    emCborDecodeStart(&state, buffer[indefinite], length);
    emCborDecodeArray(&state);

    for (i = 0; i < 5; i++) {
      uint8_t j;
      emCborDecodeArray(&state);
      for (j = 0; j < 5 - i; j++) {
        uint16_t v;
        assert(emCborDecodeValue(&state,
                                 EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                 sizeof(v),
                                 (uint8_t *) &v));
        assert(v == 100 + j);
      }
      assert(!emCborDecodeValue(&state, 0, 0, NULL));
    }
    assert(!emCborDecodeValue(&state, 0, 0, NULL));
  }

  // Long Maps (more than 23 elements)
  // For the forced definite case, this tests the shifting of map content
  // forward in the encoding buffer to create space for a length byte.

  for (indefinite = 0; indefinite < 3; indefinite++) {
    emCborSetForceDefiniteLengthEncoding(indefinite == 1);
    emCborEncodeStart(&state, buffer[indefinite], sizeof(buffer[indefinite]));
    if (indefinite) {
      emCborEncodeIndefiniteMap(&state);
    } else {
      emCborEncodeMap(&state, 25);
    }
    for (i = 0; i < 25; i++) {
      emCborEncodeKey(&state, i);
      uint8_t j;
      if (indefinite) {
        emCborEncodeIndefiniteMap(&state);
      } else {
        emCborEncodeMap(&state, (i == 12 ? 25 : 1));
      }
      for (j = 0; j < (i == 12 ? 25 : 1); j++) {
        uint16_t k = j;
        uint16_t v = 100 + j;
        emCborEncodeKey(&state, k);
        emCborEncodeValue(&state,
                          EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                          sizeof(v),
                          (uint8_t *) &v);
      }
      if (indefinite) {
        emCborEncodeBreak(&state);
      }
    }

    if (indefinite) {
      emCborEncodeBreak(&state);
    }

    length = state.finger - buffer[indefinite];

    fputc('.', stderr);

    if (debug) {
      printBuffer(buffer[indefinite], length);
      dumpCborState(&state);
    }

    if (indefinite == 1) {
      assert(memcmp(buffer[0], buffer[1], length) == 0);
    }

    emCborDecodeStart(&state, buffer[indefinite], length);
    emCborDecodeMap(&state);

    for (i = 0; i < 25; i++) {
      uint16_t k;
      uint8_t j;
      k = emCborDecodeKey(&state);
      assert(k == i);
      emCborDecodeMap(&state);
      for (j = 0; j < (i == 12 ? 25 : 1); j++) {
        uint16_t k;
        uint16_t v;
        k = emCborDecodeKey(&state);
        assert(k == j);
        assert(emCborDecodeValue(&state,
                                 EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                 sizeof(v),
                                 (uint8_t *) &v));
        assert(v == 100 + j);
      }
      assert(!emCborDecodeValue(&state, 0, 0, NULL));
    }
    assert(!emCborDecodeValue(&state, 0, 0, NULL));
  }

  // Long Arrays (more than 23 elements)
  // For the forced definite case, this tests the shifting of array content
  // forward in the encoding buffer to create space for a length byte.

  for (indefinite = 0; indefinite < 3; indefinite++) {
    emCborSetForceDefiniteLengthEncoding(indefinite == 1);
    emCborEncodeStart(&state, buffer[indefinite], sizeof(buffer[indefinite]));
    if (indefinite) {
      emCborEncodeIndefiniteArray(&state);
    } else {
      emCborEncodeArray(&state, 25);
    }
    for (i = 0; i < 25; i++) {
      uint8_t j;
      if (indefinite) {
        emCborEncodeIndefiniteArray(&state);
      } else {
        emCborEncodeArray(&state, (i == 12 ? 25 : 1));
      }
      for (j = 0; j < (i == 12 ? 25 : 1); j++) {
        uint16_t v = 100 + j;
        emCborEncodeValue(&state,
                          EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                          sizeof(v),
                          (uint8_t *) &v);
      }
      if (indefinite) {
        emCborEncodeBreak(&state);
      }
    }

    if (indefinite) {
      emCborEncodeBreak(&state);
    }

    length = state.finger - buffer[indefinite];

    fputc('.', stderr);

    if (debug) {
      printBuffer(buffer[indefinite], length);
      dumpCborState(&state);
    }

    if (indefinite == 1) {
      assert(memcmp(buffer[0], buffer[1], length) == 0);
    }

    emCborDecodeStart(&state, buffer[indefinite], length);
    emCborDecodeArray(&state);

    for (i = 0; i < 25; i++) {
      uint8_t j;
      emCborDecodeArray(&state);
      for (j = 0; j < (i == 12 ? 25 : 1); j++) {
        uint16_t v;
        assert(emCborDecodeValue(&state,
                                 EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                                 sizeof(v),
                                 (uint8_t *) &v));
        assert(v == 100 + j);
      }
      assert(!emCborDecodeValue(&state, 0, 0, NULL));
    }
    assert(!emCborDecodeValue(&state, 0, 0, NULL));
  }

  compareTestStructs(&testStruct, &decodedStruct);

  // {3: 1, 9: "s"}
  uint8_t sparseCbor[] = { 0xa2, 0x03, 0x01, 0x09, 0x61, 0x73 };
  uint8_t sparseString[] = { 0x73 };
  ZclipTestStruct sparseStruct = { 0 };
  sparseStruct.field3 = 0x01;
  sparseStruct.field9.ptr = sparseString;
  sparseStruct.field9.length = 1;
  MEMSET(&decodedStruct, 0, sizeof(ZclipTestStruct));
  emCborDecodeStart(&state, sparseCbor, sizeof(sparseCbor));
  assert(emCborDecodeStruct(&state,
                            zclipTestStructSpec,
                            (uint8_t *) &decodedStruct));
  compareTestStructs(&sparseStruct, &decodedStruct);

  fputc('.', stderr);

  // Test TLV tag key
  {
    typedef struct {
      bool    field0;
      uint8_t field1;
    } TlvKeysTestStruct;
#define EMBER_ZCLIP_STRUCT TlvKeysTestStruct
    ZclipStructSpec tlvKeysTestStructSpec[] = {
      EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
                         2,            // fieldCount
                         NULL),         // names
      EMBER_ZCLIP_FIELD_INDEXED(EMBER_ZCLIP_TYPE_BOOLEAN, field0),
      EMBER_ZCLIP_FIELD_TLV(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, field1, 7),
    };
#undef EMBER_ZCLIP_STRUCT

    // Test CBOR decoding
    // {0: true, -8: 42}  TLV 7 corresponds to key -8
    const uint8_t wantTlvCbor[] = { 0xa2, 0x00, 0xf5, 0x27, 0x18, 0x2a };
    ZclipTestStruct haveTlvStruct;
    MEMSET(&haveTlvStruct, 0, sizeof(haveTlvStruct));
    const ZclipTestStruct wantTlvStruct = {
      .field0 = true,
      .field1 = 42,
    };
    emCborDecodeStart(&state, wantTlvCbor, sizeof(wantTlvCbor));
    assert(emCborDecodeStruct(&state, tlvKeysTestStructSpec, (uint8_t *)&haveTlvStruct));
    assert(memcmp(&wantTlvStruct, &haveTlvStruct, sizeof(wantTlvStruct)) == 0);
    fputc('.', stderr);

    // Test CBOR encoding
    // {0: true, -8: 42} encoded as an indefinite map. TLV 7 corresponds to key -8
    const uint8_t indefTlvCbor[] = { 0xbf, 0x00, 0xf5, 0x27, 0x18, 0x2a, 0xff };
    uint8_t buffer[1000];
    // Encode the test struct.
    emCborEncodeStart(&state, buffer, sizeof(buffer));
    emCborEncodeStruct(&state, tlvKeysTestStructSpec, (uint8_t *) &wantTlvStruct);
    length = emCborEncodeSize(&state);
    assert(memcmp(buffer, indefTlvCbor, sizeof(indefTlvCbor)) == 0);
    fputc('.', stderr);

    // Pass a TLV tag that is out of range, expect emCborDecodeStruct to fail
    // {0: true, -66666: 42}
    const uint8_t badTlvCbor[] = { 0xa2, 0x00, 0xf5, 0x3a, 0x00, 0x01, 0x04, 0x69, 0x18, 0x2a };
    emCborDecodeStart(&state, badTlvCbor, sizeof(badTlvCbor));
    assert(emCborDecodeStruct(&state, tlvKeysTestStructSpec, (uint8_t *)&haveTlvStruct) == false);
    fputc('.', stderr);
  }

  // The remaining test cases use indefinite length encoding
  emCborSetForceDefiniteLengthEncoding(false);

  // {0: true, "e": false}
  const uint8_t wantMixedCbor[] = { 0xbf, 0x00, 0xf5, 0x61, 0x65, 0xf4, 0xff };
  const ZclipTestStruct wantMixedStruct = {
    .field0 = true,
    .field1 = false,
  };
  ZclipTestStruct haveMixedStruct;
  assert(emCborDecodeOneStruct(wantMixedCbor,
                               sizeof(wantMixedCbor),
                               mixedKeysTestStructSpec,
                               &haveMixedStruct));
  assert(memcmp(&wantMixedStruct,
                &haveMixedStruct,
                sizeof(MixedKeysTestStruct))
         == 0);

  MEMSET(buffer[0], 0, sizeof(buffer[0]));
  length = emCborEncodeOneStruct(buffer[0],
                                 sizeof(buffer[0]),
                                 mixedKeysTestStructSpec,
                                 &haveMixedStruct);
  assert(sizeof(wantMixedCbor) == length);
  assert(memcmp(wantMixedCbor, buffer[0], sizeof(wantMixedCbor)) == 0);

  fprintf(stderr, " done]\n");

  // {0:1, 2:["a","b"]}
  const uint8_t mapOfArraysCbor[] = { 0xbf, 0x00, 0x01, 0x02, 0x9f, 0x61, 0x61, 0x61, 0x62, 0xff, 0xff };
  const uint8_t arrayValue = 0x01;

  MEMSET(buffer[0], 0, sizeof(buffer[0]));
  emCborEncodeIndefiniteMapStart(&state, buffer[0], sizeof(buffer[0]));
  emCborEncodeMapEntry(&state,
                       0, // key
                       EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,
                       sizeof(arrayValue),
                       &arrayValue);
  emCborEncodeKey(&state, 2);
  emCborEncodeIndefiniteArray(&state);
  emCborEncodeValue(&state,
                    EMBER_ZCLIP_TYPE_STRING,
                    1,
                    (const uint8_t *)"a");
  emCborEncodeValue(&state,
                    EMBER_ZCLIP_TYPE_STRING,
                    1,
                    (const uint8_t *)"b");
  emCborEncodeBreak(&state);
  emCborEncodeBreak(&state);
  assert(memcmp(buffer[0],
                mapOfArraysCbor,
                sizeof(mapOfArraysCbor)) == 0);

  return 0;
}
