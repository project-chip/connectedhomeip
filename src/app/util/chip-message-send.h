/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, softwarEchoe
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *   This file declares a function that can be called to send a unicast message
 *   (in the form of a buffer and length) to a given response destination node
 *   id.
 */

#pragma once

#include "types_stub.h" // For EmberApsFrame, EmberStatus, node ids.

/**
 * @brief
 *    Called to send a unicast message.  The message is constructed
 *    by serializing the given APS frame followed by the actual message
 *    buffer passed in.
 *
 * @param[in] destination The destination node id to send the message to.
 * @param[in] apsFrame The APS frame to use for the message.
 * @param[in] messageLength The length of the message to send after the APS
 *                          frame.
 * @param[in] message The message to send after the APS frame.
 */
EmberStatus chipSendUnicast(chip::NodeId destination, EmberApsFrame * apsFrame, uint16_t messageLength, uint8_t * message);
