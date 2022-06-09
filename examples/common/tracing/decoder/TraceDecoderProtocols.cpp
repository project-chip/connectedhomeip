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

#include "TraceDecoderProtocols.h"

#include "bdx/Decoder.h"
#include "echo/Decoder.h"
#include "interaction_model/Decoder.h"
#include "secure_channel/Decoder.h"
#include "udc/Decoder.h"

#include <lib/core/CHIPTLVDebug.hpp>
#include <lib/support/BytesToHex.h>
#include <protocols/Protocols.h>

namespace {

constexpr const char * kUnknown = "Unknown";

void ENFORCE_FORMAT(1, 2) TLVPrettyPrinter(const char * aFormat, ...)
{
    va_list args;
    va_start(args, aFormat);
    vprintf(aFormat, args);
    va_end(args);
}
} // namespace

namespace chip {
namespace trace {

const char * ToProtocolName(uint16_t protocolId)
{
    const char * name = nullptr;

    auto protocol = Protocols::Id(VendorId::Common, protocolId);
    if (protocol == Protocols::SecureChannel::Id)
    {
        name = secure_channel::ToProtocolName();
    }
    else if (protocol == Protocols::InteractionModel::Id)
    {
        name = interaction_model::ToProtocolName();
    }
    else if (protocol == Protocols::BDX::Id)
    {
        name = bdx::ToProtocolName();
    }
    else if (protocol == Protocols::UserDirectedCommissioning::Id)
    {
        name = udc::ToProtocolName();
    }
    else if (protocol == Protocols::Echo::Id)
    {
        name = echo::ToProtocolName();
    }
    else
    {
        name = kUnknown;
    }

    return name;
}

const char * ToProtocolMessageTypeName(uint16_t protocolId, uint8_t protocolCode)
{
    const char * name = nullptr;

    auto protocol = Protocols::Id(VendorId::Common, protocolId);
    if (protocol == Protocols::SecureChannel::Id)
    {
        name = secure_channel::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::InteractionModel::Id)
    {
        name = interaction_model::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::BDX::Id)
    {
        name = bdx::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::UserDirectedCommissioning::Id)
    {
        name = udc::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::Echo::Id)
    {
        name = echo::ToProtocolMessageTypeName(protocolCode);
    }
    else
    {
        name = kUnknown;
    }

    return name;
}

CHIP_ERROR LogAsProtocolMessage(uint16_t protocolId, uint8_t protocolCode, const char * payload, size_t len,
                                bool interactionModelResponse)
{
    constexpr uint16_t kMaxPayloadLen = 2048;
    uint8_t data[kMaxPayloadLen]      = {};
    size_t dataLen                    = Encoding::HexToBytes(payload, len, data, sizeof(data));
    VerifyOrReturnError(dataLen != 0, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = CHIP_NO_ERROR;

    auto protocol = Protocols::Id(VendorId::Common, protocolId);
    if (protocol == Protocols::SecureChannel::Id)
    {
        err = secure_channel::LogAsProtocolMessage(protocolCode, data, dataLen);
    }
    else if (protocol == Protocols::InteractionModel::Id)
    {
        err = interaction_model::LogAsProtocolMessage(protocolCode, data, dataLen, interactionModelResponse);
    }
    else if (protocol == Protocols::BDX::Id)
    {
        err = bdx::LogAsProtocolMessage(protocolCode, data, dataLen);
    }
    else if (protocol == Protocols::UserDirectedCommissioning::Id)
    {
        err = udc::LogAsProtocolMessage(protocolCode, data, dataLen);
    }
    else if (protocol == Protocols::Echo::Id)
    {
        err = echo::LogAsProtocolMessage(protocolCode, data, dataLen);
    }
    else
    {
        err = CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(DataManagement, "Error: %s", chip::ErrorStr(err));
        TLV::TLVReader reader;
        reader.Init(data, dataLen);
        ReturnErrorOnFailure(reader.Next());
        err = TLV::Debug::Dump(reader, TLVPrettyPrinter);
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
    }

    return err;
}

} // namespace trace
} // namespace chip
