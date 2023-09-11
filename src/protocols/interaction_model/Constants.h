/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      The defines constants for the CHIP Interaction Model Protocol, present in
 *      every CHIP device.
 *
 */

#pragma once

#include <type_traits>

#include <array>
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

constexpr const char * kProtocolName = "IM";

/**
 * Version of the Interaction Model used by the node.
 */
constexpr uint16_t kVersion = 0;

/**
 * Interaction Model Protocol Message Types
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

    static auto GetTypeToNameTable()
    {
        static const std::array<MessageTypeNameLookup, 10> typeToNameTable = {
            {
                { InteractionModel::MsgType::StatusResponse, "StatusResponse" },
                { InteractionModel::MsgType::ReadRequest, "ReadRequest" },
                { InteractionModel::MsgType::SubscribeRequest, "SubscribeRequest" },
                { InteractionModel::MsgType::SubscribeResponse, "SubscribeResponse" },
                { InteractionModel::MsgType::ReportData, "ReportData" },
                { InteractionModel::MsgType::WriteRequest, "WriteRequest" },
                { InteractionModel::MsgType::WriteResponse, "WriteResponse" },
                { InteractionModel::MsgType::InvokeCommandRequest, "InvokeCommandRequest" },
                { InteractionModel::MsgType::InvokeCommandResponse, "InvokeCommandResponse" },
                { InteractionModel::MsgType::TimedRequest, "TimedRequest" },
            },
        };

        return &typeToNameTable;
    }
};

} // namespace Protocols
} // namespace chip
