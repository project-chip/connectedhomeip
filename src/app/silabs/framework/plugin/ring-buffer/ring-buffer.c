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
 * @brief Ring Buffer Plugin
 * An implementation of a basic ring buffer where each entry in the buffer
 * has the following format:
 *
 *   length - 1 or 2 bytes, little-endian.
 *            This is the length of the data only and does not include the
 *            length field itself.
 *   data - variable length
 *
 * This plugin does NOT allocate the data for the buffer, but acts on a data
 * space allocated by another plugin.  Therefore it can act on multiple
 * ring buffers.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "ring-buffer.h"

//------------------------------------------------------------------------------
// Globals

#if defined(EMBER_SCRIPTED_TEST)
  #include "app/framework/test/test-framework.h"
#else
  #define expectCheckpoint(...)
  #define debug(...)
#endif

typedef enum {
  RING_BUFFER_ADD_NEW_ENTRY     = 0,
  RING_BUFFER_APPEND_LAST_ENTRY = 1,
} RingBufferAction;

//------------------------------------------------------------------------------
// Forward Declarations

static EmberStatus addOrAppend(EmberRingBuffer* ring,
                               uint8_t* dataToAdd,
                               uint16_t dataToAddSize,
                               RingBufferAction action);

static void copyFromRingBuffer(EmberRingBuffer* ring,
                               uint16_t index,
                               uint16_t length,
                               uint8_t* returnData);

static uint16_t getNextEntryIndex(EmberRingBuffer* ring,
                                  uint16_t index);

static uint16_t calculateMaxWriteLengthWithoutOverlap(EmberRingBuffer* ring);

static uint16_t calculateNewIndexWriteData(EmberRingBuffer* ring,
                                           uint16_t index,
                                           uint16_t length,
                                           uint8_t* newData);

static uint8_t getLengthFieldWidth(EmberRingBuffer* ring);

static uint16_t getEntryLength(EmberRingBuffer* ring, uint16_t index);

static uint16_t getIndexOfDataForEntry(EmberRingBuffer* ring,
                                       uint16_t indexOfEntry);

static EmberStatus internalGetEntry(EmberRingBuffer* ring,
                                    uint16_t index,
                                    uint16_t dataIndexInEntry,
                                    uint16_t* returnEntryTotalSize,
                                    uint16_t maxReturnSize,
                                    uint16_t* returnDataSize,
                                    uint8_t* returnData);

//------------------------------------------------------------------------------
// Public API

