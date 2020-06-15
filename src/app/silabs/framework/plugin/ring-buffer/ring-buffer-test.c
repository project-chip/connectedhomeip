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
 * @brief Ring Buffer Unit tests
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "ring-buffer.h"

#include "app/framework/test/test-framework.h"

//------------------------------------------------------------------------------
// Globals

#define RING_BUFFER_SIZE 50

static uint8_t buffer[RING_BUFFER_SIZE];

static EmberRingBuffer ring = {
  RING_BUFFER_SIZE,
  // Don't care about other variables
};

#define MAX_TEST_SIZE RING_BUFFER_SIZE
#define MAX_STRING_SIZE 1000
#define MAX_ARRAY_SIZES 10

static uint8_t testItem[MAX_TEST_SIZE];
static uint16_t addEntryNumber = 0;

#define verifyLastEntry(length) \
  verifyLastEntryWithFileInfo(length, __FILE__, __LINE__)

#define verifyAddEntryNumber(ringBufferEntryNumber, length, numberOfAddEntry) \
  verifyAddEntryNumberWithFileInfo(ringBufferEntryNumber,                     \
                                   length,                                    \
                                   numberOfAddEntry,                          \
                                   __FILE__,                                  \
                                   __LINE__)

//------------------------------------------------------------------------------
// Forward Declarations

static void runAllTests(void);
static void runAddTests(void);
static void runAppendTests(void);
static void runGetEntryTests(void);
static void runNarrowWidthFieldTests(void);

//------------------------------------------------------------------------------
// Stubs

uint16_t emberAfLongStringLength(const uint8_t *buffer)
{
  // Should not be called during unit testing
  assert(0);
  return 0;
}

uint8_t emberAfStringLength(const uint8_t *buffer)
{
  // Should not be called during unit testing
  assert(0);
  return 0;
}

//------------------------------------------------------------------------------

static void deleteCallback(uint16_t entryNumber, uint16_t entrySize)
{
}

// Initialize Globals
static void setupGlobals(void)
{
  uint8_t i;

  addEntryNumber = 0;

  // Initialize test data with a pattern so we can compare against later
  for (i = 0; i < MAX_TEST_SIZE; i++) {
    testItem[i] = (uint8_t)i;
  }

  note(".");
}

static void addEntry(uint16_t size)
{
  char formattedString[MAX_STRING_SIZE];
  MEMSET(formattedString, 0, MAX_STRING_SIZE);

  snprintf(formattedString,
           MAX_STRING_SIZE,
           "Calling emberAfPluginRingBufferAddEntry() addEntryNumber %d of %d bytes",
           addEntryNumber,
           size);

  debug("%s\n", formattedString);

  // We want the test sequence to be unique for each entry *ever* added.
  // Our test pattern is just a buffer of bytes incrementing 0 -> 255
  // When we add an entry the first digit matches the number of the added
  // entry.  So entry 3 will be the data
  // 030405060708 ... up to the corresponding length
  // Thus the &(testItem[addEntryNumber])
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferAddEntry(&ring,
                                                      &(testItem[addEntryNumber]),
                                                      size),
                      "EMBER_SUCCESS",
                      formattedString);
  addEntryNumber++;
}

// Array of sizes must be NULL terminated (since 0 is an invalid size)
static void setupRingBufferWithVariousSizes(bool initializeRing,
                                            bool wideLengthField,
                                            uint16_t* arrayOfSizes)
{
  uint8_t i = 0;
  if (initializeRing) {
    expectComparisonHex(EMBER_SUCCESS,
                        emberAfPluginRingBufferInitStruct(&ring,
                                                          wideLengthField,
                                                          deleteCallback,
                                                          buffer),
                        "EMBER_SUCCESS",
                        "emberAfPluginRingBufferInitStruct()");
  }
  while (arrayOfSizes[i] != 0) {
    addEntry(arrayOfSizes[i]);
    i++;
  }
}

