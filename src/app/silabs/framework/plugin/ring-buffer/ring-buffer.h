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
 * @brief Definitions for the Ring Buffer plugin.
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_PLUGIN_RING_BUFFER_WIDE_LENGTH_ENTRY_OVERHEAD 2
#define EMBER_AF_PLUGIN_RING_BUFFER_NARROW_LENGTH_ENTRY_OVERHEAD 1

#define EMBER_AF_PLUGIN_RING_BUFFER_MAX_LENGTH_ENTRY_OVERHEAD \
  EMBER_AF_PLUGIN_RING_BUFFER_WIDE_LENGTH_ENTRY_OVERHEAD

typedef void (EmberAfPluginRingBufferDeleteFunction)(uint16_t entryNumber,
                                                     uint16_t entrySize);

typedef struct {
  const uint16_t maxSize;

  // Private variables, don't modify outside ring-buffer.c
  // (I wish we had C++)
  uint8_t* data;
  uint16_t firstItemIndex;
  uint16_t tailIndex;
  uint16_t entryCount;
  EmberAfPluginRingBufferDeleteFunction* deleteCallback;
  uint32_t addedItems;
  uint32_t deletedItems;
  uint16_t lastItemLengthIndex; // used for append
  uint16_t iteratorIndex;
  bool iteratorValid;
  uint8_t lengthFieldWidthBytes;
} EmberRingBuffer;

// Intializes the ring buffer struct based on the data passed.
// widLengthField determines whether the ring buffer uses 1 or 2 byte length fields
// for entry.  wideLengthField = false uses 1 byte length, wideLengthField = true
// uses 2 byte lengths.
// All entries in the same ring buffer utilze the same size length field.
// Different ring buffers may use differnt sizes.
EmberStatus emberAfPluginRingBufferInitStruct(EmberRingBuffer* ringBufferStruct,
                                              bool wideLengthField,
                                              EmberAfPluginRingBufferDeleteFunction* deleteCallback,
                                              uint8_t* dataPtr);

EmberStatus emberAfPluginRingBufferAddEntry(EmberRingBuffer* ring,
                                            uint8_t* dataToAdd,
                                            uint16_t dataToAddSize);

// Retrieves data from the specified entry.  Returns the entry's total
// size in value pointed to by returnEntryTotalSize.  The amount of data
// returned is written to the pointer returnDataSize.  This allows
// for the caller to get the entry's total size at the same time they
// are able to grab a chunk of the entry up to some limit.
// - entryNumber numbering starts from 0
// - returnData may be set to NULL, in which case no return data is written.
// - returnEntryTotalSize does not include the entry's length overhead,
//   which the caller shouldn't care about anyway.
EmberStatus emberAfPluginRingBufferGetEntryByEntryNumber(EmberRingBuffer* ring,
                                                         uint16_t  entryNumber,
                                                         uint16_t  dataIndexInEntry,
                                                         uint16_t* returnEntryTotalSize,
                                                         uint16_t  maxReturnSize,
                                                         uint16_t* returnDataSize,
                                                         uint8_t*  returnData);

EmberStatus emberAfPluginRingBufferGetLastEntry(EmberRingBuffer* ring,
                                                uint16_t dataIndexInEntry,
                                                uint16_t* returnEntryTotalSize,
                                                uint16_t maxReturnSize,
                                                uint16_t* returnDataSize,
                                                uint8_t* returnData);

// Initializes the iterator to the first entry in the ring buffer.
// You may immediately call emberAfPluginRingBufferGetEntryByIterator()
// after this function is called.
EmberStatus emberAfPluginRingBufferInitIterator(EmberRingBuffer* ring);

// Returns EMBER_OPERATION_IN_PROGRESS if there are more items left to
// iterate through.  Returns EMBER_SUCCESS on reaching the last item,
// and the iterator is marked invalid (needs to be initialized again).
// Returns EMBER_INVALID_CALL if the iterator has not been initialized.
// An iterator can also be marked invalid if the
// ring buffer has deleted the entry that the iterator was currently on.
EmberStatus emberAfPluginRingBufferIteratorNextEntry(EmberRingBuffer* ring);

// Similar to emberAfPluginRingBufferGetEntryByEntryNumber().
// The iterator must have already been initialized
EmberStatus emberAfPluginRingBufferGetEntryByIterator(EmberRingBuffer* ring,
                                                      uint16_t dataIndexInEntry,
                                                      uint16_t* returnEntryTotalSize,
                                                      uint16_t maxReturnSize,
                                                      uint16_t* returnDataSize,
                                                      uint8_t* returnData);

EmberStatus emberAfPluginRingBufferUpdateEntryByIterator(EmberRingBuffer* ring,
                                                         uint16_t dataIndexInEntry,
                                                         uint8_t* updatedData,
                                                         uint16_t updatedDataLength);

EmberStatus emberAfPluginRingBufferAppendLastEntry(EmberRingBuffer* ring,
                                                   uint8_t* dataToAdd,
                                                   uint16_t dataToAddSize);
