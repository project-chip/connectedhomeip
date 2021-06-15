/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      The defines constants for the CHIP Interaction Model Protocol, present in
 *      every CHIP device.
 *
 */

#pragma once

#include <protocols/Protocols.h>

/**
 *   @namespace chip::Protocols::InteractionModel
 *
 *   @brief
 *     This namespace includes all interfaces within CHIP for the
 *     CHIP InteractionModel protocol.
 *
 *     The interfaces define message types.
 */

namespace chip {
namespace Protocols {
namespace InteractionModel {

/**
 * Version of the Interaction Model used by the node.
 */
constexpr uint16_t kVersion = 0;

/**
 * SecureChannel Protocol Message Types
 */
enum class MsgType : uint8_t
{
    ReadRequest           = 0x02,
    SubscribeRequest      = 0x03,
    SubscribeResponse     = 0x04,
    ReportData            = 0x05,
    WriteRequest          = 0x06,
    WriteResponse         = 0x07,
    InvokeCommandRequest  = 0x08,
    InvokeCommandResponse = 0x09,
    TimedRequest          = 0x0a,
};

// This table comes from the IM's "Status Code Table" section from the Interaction Model spec.
enum class ProtocolCode : uint16_t
{
    Success                = 0x0,
    Failure                = 0x01,
    InvalidSubscription    = 0x7d,
    UnsupportedAccess      = 0x7e,
    UnsupportedEndpoint    = 0x7f,
    InvalidAction          = 0x80,
    UnsupportedCommand     = 0x81,
    Reserved82             = 0x82,
    Reserved83             = 0x83,
    Reserved84             = 0x84,
    InvalidCommand         = 0x85,
    UnsupportedAttribute   = 0x86,
    InvalidValue           = 0x87,
    UnsupportedWrite       = 0x88,
    ResourceExhausted      = 0x89,
    Reserved8a             = 0x8a,
    NotFound               = 0x8b,
    UnreportableAttribute  = 0x8c,
    InvalidDataType        = 0x8d,
    Reserved8e             = 0x8e,
    UnsupportedRead        = 0x8f,
    Reserved90             = 0x90,
    Reserved91             = 0x91,
    Reserved92             = 0x92,
    Reserved93             = 0x93,
    Timeout                = 0x94,
    Reserved95             = 0x95,
    Reserved96             = 0x96,
    Reserved97             = 0x97,
    Reserved98             = 0x98,
    Reserved99             = 0x99,
    Reserved9a             = 0x9a,
    ConstraintError        = 0x9b,
    Busy                   = 0x9c,
    Reservedc0             = 0xc0,
    Reservedc1             = 0xc1,
    Reservedc2             = 0xc2,
    UnsupportedCluster     = 0xc3,
    Reservedc4             = 0xc4,
    NoUpstreamSubscription = 0xc5,
    InvalidArgument        = 0xc6,
};

inline uint16_t ToUint16(ProtocolCode aProtocolCode)
{
    static_assert(std::is_same<uint16_t, std::underlying_type_t<ProtocolCode>>::value, "Cast might not be right");
    return static_cast<uint16_t>(aProtocolCode);
}
} // namespace InteractionModel

template <>
struct MessageTypeTraits<InteractionModel::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return InteractionModel::Id; }
};

} // namespace Protocols
} // namespace chip
