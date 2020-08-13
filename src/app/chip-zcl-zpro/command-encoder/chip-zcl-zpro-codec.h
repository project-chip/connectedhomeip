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

#ifndef CHIP_ZCL_ZPRO_CODEC_H
#define CHIP_ZCL_ZPRO_CODEC_H

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t EmberApsOption;

/** @brief An in-memory representation of a ZigBee APS frame
 * of an incoming or outgoing message. Copy pasted here so that we can compile this unit of code independently.
 */
typedef struct
{
    /** The application profile ID that describes the format of the message. */
    uint16_t profileId;
    /** The cluster ID for this message. */
    uint16_t clusterId;
    /** The source endpoint. */
    uint8_t sourceEndpoint;
    /** The destination endpoint. */
    uint8_t destinationEndpoint;
    /** A bitmask of options from the enumeration above. */
    EmberApsOption options;
    /** The group ID for this message, if it is multicast mode. */
    uint16_t groupId;
    /** The sequence number. */
    uint8_t sequence;
    uint8_t radius;
} EmberApsFrame;

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Encode an on command for on-off server into buffer including the APS frame
 */
uint32_t encodeOnCommand(uint8_t * buffer, uint32_t buf_length, uint8_t destination_endpoint);

/** @brief Encode an off command for on-off server into buffer including the APS frame
 */

uint32_t encodeOffCommand(uint8_t * buffer, uint32_t buf_length, uint8_t destination_endpoint);

/** @brief Encode a toggle command for on-off server into buffer including the APS frame
 */

uint32_t encodeToggleCommand(uint8_t * buffer, uint32_t buf_length, uint8_t destination_endpoint);

/**
 * @brief Encode a Read Attributes command for the given cluster and the given
 * list of attributes.
 */
uint16_t encodeReadAttributesCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t cluster_id,
                                     uint16_t * attr_ids, uint16_t attr_id_count);

/**
 * @brief Encode a command to read the OnOff attribute from the on/off
 * cluster.
 */
uint16_t encodeReadOnOffCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

/** @brief Extracts an aps frame from buffer into outApsFrame
 * @param buffer Buffer to read from
 * @param buf_length Length of buffer
 * @param outApsFrame Pointer to EmberApsFrame struct to read into
 * @return returns the number of bytes that were consumed to read out the EmberApsFrame. 0 means an error was encountered
 */
uint16_t extractApsFrame(uint8_t * buffer, uint32_t buf_length, EmberApsFrame * outApsFrame);

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
 *   - If buffer is null, the number of bytes needed to encode.
 *   - If buffer is non-null but buf_length is not enough to hold the
 *     EmberApsFrame, 0.
 *   - If buffer us non-null and buf_length is large enough, the number of bytes
 *     placed in buffer.
 */
uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame);

#ifdef __cplusplus
}
#endif

#endif // CHIP_ZCL_ZPRO_CODEC_H
