/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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
 *    @file
 *      The defines constants for the CHIP Secure Channel Protocol, present in
 *      every CHIP device.
 *
 */

#pragma once

#include <protocols/Protocols.h>

/**
 *   @namespace chip::Protocols::SecureChannel
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP SecureChannel protocol.
 *
 *     The interfaces define message types and status codes.
 */

namespace chip {
namespace Protocols {
namespace SecureChannel {

/**
 * SecureChannel Protocol Message Types
 */
enum class MsgType : uint8_t
{
    // Message Counter Synchronization Protocol Message Types
    MsgCounterSyncReq = 0x00,
    MsgCounterSyncRsp = 0x01,

    // Reliable Messaging Protocol Message Types
    StandaloneAck = 0x10,

    // Password-based session establishment Message Types
    PBKDFParamRequest  = 0x20,
    PBKDFParamResponse = 0x21,
    PASE_Spake2p1      = 0x22,
    PASE_Spake2p2      = 0x23,
    PASE_Spake2p3      = 0x24,
    PASE_Spake2pError  = 0x2F,

    // Certificate-based session establishment Message Types
    CASE_SigmaR1  = 0x30,
    CASE_SigmaR2  = 0x31,
    CASE_SigmaR3  = 0x32,
    CASE_SigmaErr = 0x3F,
};

/**
 * SecureChannel Protocol Status Codes
 */
enum class StatusCode
{
    AlreadyMemberOfFabric = 1, /**< The recipient is already a member of a fabric. */
    NotMemberOfFabric     = 2, /**< The recipient is not a member of a fabric. */
    InvalidFabricConfig   = 3  /**< The specified fabric configuration was invalid. */
};

} // namespace SecureChannel

template <>
struct MessageTypeTraits<SecureChannel::MsgType>
{
    static constexpr uint16_t ProtocolId = chip::Protocols::kProtocol_SecureChannel;
};

} // namespace Protocols
} // namespace chip