void printAllEntries(void)
{
  uint16_t returnDataSize;
  uint16_t entryTotalSize;
  uint8_t returnData[MAX_TEST_SIZE];
  MEMSET(returnData, 0xFF, MAX_TEST_SIZE);
  bool keepGoing = true;
  uint16_t entryNumber = 0;
  uint16_t i;
  EmberStatus status;

  if (!isTestFrameworkDebugOn()) {
    return;
  }

  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferInitIterator(&ring),
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferInitIterator()");

  do {
    // We just want to print the entries and DON'T want extra debug
    // to print from the functions below.  Temporaily disable
    // debug so the contents of the ring buffer can be nicely
    // printed in a contiguous output.
    setTestFrameworkDebug(false);

    expectComparisonHex(EMBER_SUCCESS,
                        emberAfPluginRingBufferGetEntryByIterator(&ring,
                                                                  0, // index in entry
                                                                  &entryTotalSize,
                                                                  MAX_TEST_SIZE,
                                                                  &returnDataSize,
                                                                  returnData),
                        "EMBER_SUCCESS",
                        "emberAfPluginRingBufferGetEntryByIterator()");

    expectComparisonDecimal(entryTotalSize,
                            returnDataSize,
                            "entryTotalSize",
                            "returnDataSize");
    setTestFrameworkDebug(true);

    debug("Entry %d, Index: %d, Length %d\n", entryNumber, ring.iteratorIndex, entryTotalSize);

    debug("%d: ", entryNumber);
    for (i = 0; i < entryTotalSize; i++) {
      debug("%X", returnData[i]);
      if (i > 0 && (i % 16 == 0)) {
        debug("\n");
      }
    }
    debug("\n");
    entryNumber++;

    status = emberAfPluginRingBufferIteratorNextEntry(&ring);
    if (status == EMBER_SUCCESS) {
      keepGoing = false;

      // Operation in progress means there are more entries to iterate through.
      // Anything else is a bug
    } else if (status != EMBER_OPERATION_IN_PROGRESS) {
      assert(0);
    }
  } while (keepGoing);

  debug("Tail: %d\n", ring.tailIndex);
}

static void verifyAddEntryNumberWithFileInfo(uint16_t ringBufferEntryNumber,
                                             uint16_t length,
                                             uint16_t numberOfAddEntry,
                                             const char* file,
                                             int lineNumber)
{
  uint16_t entryTotalSize;
  uint8_t returnData[MAX_TEST_SIZE];
  uint16_t returnDataSize;
  char formattedString[MAX_STRING_SIZE];

  MEMSET(returnData, 0xFF, MAX_TEST_SIZE);

  // When writing data, we want each item to be unique so our verify
  // doesn't check the wrong entries and pass the test.
  // Our test pattern is a large buffer of bytes incrementing from 0 -> 255
  // When we write, we just start from a different point in the test buffer
  // so the verify must start from the same point.  That point in the test
  // data is equal to the addedEntryNumber.
  // Every entry ever added to the ring buffer (even after a delete) will
  // keep incrementing the initial point in the test buffer.
  // So when we write addEntryNumber 3 (0 indexed) it corresponds to the pattern
  // 030405060708 ... with the corresponding length

  // Thus the &(testItem[addEntryNumber]) below, and for other verifyData() calls

  expect(EMBER_SUCCESS
         == emberAfPluginRingBufferGetEntryByEntryNumber(&ring,
                                                         ringBufferEntryNumber,
                                                         0, // index in entry
                                                         &entryTotalSize,
                                                         MAX_TEST_SIZE,
                                                         &returnDataSize,
                                                         returnData));
  snprintf(formattedString,
           MAX_STRING_SIZE,
           "Verifying ring buffer data at entry %d",
           ringBufferEntryNumber);
  verifyDataWithFileInfo(formattedString,
                         length,
                         &(testItem[numberOfAddEntry]),
                         returnDataSize,
                         returnData,
                         file,
                         lineNumber);
}

static void verifyLastEntryWithFileInfo(uint16_t length, const char* file, int lineNumber)
{
  // After an addEntry() call, the addEntryNumber is incremented.
  // To verify the last entry we must subtract 1 (see below).
  // So we want to make sure the addEntryNumber is not 0 when
  // we subtract 1.
  assert(addEntryNumber > 0);

  // We grab the last entry by subtracting 1 from the entryCount so make
  // sure it isn't zero.
  assert(ring.entryCount > 0);

  verifyAddEntryNumberWithFileInfo(ring.entryCount - 1,
                                   length,
                                   addEntryNumber - 1,
                                   file,
                                   lineNumber);
}

