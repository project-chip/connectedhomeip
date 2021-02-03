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
 * SecureChannel Protocol Message Types
 */
enum class MsgType : uint8_t
{
    SubscribeRequest      = 0x01,
    ReadRequest           = 0x02,
    ReportData            = 0x03,
    WriteRequest          = 0x04,
    WriteResponse         = 0x05,
    InvokeCommandRequest  = 0x06,
    InvokeCommandResponse = 0x07,
};

} // namespace InteractionModel

template <>
struct MessageTypeTraits<InteractionModel::MsgType>
{
    static constexpr uint16_t ProtocolId = chip::Protocols::kProtocol_InteractionModel;
};

} // namespace Protocols
} // namespace chip