EmberStatus emberAfPluginRingBufferInitStruct(EmberRingBuffer* ringBufferStruct,
                                              bool wideLengthField,
                                              EmberAfPluginRingBufferDeleteFunction* deleteCallback,
                                              uint8_t* data)
{
  debug("%s\n", __PRETTY_FUNCTION__);
  if (data == NULL) {
    return EMBER_INVALID_CALL;
  }

  ringBufferStruct->firstItemIndex = 0;
  ringBufferStruct->tailIndex = 0;
  ringBufferStruct->entryCount = 0;
  ringBufferStruct->data = data;
  ringBufferStruct->deleteCallback = deleteCallback;
  ringBufferStruct->deletedItems = 0;
  ringBufferStruct->addedItems = 0;
  ringBufferStruct->lastItemLengthIndex = 0;
  ringBufferStruct->iteratorValid = false;
  ringBufferStruct->lengthFieldWidthBytes =
    (wideLengthField
     ? EMBER_AF_PLUGIN_RING_BUFFER_WIDE_LENGTH_ENTRY_OVERHEAD
     : EMBER_AF_PLUGIN_RING_BUFFER_NARROW_LENGTH_ENTRY_OVERHEAD);

  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginRingBufferAddEntry(EmberRingBuffer* ring,
                                            uint8_t* dataToAdd,
                                            uint16_t dataToAddSize)
{
  return addOrAppend(ring,
                     dataToAdd,
                     dataToAddSize,
                     RING_BUFFER_ADD_NEW_ENTRY);
}

EmberStatus emberAfPluginRingBufferGetEntryByEntryNumber(EmberRingBuffer* ring,
                                                         uint16_t entryNumber,
                                                         uint16_t dataIndexInEntry,
                                                         uint16_t* returnEntryTotalSize,
                                                         uint16_t maxReturnSize,
                                                         uint16_t* returnDataSize,
                                                         uint8_t* returnData)
{
  uint16_t currentEntryNumber = 0;
  uint16_t index = ring->firstItemIndex;
  EmberStatus status;

  debug("\n==> %s\n", __PRETTY_FUNCTION__);

  if (ring->entryCount <= entryNumber) {
    debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
    return EMBER_INDEX_OUT_OF_RANGE;
  }

  while (currentEntryNumber != entryNumber) {
    index = getNextEntryIndex(ring, index);
    currentEntryNumber++;
  }

  status = internalGetEntry(ring,
                            index,
                            dataIndexInEntry,
                            returnEntryTotalSize,
                            maxReturnSize,
                            returnDataSize,
                            returnData);

  debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
  return status;
}

EmberStatus emberAfPluginRingBufferGetLastEntry(EmberRingBuffer* ring,
                                                uint16_t dataIndexInEntry,
                                                uint16_t* returnEntryTotalSize,
                                                uint16_t maxReturnSize,
                                                uint16_t* returnDataSize,
                                                uint8_t* returnData)
{
  return emberAfPluginRingBufferGetEntryByEntryNumber(ring,
                                                      ring->entryCount - 1,
                                                      dataIndexInEntry,
                                                      returnEntryTotalSize,
                                                      maxReturnSize,
                                                      returnDataSize,
                                                      returnData);
}

EmberStatus emberAfPluginRingBufferInitIterator(EmberRingBuffer* ring)
{
  if (ring->entryCount == 0) {
    return EMBER_INVALID_CALL;
  }

  ring->iteratorIndex = ring->firstItemIndex;
  ring->iteratorValid = true;
  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginRingBufferIteratorNextEntry(EmberRingBuffer* ring)
{
  if (ring->iteratorValid == false) {
    return EMBER_INVALID_CALL;
  }

  if (ring->iteratorIndex == ring->lastItemLengthIndex) {
    ring->iteratorValid = false;
    return EMBER_SUCCESS;
  }

  ring->iteratorIndex = getNextEntryIndex(ring, ring->iteratorIndex);
  return EMBER_OPERATION_IN_PROGRESS;
}

EmberStatus emberAfPluginRingBufferGetEntryByIterator(EmberRingBuffer* ring,
                                                      uint16_t dataIndexInEntry,
                                                      uint16_t* returnEntryTotalSize,
                                                      uint16_t maxReturnSize,
                                                      uint16_t* returnDataSize,
                                                      uint8_t* returnData)
{
  if (ring->iteratorValid == false) {
    return EMBER_INVALID_CALL;
  }

  return internalGetEntry(ring,
                          ring->iteratorIndex,
                          dataIndexInEntry,
                          returnEntryTotalSize,
                          maxReturnSize,
                          returnDataSize,
                          returnData);
}

EmberStatus emberAfPluginRingBufferUpdateEntryByIterator(EmberRingBuffer* ring,
                                                         uint16_t dataIndexInEntry,
                                                         uint8_t* updatedData,
                                                         uint16_t updatedDataLength)
{
  uint16_t length;
  uint16_t index;

  if (ring->iteratorValid == false) {
    return EMBER_INVALID_CALL;
  }

  length = getEntryLength(ring, ring->iteratorIndex);
  if (dataIndexInEntry + updatedDataLength > length) {
    return EMBER_MESSAGE_TOO_LONG;
  }

  index = dataIndexInEntry + ring->iteratorIndex + getLengthFieldWidth(ring);

  calculateNewIndexWriteData(ring,
                             index,
                             updatedDataLength,
                             updatedData);

  return EMBER_SUCCESS;
}

EmberStatus emberAfPluginRingBufferAppendLastEntry(EmberRingBuffer* ring,
                                                   uint8_t* dataToAdd,
                                                   uint16_t dataToAddSize)
{
  EmberStatus status;
  uint8_t serializedLengthData[EMBER_AF_PLUGIN_RING_BUFFER_WIDE_LENGTH_ENTRY_OVERHEAD];
  uint16_t newLength;
  uint16_t indexOfLastEntry;
  uint8_t i;

  debug("\n==> %s()\n", __PRETTY_FUNCTION__);

  newLength = getEntryLength(ring, ring->lastItemLengthIndex) + dataToAddSize;
  if (newLength + getLengthFieldWidth(ring) > ring->maxSize) {
    return EMBER_MESSAGE_TOO_LONG;
  }

  status = addOrAppend(ring,
                       dataToAdd,
                       dataToAddSize,
                       RING_BUFFER_APPEND_LAST_ENTRY);
  if (status != EMBER_SUCCESS) {
    debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
    return status;
  }

  indexOfLastEntry = ring->lastItemLengthIndex;

  debug("Updating Length field to %d (index: %d)\n", newLength, indexOfLastEntry);
  serializedLengthData[0] = LOW_BYTE(newLength);
  serializedLengthData[1] = HIGH_BYTE(newLength);

  // We must do this write piecemail in case the length field crosses
  // the end of the buffer.  We don't need to worry about deleting the
  // first element in the ring buffer because we are just updating the existing
  // length field.
  for (i = 0; i < getLengthFieldWidth(ring); i++) {
    indexOfLastEntry = calculateNewIndexWriteData(ring,
                                                  indexOfLastEntry,
                                                  1,
                                                  &serializedLengthData[i]);
  }

  debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
  return status;
}

//------------------------------------------------------------------------------
// Private API

static EmberStatus addOrAppend(EmberRingBuffer* ring,
                               uint8_t* dataToAdd,
                               uint16_t dataToAddSize,
                               RingBufferAction action)
{
  uint16_t lengthDataWritten = 0;
  uint16_t newTailIndex;

  debug("\n==> %s()\n", __PRETTY_FUNCTION__);

  if (dataToAddSize == 0) {
    debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
    return EMBER_INVALID_CALL;
  }

  if (dataToAddSize + getLengthFieldWidth(ring) >= ring->maxSize) {
    debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
    return EMBER_MESSAGE_TOO_LONG;
  }

  if (dataToAddSize >= (2 << (getLengthFieldWidth(ring) * 8))) {
    debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
    return EMBER_MESSAGE_TOO_LONG;
  }

  if (ring->entryCount == 0) {
    if (action == RING_BUFFER_APPEND_LAST_ENTRY) {
      debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
      return EMBER_INVALID_CALL;
    }

    // Simple add case
    ring->data[ring->firstItemIndex]     = LOW_BYTE(dataToAddSize);
    ring->data[ring->firstItemIndex + 1] = HIGH_BYTE(dataToAddSize);
    ring->lastItemLengthIndex = 0;

    MEMCOPY(ring->data + getLengthFieldWidth(ring), dataToAdd, dataToAddSize);

    ring->tailIndex = dataToAddSize + getLengthFieldWidth(ring);
    ring->entryCount++;

    debug("Moved tail index from UNSET to %d\n", ring->tailIndex);
    debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
    return EMBER_SUCCESS;
  }

  // 1) Write data up to the firstItemIndex
  // 2) If more data needs to be written, move the firstItemIndex
  //    to the next item in the list.
  // 3) Goto step 1

  while (dataToAddSize > 0) {
    uint16_t maxWriteLength = calculateMaxWriteLengthWithoutOverlap(ring);
    if (maxWriteLength > dataToAddSize) {
      maxWriteLength = dataToAddSize;
    }

    // There is an edge case where the length field could
    // cross across the end of the ring buffer.
    // So it may take multiple writes to write the length field.

    if (action == RING_BUFFER_ADD_NEW_ENTRY
        && maxWriteLength > 0
        && lengthDataWritten < getLengthFieldWidth(ring)) {
      uint8_t serializedLengthData[EMBER_AF_PLUGIN_RING_BUFFER_MAX_LENGTH_ENTRY_OVERHEAD];
      serializedLengthData[0] = LOW_BYTE(dataToAddSize);
      serializedLengthData[1] = HIGH_BYTE(dataToAddSize);
      if (maxWriteLength > getLengthFieldWidth(ring)) {
        maxWriteLength = getLengthFieldWidth(ring);
      }

      if (lengthDataWritten == 0) {
        ring->lastItemLengthIndex = ring->tailIndex;
      }

      // If we previously wrote a partial length field,
      // we need to limit how much additional data we need to write.
      maxWriteLength -= lengthDataWritten;

      newTailIndex = calculateNewIndexWriteData(ring,
                                                ring->tailIndex,
                                                maxWriteLength,
                                                &(serializedLengthData[lengthDataWritten]));
      debug("Moved tail index from %d to %d\n", ring->tailIndex, newTailIndex);

      lengthDataWritten += maxWriteLength;
      debug("Wrote %d bytes of length data (Value = %d)\n",
            lengthDataWritten,
            dataToAddSize);

      EMBER_TEST_ASSERT(ring->tailIndex != newTailIndex);

      ring->tailIndex = newTailIndex;
    } else if (maxWriteLength > 0) {
      // Length written.  Write the data now.
      debug("Writing data to ring buffer of length %d bytes\n", maxWriteLength);
      newTailIndex = calculateNewIndexWriteData(ring,
                                                ring->tailIndex,
                                                maxWriteLength,
                                                dataToAdd);
      debug("Moved tail index from %d to %d\n", ring->tailIndex, newTailIndex);
      assert(ring->tailIndex != newTailIndex);
      ring->tailIndex = newTailIndex;
      dataToAddSize -= maxWriteLength;
      dataToAdd += maxWriteLength;
    }

    // We have run up against the beginning of the first item AND we still
    // need to write data.  We want to allow for writing up to the first
    // item but NOT deleting the first item if we don't need to yet.
    if (dataToAddSize > 0
        && ring->tailIndex == ring->firstItemIndex) {
      uint16_t newFirstItemIndex = getNextEntryIndex(ring,
                                                     ring->firstItemIndex);
      if (ring->deleteCallback != NULL) {
        ring->deleteCallback(0,    // 0 = entry number
                             getEntryLength(ring, ring->firstItemIndex));
      }
      debug("Deleted first item.  Moved first index from %d to %d\n",
            ring->firstItemIndex,
            newFirstItemIndex);

      if (ring->iteratorIndex == ring->firstItemIndex) {
        debug("Marked iterator index invalid.\n");
        ring->iteratorValid = false;
      }

      ring->firstItemIndex = newFirstItemIndex;
      ring->entryCount--;
      ring->deletedItems++;
    }
  }

  if (action == RING_BUFFER_ADD_NEW_ENTRY) {
    ring->entryCount++;
    ring->addedItems++;
  }

  debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
  return EMBER_SUCCESS;
}

static void copyFromRingBuffer(EmberRingBuffer* ring,
                               uint16_t index,
                               uint16_t length,
                               uint8_t* returnData)
{
  uint16_t copyLength = length;
  EMBER_TEST_ASSERT(index < ring->maxSize);
  if ((index + length) > ring->maxSize) {
    copyLength = ring->maxSize - index;
    MEMCOPY(returnData, &(ring->data[index]), copyLength);
    returnData += copyLength;
    copyLength = length - copyLength;
    index = 0;
  }
  MEMCOPY(returnData, &(ring->data[index]), copyLength);
}

static uint16_t calculateMaxWriteLengthWithoutOverlap(EmberRingBuffer* ring)
{
  int16u maxLength;

  if (ring->firstItemIndex >= ring->tailIndex) {
    maxLength = ring->firstItemIndex - ring->tailIndex;
  } else {
    maxLength = ring->maxSize - ring->tailIndex;
    maxLength += ring->firstItemIndex;
  }

  debug("Max write length is %d\n", maxLength);
  EMBER_TEST_ASSERT(maxLength <= ring->maxSize);
  return maxLength;
}

static uint16_t calculateNewIndexWriteData(EmberRingBuffer* ring,
                                           uint16_t index,
                                           uint16_t length,
                                           uint8_t* newData)
{
  uint16_t truncatedLength = length;
  uint16_t newDataIndex = 0;
  EMBER_TEST_ASSERT(length <= ring->maxSize);

  // This  'if' is not a >= by design. If we have just enough space to write
  // the data we don't want to execute both this 'if' loop
  // with a memcopy, and a second memcopy below.
  if (index + length > ring->maxSize) {
    truncatedLength = ring->maxSize - index;
    debug("Truncated Write - Index: %d, Truncated Length: %d, Total Length: %d\n",
          index,
          truncatedLength,
          length);
    MEMCOPY(&(ring->data[index]), newData, truncatedLength);
    index = 0;
    newDataIndex = truncatedLength;
    truncatedLength = length - truncatedLength;
  }
  MEMCOPY(&(ring->data[index]), &newData[newDataIndex], truncatedLength);
  debug("Write Data - Index: %d, New Data Index: %d, Length: %d\n",
        index,
        newDataIndex,
        truncatedLength);

  // If our write exactly hits the buffer end that is fine but we must
  // recalculate the next index for that edge case.
  if ((index + truncatedLength) == ring->maxSize) {
    return 0;
  }
  return index + truncatedLength;
}

static uint8_t getLengthFieldWidth(EmberRingBuffer* ring)
{
  return ring->lengthFieldWidthBytes;
}

static uint16_t getEntryLength(EmberRingBuffer* ring, uint16_t indexOfEntry)
{
  uint16_t length;
  EMBER_TEST_ASSERT(indexOfEntry < ring->maxSize);
  length = ring->data[indexOfEntry];
  indexOfEntry++;
  if (indexOfEntry >= ring->maxSize) {
    indexOfEntry = 0;
  }
  if (getLengthFieldWidth(ring) > 1) {
    length += (ring->data[indexOfEntry] << 8);
  }
  return length;
}

// Get index of the entry, accounting for wrap back to the beginning.
static uint16_t getNextEntryIndex(EmberRingBuffer* ring,
                                  uint16_t indexOfEntry)
{
  uint16_t length;
  EMBER_TEST_ASSERT(indexOfEntry < ring->maxSize);

  length = getEntryLength(ring, indexOfEntry);
  indexOfEntry += getLengthFieldWidth(ring);
  indexOfEntry += length;

  if (indexOfEntry >= ring->maxSize) {
    indexOfEntry -= ring->maxSize;
  }

  return indexOfEntry;
}

static uint16_t getIndexOfDataForEntry(EmberRingBuffer* ring,
                                       uint16_t indexOfEntry)
{
  EMBER_TEST_ASSERT(indexOfEntry < ring->maxSize);

  indexOfEntry += getLengthFieldWidth(ring);
  if (indexOfEntry >= ring->maxSize) {
    indexOfEntry -= ring->maxSize;
  }
  return indexOfEntry;
}

static EmberStatus internalGetEntry(EmberRingBuffer* ring,
                                    uint16_t index,
                                    uint16_t dataIndexInEntry,
                                    uint16_t* returnEntryTotalSize,
                                    uint16_t maxReturnSize,
                                    uint16_t* returnDataSize,
                                    uint8_t* returnData)
{
  debug("\n==> %s\n", __PRETTY_FUNCTION__);

  *returnDataSize = getEntryLength(ring, index);

  if (returnData == NULL || maxReturnSize == 0) {
    return EMBER_SUCCESS;
  }

  // Record the total entry size so that the caller knows this value
  *returnEntryTotalSize = *returnDataSize;

  // Adjust the return data size by taking into account the index into the entry
  *returnDataSize = *returnEntryTotalSize - dataIndexInEntry;

  if (maxReturnSize < *returnDataSize) {
    *returnDataSize = maxReturnSize;
  }

  index = getIndexOfDataForEntry(ring, index) + dataIndexInEntry;
  if (index >= ring->maxSize) {
    index -= ring->maxSize;
  }
  if (dataIndexInEntry > *returnEntryTotalSize) {
    return EMBER_INDEX_OUT_OF_RANGE;
  } else if (dataIndexInEntry == *returnEntryTotalSize) {
    // No data to copy.
    return EMBER_SUCCESS;
  }

  copyFromRingBuffer(ring,
                     index,
                     *returnDataSize,
                     returnData);

  debug("\n<== %s returned\n", __PRETTY_FUNCTION__);
  return EMBER_SUCCESS;
}
