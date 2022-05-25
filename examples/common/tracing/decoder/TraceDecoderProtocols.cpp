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

#include "TraceDecoderProtocolBDX.h"
#include "TraceDecoderProtocolEcho.h"
#include "TraceDecoderProtocolInteractionModel.h"
#include "TraceDecoderProtocolSecureChannel.h"
#include "TraceDecoderProtocolUserDirectedCommissioning.h"

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
    auto protocol = Protocols::Id(VendorId::Common, protocolId);
    if (protocol == Protocols::SecureChannel::Id)
    {
        return securechannel::ToProtocolName();
    }
    else if (protocol == Protocols::InteractionModel::Id)
    {
        return im::ToProtocolName();
    }
    else if (protocol == Protocols::BDX::Id)
    {
        return bdx::ToProtocolName();
    }
    else if (protocol == Protocols::UserDirectedCommissioning::Id)
    {
        return udc::ToProtocolName();
    }
    else if (protocol == Protocols::Echo::Id)
    {
        return echo::ToProtocolName();
    }
    else
    {
        return kUnknown;
    }
}

const char * ToProtocolMessageTypeName(uint16_t protocolId, uint8_t protocolCode)
{
    auto protocol = Protocols::Id(VendorId::Common, protocolId);
    if (protocol == Protocols::SecureChannel::Id)
    {
        return securechannel::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::InteractionModel::Id)
    {
        return im::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::BDX::Id)
    {
        return bdx::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::UserDirectedCommissioning::Id)
    {
        return udc::ToProtocolMessageTypeName(protocolCode);
    }
    else if (protocol == Protocols::Echo::Id)
    {
        return echo::ToProtocolMessageTypeName(protocolCode);
    }
    else
    {
        return kUnknown;
    }
}

CHIP_ERROR LogAsProtocolMessage(uint16_t protocolId, uint8_t protocolCode, const char * payload, size_t len)
{
    constexpr uint16_t kMaxPayloadLen = 2048;
    uint8_t data[kMaxPayloadLen]      = {};
    size_t dataLen                    = Encoding::HexToBytes(payload, len, data, sizeof(data));
    VerifyOrReturnError(dataLen != 0, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = CHIP_NO_ERROR;

    auto protocol = Protocols::Id(VendorId::Common, protocolId);
    if (protocol == Protocols::SecureChannel::Id)
    {
        err = securechannel::LogAsProtocolMessage(protocolCode, data, dataLen);
    }
    else if (protocol == Protocols::InteractionModel::Id)
    {
        err = im::LogAsProtocolMessage(protocolCode, data, dataLen);
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
            return CHIP_NO_ERROR;
        }
    }

    return err;
}

} // namespace trace
} // namespace chip