static void runAllTests(void)
{
  // Cases to test
  // 1. Simple wrap - Last Element index < first elment index
  // 2. Length field can only be partially written due to limited free space.
  // 3. Length field crosses to beginning of buffer
  // 4. Wrap with the length field at end of buffer, data at beginning.
  // 5. Writing an element that requires deleting multiple entires to make space.
  // 6. Wrap of data, but not length, to beginning of buffer.
  // 7. Append of data - simple case
  // 8. Append of data with wrap
  // 9. Append of data where length field crosses buffer end
  // 10. Append of data where data to append crosses boundary
  // 11. Append causes the entry to wrap to itself
  //     (can't delete and make space)
  // 12. Get an entry where the returnDataSize < entryLength
  // 13. Get an entry where the requested dataIndexInEntry is
  //     not 0.
  // 14. Get an entry where the requested dataIndexInEntry is
  //     greater than the end of the buffer.
  // 15. Initialize an iterator, advance the iterator,
  //     add an entry that deletes the item that the iterator
  //     is on.  Try to get the entry the iterator is on.

  // 16. Test 1-byte length fields

  runAddTests();
  runAppendTests();
  runGetEntryTests();

  runNarrowWidthFieldTests();
}

static void runAddTests(void)
{
  uint16_t entryTotalSize;
  uint8_t testItemLength = 10;
  uint8_t returnData[MAX_TEST_SIZE];
  uint16_t returnDataSize;
  uint16_t arrayOfSizes[MAX_ARRAY_SIZES];
  bool wideLengthField = true;

  MEMSET(arrayOfSizes, 0, sizeof(uint16_t) * MAX_ARRAY_SIZES);
  MEMSET(returnData, 0xFF, MAX_TEST_SIZE);

  setupGlobals();

  // The ring buffer can easily get stuck in an endless loop when there is a bug
  // in the code.  So we turn on a watchdog to catch these problems.
  enableExpectWatchdog();

  // All the tests in runAddTests() ASSUME WIDE LENGTH FIELD (i.e. 2-bytes)
  // That is the more complicated testing as it involves setting up
  // the buffer to hit various edge conditions.

  setupRingBufferWithVariousSizes(true,  // initialize ring?
                                  wideLengthField,
                                  arrayOfSizes);

  expectComparisonHex(EMBER_MESSAGE_TOO_LONG,
                      emberAfPluginRingBufferAddEntry(&ring,
                                                      NULL,
                                                      RING_BUFFER_SIZE + 1),
                      "EMBER_MESSAGE_TOO_LONG",
                      "emberAfPluginRingBufferAddEntry() with RING_BUFFER_SIZE + 1");

  arrayOfSizes[0] = 10;
  arrayOfSizes[1] = 10;

  setupRingBufferWithVariousSizes(false, wideLengthField, arrayOfSizes);
  printAllEntries();

  verifyAddEntryNumber(0,  // ring buffer entry number
                       testItemLength,
                       0); // addEntryNumber that we added

  // Verify item 1
  verifyLastEntry(testItemLength);

  // Add 2 more items of the same size
  setupRingBufferWithVariousSizes(false,
                                  wideLengthField,
                                  arrayOfSizes);
  expectComparisonDecimal(ring.entryCount, 4, "ring.EntryCount", "4");

  verifyAddEntryNumber(0,  // ring buffer entry number
                       testItemLength,
                       0); // addEntryNumber that we added

  // Tail Index should now be at 48 of 50.  Adding two more bytes causes a wrap and deletes
  // the first elment
  expectComparisonDecimal(ring.tailIndex, 48, "ring.tailIndex", "48");

  printAllEntries();

  // Tests: 1. Simple wrap, 4. Length field at end of buffer

  // We add +1 to the length to set ourselves up for test case 2, where
  // the length value can only be partially written.
  // Note, this simple wrap case also covers case 4 where the length field
  // is at the end of the buffer.
  testItemLength += 1;
  addEntry(testItemLength);
  expectComparisonDecimal(ring.entryCount, 4, "ring.EntryCount", "4");

  expectComparisonDecimal(11,
                          ring.data[MAX_TEST_SIZE - 2],
                          "Item 4 size",
                          "ring buffer data");

  printAllEntries();

  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferGetEntryByEntryNumber(&ring,
                                                                   3,
                                                                   0, // index into entry
                                                                   &entryTotalSize,
                                                                   MAX_TEST_SIZE,
                                                                   &returnDataSize,
                                                                   returnData),
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferGetEntryByEntryNumber(3)");

  verifyData("Verifying ring buffer data at entry 3",
             testItemLength,
             &(testItem[4]),
             returnDataSize,
             returnData);

  // Tail should now be at 11.  First element is at 12
  expectComparisonDecimal(ring.tailIndex, 11, "ring.tailIndex", "11");

  // 2. Length field can only be partially written due to limited free space.
  // Writing another element should trigger a delete.
  // Then writing our data should trigger another delete because there is not
  // enough space.

  // Item 5
  addEntry(testItemLength);
  expectComparisonDecimal(ring.tailIndex, 24, "ring.tailIndex", "24");
  verifyLastEntry(testItemLength);

  testItemLength = 10;

  // Item 6
  addEntry(testItemLength);
  printAllEntries();
  verifyLastEntry(testItemLength);

  testItemLength = 11;

  // Item 7

  // Test 5. Writing an element that requires deleting multiple entires to make space.
  // Test 6. Wrap of data, but not length, to beginning of buffer.
  addEntry(testItemLength);
  printAllEntries();
  verifyLastEntry(testItemLength);

  expectComparisonDecimal(ring.tailIndex, 49, "ring.tailIndex", "49");

  // Test: 3. Length field crosses to beginning of buffer
  // Item 8
  addEntry(testItemLength);
  printAllEntries();
  verifyLastEntry(testItemLength);
}

