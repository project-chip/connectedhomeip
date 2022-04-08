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

#include <type_traits>

#include <protocols/Protocols.h>
#include <protocols/interaction_model/StatusCode.h>

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
    StatusResponse        = 0x01,
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

} // namespace InteractionModel

template <>
struct MessageTypeTraits<InteractionModel::MsgType>
{
    static constexpr const Protocols::Id & ProtocolId() { return InteractionModel::Id; }
};

} // namespace Protocols
} // namespace chip
