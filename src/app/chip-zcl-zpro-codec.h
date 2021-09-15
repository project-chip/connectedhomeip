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

#pragma once

#include <app/util/basic-types.h>
#include <lib/support/BufferWriter.h>

typedef uint16_t EmberApsOption;

/** @brief An in-memory representation of a ZigBee APS frame
 * of an incoming or outgoing message. Copy pasted here so that we can compile this unit of code independently.
 */
typedef struct
{
    /** The cluster ID for this message. */
    chip::ClusterId clusterId;
    /** The source endpoint. */
    chip::EndpointId sourceEndpoint;
    /** The destination endpoint. */
    chip::EndpointId destinationEndpoint;
    /** A bitmask of options from the enumeration above. */
    EmberApsOption options;
    /** The group ID for this message, if it is multicast mode. */
    chip::GroupId groupId;
    /** The sequence number. */
    uint8_t sequence;
    uint8_t radius;
} EmberApsFrame;

/** @brief Extracts an aps frame from buffer into outApsFrame
 * @param buffer Buffer to read from
 * @param buf_length Length of buffer
 * @param outApsFrame Pointer to EmberApsFrame struct to read into
 * @return returns the number of bytes that were consumed to read out the EmberApsFrame. 0 means an error was encountered
 */
uint16_t extractApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * outApsFrame);

/** @brief Populates msg with address of the zcl message within buffer.
 * @return Returns the length of msg buffer. Returns 0 on error e.g. if buffer is too short.
 */
uint16_t extractMessage(uint8_t * buffer, uint16_t buffer_length, uint8_t ** msg);

/** @brief Prints an aps frame struct
 */
void printApsFrame(EmberApsFrame * frame);

/**
 * @brief Encode an APS frame into the given buffer.  Returns the number of
 * bytes of buffer used by the encoding or 0 if the given buffer is not big
 * enough.  If buffer is null, no encoding will happen; the function will
 * instead return the number of bytes that would be needed to encode the APS
 * frame.
 *
 * @param[in] buffer The buffer to write to.  If null, the call is in "count the
 *                   bytes" mode, and no writing will happen.
 * @parem[in] buf_length The size of the buffer.  Ignored if buffer is null.
 * @param[in] apsFrame The frame to encode.
 *
 * @return
 *   - If buffer is null, the number of bytes needed to encode.  If this number
 *     does not fit in a uint16_t, 0 will be returned.
 *   - If buffer is non-null but buf_length is not enough to hold the
 *     EmberApsFrame, 0.
 *   - If buffer is non-null and buf_length is large enough, the number of bytes
 *     placed in buffer.
 */
uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame);

/**
 * @brief Encode the given informations into the given BufferWriter.
 * Returns the number of bytes of buffer used by the encoding or 0 if
 * the given buffer is not big enough.
 *
 * @return
 *   - If the buffer length is not enough to hold the EmberApsFrame, 0.
 *   - If the buffer length is large enough, the number of bytes placed in buffer.
 */
uint16_t doEncodeApsFrame(chip::Encoding::LittleEndian::BufferWriter & buf, chip::ClusterId clusterId,
                          chip::EndpointId sourceEndpoint, chip::EndpointId destinationEndpoint, EmberApsOption options,
                          chip::GroupId groupId, uint8_t sequence, uint8_t radius, bool isMeasuring);
