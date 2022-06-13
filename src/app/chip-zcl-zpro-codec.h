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
#include <lib/core/GroupId.h>
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
