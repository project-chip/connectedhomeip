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
 *   This file declares the ChipResponseDestination struct and a function that
 *   can be called to send a response (in the form of a buffer and length)
 *   to a given response destination.
 *
 *   This file is designed to be included from both C and C++ code.  When
 *   included from C it provides only an opaque declaration of the destination
 *   struct, because the types in the struct can only be used in C++.
 */

#ifndef CHIP_RESPONSE_H
#define CHIP_RESPONSE_H

#include "types_stub.h" // For EmberApsFrame and EmberStatus

#ifdef __cplusplus

#include <transport/MessageHeader.h>    // For NodeId
#include <transport/SecureSessionMgr.h> // For SecureSessionMgrBase

/**
 * @brief
 *   This struct encapsulates the information needed to send a message in
 *   response to an incoming message: the source node id and the session the
 *   incoming message came in on, for now.  This struct is intended to have
 *   stack lifetime (to ensure that the SecureSessionMgrBase pointer remains
 *   valid).  Consumers must not store references to this struct in a way that
 *   would violate stack discipline.
 */
struct ChipResponseDestination
{
    ChipResponseDestination(chip::NodeId sourceNodeId, chip::SecureSessionMgrBase * secureSessionManager) :
        mSourceNodeId(sourceNodeId), mSecureSessionManager(secureSessionManager)
    {}
    chip::NodeId mSourceNodeId;
    chip::SecureSessionMgrBase * mSecureSessionManager;
};

#else // __cplusplus

typedef struct ChipResponseDestination ChipResponseDestination;

#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
/**
 * @brief
 *    Called to send a response to a message.  The message is constructed
 *    by serializing the given APS frame followed by the actual message
 *    buffer passed in.
 *
 * @param[in] destination The destination to send the response to.
 * @param[in] apsFrame The APS frame to use for the response.
 * @param[in] messageLength The length of the message to send after the APS
 *                          frame.
 * @param[in] message The message to send after the APS frame.
 */
EmberStatus chipSendResponse(ChipResponseDestination * destination, EmberApsFrame * apsFrame, uint16_t messageLength,
                             uint8_t * message);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // CHIP_RESPONSE_H
