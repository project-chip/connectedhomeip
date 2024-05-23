/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "Decoder.h"
#include "DecoderCustomLog.h"

#include <app/MessageDef/InvokeRequestMessage.h>
#include <app/MessageDef/InvokeResponseMessage.h>
#include <app/MessageDef/ReadRequestMessage.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/MessageDef/StatusResponseMessage.h>
#include <app/MessageDef/SubscribeRequestMessage.h>
#include <app/MessageDef/SubscribeResponseMessage.h>
#include <app/MessageDef/TimedRequestMessage.h>
#include <app/MessageDef/WriteRequestMessage.h>
#include <app/MessageDef/WriteResponseMessage.h>

namespace {
constexpr char kProtocolName[] = "Interaction Model";

constexpr char kUnknown[]               = "Unknown";
constexpr char kStatusResponse[]        = "Status Response";
constexpr char kReadRequest[]           = "Read Request";
constexpr char kSubscribeRequest[]      = "Subscribe Request";
constexpr char kSubscribeResponse[]     = "Subscribe Response";
constexpr char kReportData[]            = "Report Data";
constexpr char kWriteRequest[]          = "Write Request";
constexpr char kWriteResponse[]         = "Write Response";
constexpr char kInvokeCommandRequest[]  = "InvokeCommandRequest";
constexpr char kInvokeCommandResponse[] = "InvokeCommandResponse";
constexpr char kTimedRequest[]          = "Timed Request";

} // namespace

using MessageType = chip::Protocols::InteractionModel::MsgType;

namespace chip {
namespace trace {
namespace interaction_model {

CHIP_ERROR DecodeStatusResponse(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeReadRequest(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeSubscribeRequest(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeSubscribeResponse(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeReportData(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeWriteRequest(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeWriteResponse(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeInvokeCommandRequest(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeInvokeCommandResponse(TLV::TLVReader & reader, bool decode);
CHIP_ERROR DecodeTimedRequest(TLV::TLVReader & reader, bool decode);

const char * ToProtocolName()
{
    return kProtocolName;
}

const char * ToProtocolMessageTypeName(uint8_t protocolCode)
{
    switch (protocolCode)
    {
    case to_underlying(MessageType::StatusResponse):
        return kStatusResponse;
    case to_underlying(MessageType::ReadRequest):
        return kReadRequest;
    case to_underlying(MessageType::SubscribeRequest):
        return kSubscribeRequest;
    case to_underlying(MessageType::SubscribeResponse):
        return kSubscribeResponse;
    case to_underlying(MessageType::ReportData):
        return kReportData;
    case to_underlying(MessageType::WriteRequest):
        return kWriteRequest;
    case to_underlying(MessageType::WriteResponse):
        return kWriteResponse;
    case to_underlying(MessageType::InvokeCommandRequest):
        return kInvokeCommandRequest;
    case to_underlying(MessageType::InvokeCommandResponse):
        return kInvokeCommandResponse;
    case to_underlying(MessageType::TimedRequest):
        return kTimedRequest;
    default:
        return kUnknown;
    }
}

CHIP_ERROR LogAsProtocolMessage(uint8_t protocolCode, const uint8_t * data, size_t len, bool decodeResponse)
{
    TLV::TLVReader reader;
    reader.Init(data, len);

    switch (protocolCode)
    {
    case to_underlying(MessageType::StatusResponse):
        return DecodeStatusResponse(reader, decodeResponse);
    case to_underlying(MessageType::ReadRequest):
        return DecodeReadRequest(reader, decodeResponse);
    case to_underlying(MessageType::SubscribeRequest):
        return DecodeSubscribeRequest(reader, decodeResponse);
    case to_underlying(MessageType::SubscribeResponse):
        return DecodeSubscribeResponse(reader, decodeResponse);
    case to_underlying(MessageType::ReportData):
        return DecodeReportData(reader, decodeResponse);
    case to_underlying(MessageType::WriteRequest):
        return DecodeWriteRequest(reader, decodeResponse);
    case to_underlying(MessageType::WriteResponse):
        return DecodeWriteResponse(reader, decodeResponse);
    case to_underlying(MessageType::InvokeCommandRequest):
        return DecodeInvokeCommandRequest(reader, decodeResponse);
    case to_underlying(MessageType::InvokeCommandResponse):
        return DecodeInvokeCommandResponse(reader, decodeResponse);
    case to_underlying(MessageType::TimedRequest):
        return DecodeTimedRequest(reader, decodeResponse);
    default:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
}

CHIP_ERROR DecodeStatusResponse(TLV::TLVReader & reader, bool decode)
{
#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::StatusResponseMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeReadRequest(TLV::TLVReader & reader, bool decode)
{
#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::ReadRequestMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        return parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeSubscribeRequest(TLV::TLVReader & reader, bool decode)
{
#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::SubscribeRequestMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        return parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeSubscribeResponse(TLV::TLVReader & reader, bool decode)
{
#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::SubscribeResponseMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeReportData(TLV::TLVReader & reader, bool decode)
{
    ReturnErrorOnFailure(MaybeDecodeNestedReadResponse(reader.GetReadPoint(), reader.GetTotalLength()));

#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::ReportDataMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeWriteRequest(TLV::TLVReader & reader, bool decode)
{
#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::WriteRequestMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        return parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeWriteResponse(TLV::TLVReader & reader, bool decode)
{
#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::WriteResponseMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        return parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeInvokeCommandRequest(TLV::TLVReader & reader, bool decode)
{
    ReturnErrorOnFailure(MaybeDecodeNestedCommandRequest(reader.GetReadPoint(), reader.GetTotalLength()));

#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::InvokeRequestMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        return parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeInvokeCommandResponse(TLV::TLVReader & reader, bool decode)
{
    ReturnErrorOnFailure(MaybeDecodeNestedCommandResponse(reader.GetReadPoint(), reader.GetTotalLength()));

#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::InvokeResponseMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeTimedRequest(TLV::TLVReader & reader, bool decode)
{
#if CHIP_CONFIG_IM_PRETTY_PRINT
    if (decode)
    {
        app::TimedRequestMessage::Parser parser;
        ReturnErrorOnFailure(parser.Init(reader));
        parser.PrettyPrint();
    }
#endif

    return CHIP_NO_ERROR;
}

} // namespace interaction_model
} // namespace trace
} // namespace chip
