/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>

#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>

#include <type_traits>

using namespace chip::Encoding;
using GeneralStatusCode = chip::Protocols::SecureChannel::GeneralStatusCode;

namespace chip {
namespace Protocols {
namespace SecureChannel {

StatusReport::StatusReport() :
    mGeneralCode(GeneralStatusCode::kSuccess), mProtocolId(SecureChannel::Id), mProtocolCode(0), mProtocolData(nullptr)
{}

StatusReport::StatusReport(GeneralStatusCode generalCode, Protocols::Id protocolId, uint16_t protocolCode) :
    mGeneralCode(generalCode), mProtocolId(protocolId), mProtocolCode(protocolCode), mProtocolData(nullptr)
{}

StatusReport::StatusReport(GeneralStatusCode generalCode, Protocols::Id protocolId, uint16_t protocolCode,
                           System::PacketBufferHandle protocolData) :
    mGeneralCode(generalCode),
    mProtocolId(protocolId), mProtocolCode(protocolCode), mProtocolData(std::move(protocolData))
{}

CHIP_ERROR StatusReport::Parse(System::PacketBufferHandle buf)
{
    uint16_t tempGeneralCode = 0;

    ReturnErrorCodeIf(buf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t * bufStart = buf->Start();
    LittleEndian::Reader bufReader(bufStart, buf->DataLength());

    uint32_t protocolId;
    ReturnErrorOnFailure(bufReader.Read16(&tempGeneralCode).Read32(&protocolId).Read16(&mProtocolCode).StatusCode());
    mProtocolId  = Protocols::Id::FromFullyQualifiedSpecForm(protocolId);
    mGeneralCode = static_cast<GeneralStatusCode>(tempGeneralCode);

    // Any data that exists after the required fields is considered protocol-specific data.
    if (bufReader.OctetsRead() < buf->DataLength())
    {
        mProtocolData = System::PacketBufferHandle::NewWithData(buf->Start() + bufReader.OctetsRead(),
                                                                buf->DataLength() - bufReader.OctetsRead(),
                                                                /* aAdditionalSize = */ 0, /* aReservedSize = */ 0);
        if (mProtocolData.IsNull())
        {
            return CHIP_ERROR_NO_MEMORY;
        }
    }
    else
    {
        mProtocolData = nullptr;
    }

    return CHIP_NO_ERROR;
}

Encoding::LittleEndian::BufferWriter & StatusReport::WriteToBuffer(Encoding::LittleEndian::BufferWriter & buf) const
{
    buf.Put16(to_underlying(mGeneralCode)).Put32(mProtocolId.ToFullyQualifiedSpecForm()).Put16(mProtocolCode);
    if (!mProtocolData.IsNull())
    {
        buf.Put(mProtocolData->Start(), mProtocolData->DataLength());
    }
    return buf;
}

size_t StatusReport::Size() const
{
    LittleEndian::BufferWriter emptyBuf(nullptr, 0);
    return WriteToBuffer(emptyBuf).Needed();
}

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
