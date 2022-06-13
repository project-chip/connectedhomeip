/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/***************************************************************************/
/**
 * @file
 * @brief API for generating command buffer.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <app/util/af-types.h>

/*
   @addtogroup command
   @{
 */

/** @name Client API functions */
// @{

/**
 * @brief Function that fills in the buffer with command.
 *
 * Fills the outgoing ZCL buffer and returns the number
 * of bytes used in a buffer. Uses the buffers that
 * were previously set with emberAfSetExternalBuffer.
 *
 * @param frameControl Byte used for frame control.
 * @param clusterId Cluster ID of message.
 * @param commandId Command ID of message.
 * @param format String format for further arguments to the function.
 *   Format values are:
 *     - '0' - '9' and 'A' - 'G': Pointer to a buffer containing 0--16 raw
 *            bytes.  The data is copied as is to the destination buffer.
 *     - 'u': uint8_t.
 *     - 'v': uint16_t. The bytes are copied in little-endian format to the
 *            destination buffer.
 *     - 'x': int24u. The bytes are copied in little-endian format to the
 *            destination buffer.
 *     - 'w': uint32_t. The bytes are copied in little-endian format to the
 *            destination buffer.
 *     - 'l': Pointer to a buffer containing a ZigBee long string, with the
 *            first two bytes of the buffer specifying the length of the string
 *            data in little-endian format. The length bytes and the string
 *            itself are both included as is in the destination buffer.
 *     - 's': Pointer to a buffer containing a ZigBee string, with the first
 *            byte of the buffer specifying the length of the string data. The
 *            length byte and the string itself are both included as is in the
 *            destination buffer.
 *     - 'L': Pointer to a buffer containing a long string followed by an
 *            uint16_t specifying the length of the string data. The length
 *            bytes in little-endian format will precede the string itself in
 *            the destination buffer.
 *     - 'S': Pointer to a buffer containing a string followed by an uint8_t
 *            specifying the length of the string data. The length byte will
 *            precede the string itself in the destination buffer.
 *     - 'b': Pointer to a buffer containing a sequence of raw bytes followed
 *            by an uint16_t specifying the length of the data. The data is
 *            copied as is to the destination buffer. The length is not
 *            included.
 */
uint16_t emberAfFillExternalBuffer(uint8_t frameControl, chip::ClusterId clusterId, chip::CommandId commandId, const char * format,
                                   ...);

/**
 * @brief Function that registers the buffer to use with the fill function.
 * Registers the buffer for use with the emberAfFillExternalBuffer function.
 *
 * @param buffer Main buffer for constructing message.
 * @param bufferLen Available length of buffer.
 * @param responseLengthPtr Location where length of message will be written into.
 * @param apsFramePtr Location where APS frame data will be written.
 */
void emberAfSetExternalBuffer(uint8_t * buffer, uint16_t bufferLen, uint16_t * responseLengthPtr, EmberApsFrame * apsFramePtr);

#if !defined(DOXYGEN_SHOULD_SKIP_THIS)
// The buffer used for filling ZCL Messages.
extern uint8_t * emAfZclBuffer;
// Max length of the buffer.
extern uint16_t emAfZclBufferLen;
// Pointer to where this API should put the length.
extern uint16_t * emAfResponseLengthPtr;
// The APS frame accompanying the ZCL message.
extern EmberApsFrame * emAfCommandApsFrame;
#endif

// Generated macros.

#define emberAfAppendToExternalBuffer(...) emberAfPutBlockInResp(__VA_ARGS__)

/** @} END Client API functions */

/** @} END addtogroup
 */