static void runAppendTests(void)
{
  uint8_t testItemLength = 10;
  uint16_t arrayOfSizes[MAX_ARRAY_SIZES];
  bool wideLengthField = true;

  MEMSET(arrayOfSizes, 0, sizeof(uint16_t) * MAX_ARRAY_SIZES);

  setupGlobals();

  // The ring buffer can easily get stuck in an endless loop when there is a bug
  // in the code.  So we turn on a watchdog to catch these problems.
  enableExpectWatchdog();

  arrayOfSizes[0] = 10;
  arrayOfSizes[1] = 10;
  arrayOfSizes[2] = 10;
  arrayOfSizes[3] = 10;

  setupRingBufferWithVariousSizes(true,
                                  wideLengthField,
                                  arrayOfSizes);

  printAllEntries();
  verifyLastEntry(testItemLength);

  // Test Case 7: Simple Append

  // To make our verify work correctly we need the test sequence to be correct.
  // i.e. Our entry 0C0D0E0F101112131415 is appended with two bytes, which
  // we want to be 1617.  The current addEntryNumber contains the index into our
  // test sequence of 0D, because it was incremented *after* our last add.  So subtract
  // 1 and we get the correct index into the whole sequence for this entry: 0C.
  // However, because we are appending after the original 10 bytes we must add
  // the testItemLength.  Thus why we write for the data to add pointer
  //    &(testItem[addEntryNumber - 1 + testItemLength])
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferAppendLastEntry(&ring,
                                                             &(testItem[addEntryNumber - 1 + testItemLength]),
                                                             2),  // append length
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferAppendLastEntry()");
  testItemLength += 2;
  printAllEntries();
  verifyLastEntry(testItemLength);

  // Test Case 8: Append of data with wrap
  // Now append 2 more bytes and the sequence will need to wrap to the beginning,
  // triggering a delete of the first entry
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferAppendLastEntry(&ring,
                                                             &(testItem[addEntryNumber - 1 + testItemLength]),
                                                             2),  // append length
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferAppendLastEntry()");
  testItemLength += 2;
  printAllEntries();
  verifyLastEntry(testItemLength);

  MEMSET(arrayOfSizes, 0, sizeof(uint16_t) * MAX_ARRAY_SIZES);
  arrayOfSizes[0] = 10;
  arrayOfSizes[1] = 10;
  arrayOfSizes[2] = 8;

  setupRingBufferWithVariousSizes(false, wideLengthField, arrayOfSizes);

  // Setup the last item so it is 1 less than the ring buffer size.
  testItemLength = 11;
  addEntry(testItemLength);
  printAllEntries();

  expectComparisonDecimal(ring.tailIndex,
                          ring.maxSize - 1,
                          "ring.tailIndex",
                          "ring.maxSize - 1");

  // Our next item should have a length field that crosses buffer boundaries,
  // which is what we need for test case 9.
  testItemLength = 10;
  addEntry(testItemLength);

  // Test Case 9: Append where the length field crosses buffer boundaries
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferAppendLastEntry(&ring,
                                                             &(testItem[addEntryNumber - 1 + testItemLength]),
                                                             2),  // append length
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferAppendLastEntry()");
  testItemLength += 2;
  printAllEntries();
  verifyLastEntry(testItemLength);

  MEMSET(arrayOfSizes, 0, sizeof(uint16_t) * MAX_ARRAY_SIZES);
  arrayOfSizes[0] = 8;
  arrayOfSizes[1] = 8;
  arrayOfSizes[2] = 8;

  setupRingBufferWithVariousSizes(false, wideLengthField, arrayOfSizes);

  expectComparisonDecimal(ring.tailIndex,
                          43,
                          "ring.tailIndex",
                          "43");
  // Last testItemLength we added was 8.
  testItemLength = 8;

  printAllEntries();

  // Test 10: Append where data wraps
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferAppendLastEntry(&ring,
                                                             &(testItem[addEntryNumber - 1 + testItemLength]),
                                                             10),  // append length
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferAppendLastEntry()");
  testItemLength += 10;
  printAllEntries();
  verifyLastEntry(testItemLength);

  // Last item length should be 18.

  // Test Case 11: Appending to entry would exceed buffer capacity.
  expectComparisonHex(EMBER_MESSAGE_TOO_LONG,
                      emberAfPluginRingBufferAppendLastEntry(&ring,
                                                             &(testItem[addEntryNumber - 1 + testItemLength]),
                                                             ring.maxSize - 18),  // append length
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferAppendLastEntry()");
}

static void runGetEntryTests(void)
{
  uint16_t testItemLength = 10;
  uint16_t arrayOfSizes[MAX_ARRAY_SIZES];
  uint16_t entryTotalSize;
  uint16_t returnDataSize;
  uint8_t returnData[MAX_TEST_SIZE];
  uint16_t indexWithinEntry = 0;

  uint16_t limitedMaxReturnSize = 4;

  bool wideLengthField = true;

  MEMSET(arrayOfSizes, 0, sizeof(uint16_t) * MAX_ARRAY_SIZES);
  MEMSET(returnData, 0xFF, MAX_TEST_SIZE);

  setupGlobals();

  // The ring buffer can easily get stuck in an endless loop when there is a bug
  // in the code.  So we turn on a watchdog to catch these problems.
  enableExpectWatchdog();

  arrayOfSizes[0] = 10;
  arrayOfSizes[1] = 10;
  arrayOfSizes[2] = 10;
  arrayOfSizes[3] = 10;

  setupRingBufferWithVariousSizes(true, wideLengthField, arrayOfSizes);

  printAllEntries();
  verifyLastEntry(testItemLength);

  // Test 12. Get an entry where the returnDataSize < entryLength

  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferGetEntryByEntryNumber(&ring,
                                                                   1, // entry number
                                                                   indexWithinEntry,
                                                                   &entryTotalSize,
                                                                   limitedMaxReturnSize,
                                                                   &returnDataSize,
                                                                   returnData),
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferGetEntryByEntryNumber()");

  // Now verify we didn't write any MORE data than the limited
  // return size by checking that the returned data is still 0xFF.
  expectComparisonHex(0xFF,
                      returnData[limitedMaxReturnSize],
                      "0xFF",
                      "returnData[limitedMaxReturnSize]");

  // This uses &(testItem[1]) because when we added the item we used
  // testItem[addEntryNumber]. addEntryNumber starts numbering from 0 and this
  // is verifying the 2nd element.
  verifyDataWithFileInfo("Verify of get entry where limitedMaxReturnSize < size of entry",
                         limitedMaxReturnSize,
                         &(testItem[1]),  // See comment above
                         returnDataSize,
                         returnData,
                         __FILE__,
                         __LINE__);

  // 13. Get an entry where the requested dataIndexInEntry is
  //     not 0.

  // Clear the data so we don't have any false positives with the data that
  // is returned in a subsequent test.
  MEMSET(returnData, 0xFF, MAX_TEST_SIZE);

  indexWithinEntry = 4;

  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferGetEntryByEntryNumber(&ring,
                                                                   1, // entry number
                                                                   indexWithinEntry,
                                                                   &entryTotalSize,
                                                                   limitedMaxReturnSize,
                                                                   &returnDataSize,
                                                                   returnData),
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferGetEntryByEntryNumber()");

  // Now verify we didn't write any MORE data than the limited
  // return size by checking that the returned data is still 0xFF.
  expectComparisonHex(0xFF,
                      returnData[limitedMaxReturnSize],
                      "0xFF",
                      "returnData[limitedMaxReturnSize]");

  // See also previous verifyDataWithFileInfo() above.
  // We verify data entered into our ring buffer based on the test sequence.
  // The test sequence is an increasing set of bytes from 0 - 255.  Every
  // entry added indexes into the test sequence based on the addEntryNumber.
  // We then add indexWithEntry because our get() call also indexes at a different point.
  verifyDataWithFileInfo("Verify an entry where the index within the entry is > 0",
                         limitedMaxReturnSize,
                         &(testItem[1 + indexWithinEntry]),  // See comment above
                         returnDataSize,
                         returnData,
                         __FILE__,
                         __LINE__);

  // 14. Get an entry where the requested dataIndexInEntry is
  //     greater than the end of the buffer.
  expectComparisonHex(EMBER_INDEX_OUT_OF_RANGE,
                      emberAfPluginRingBufferGetEntryByEntryNumber(&ring,
                                                                   1, // entry number
                                                                   testItemLength + 1, // index within entry
                                                                   &entryTotalSize,
                                                                   limitedMaxReturnSize,
                                                                   &returnDataSize,
                                                                   returnData),
                      "EMBER_INDEX_OUT_OF_RANGE",
                      "emberAfPluginRingBufferGetEntryByEntryNumber()");

  // 15. Initialize an iterator, advance the iterator,
  //     add an entry that deletes the item that the iterator
  //     is on.  Try to get the entry the iterator is on.
  expectComparisonHex(EMBER_SUCCESS,
                      emberAfPluginRingBufferInitIterator(&ring),
                      "EMBER_SUCCESS",
                      "emberAfPluginRingBufferInitIterator()");

  expectComparisonHex(EMBER_OPERATION_IN_PROGRESS,
                      emberAfPluginRingBufferIteratorNextEntry(&ring),
                      "EMBER_OPERATION_IN_PROGRESS",
                      "emberAfPluginRingBufferIteratorNextEntry()");

  // First entry should be at 0, second entry at 12.
  expectComparisonDecimal(ring.iteratorIndex, 12, "ring.iteratorIndex", "12");

  expectComparisonDecimal(ring.tailIndex, 48, "ring.iteratorIndex", "48");

  // Adding an item of 14 bytes should delete entry at index 0, and entry at
  // index 12.
  testItemLength = 14;
  addEntry(testItemLength);

  // TAKE CARE NOT TO CALL "printAllEntries()" before the expect below.
  // The printAllEntries() call uses the  iterator code and that context is
  // stored inside the ring buffer global.

  expectComparisonHex(EMBER_INVALID_CALL,
                      emberAfPluginRingBufferIteratorNextEntry(&ring),
                      "EMBER_OPERATION_IN_PROGRESS",
                      "emberAfPluginRingBufferIteratorNextEntry()");

  printAllEntries();

  verifyLastEntry(testItemLength);
}

static void runNarrowWidthFieldTests(void)
{
  uint16_t testItemLength = 10;
  uint16_t arrayOfSizes[MAX_ARRAY_SIZES];
  uint8_t returnData[MAX_TEST_SIZE];

  bool wideLengthField = false;

  MEMSET(arrayOfSizes, 0, sizeof(uint16_t) * MAX_ARRAY_SIZES);
  MEMSET(returnData, 0xFF, MAX_TEST_SIZE);

  setupGlobals();

  // The ring buffer can easily get stuck in an endless loop when there is a bug
  // in the code.  So we turn on a watchdog to catch these problems.
  enableExpectWatchdog();

  arrayOfSizes[0] = 10;
  arrayOfSizes[1] = 10;
  arrayOfSizes[2] = 10;
  arrayOfSizes[3] = 10;

  setupRingBufferWithVariousSizes(true, wideLengthField, arrayOfSizes);

  printAllEntries();
  verifyLastEntry(testItemLength);

  // Add an entry to cause the buffer to wrap
  addEntry(testItemLength);

  // The first entry (0) should now be the 2nd one we added (index 1 in testData),
  // since our prior add has caused the buffer to wrap and delete an entry.
  // The data of ring buffer entry 0 should start from testItem[1] since we increment
  // the index within the test data to make each entry unique.
  verifyAddEntryNumber(0,  // ring buffer entry number
                       testItemLength,
                       1);  // index within test data
}

int main(int argc, char* argv[])
{
  const TestCase allTests[] = {
    { "all-tests", runAllTests },
    { NULL },
  };

  return parseCommandLineAndExecuteTest(argc,
                                        argv,
                                        "af-plugin-ring-buffer-test",
                                        allTests);
}
