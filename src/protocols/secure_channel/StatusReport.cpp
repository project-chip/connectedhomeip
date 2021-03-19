/**
 *    @file
 *      Implements methods for the StatusReport class.
 */

#include <protocols/secure_channel/StatusReport.h>

#include <support/BufferReader.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>

using namespace chip::Encoding;

namespace chip {
namespace Protocols {
namespace SecureChannel {

StatusReport::StatusReport() : mGeneralCode(0), mProtocolId(0), mProtocolCode(0), mProtocolData(nullptr) {}

StatusReport::StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode) :
    mGeneralCode(generalCode), mProtocolId(protocolId), mProtocolCode(protocolCode), mProtocolData(nullptr)
{}

StatusReport::StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode,
                           System::PacketBufferHandle protocolData) :
    mGeneralCode(generalCode),
    mProtocolId(protocolId), mProtocolCode(protocolCode)
{
    mProtocolData = std::move(protocolData);
}

CHIP_ERROR StatusReport::Parse(System::PacketBufferHandle buf)
{
    ReturnErrorCodeIf(buf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t * bufStart = buf->Start();
    LittleEndian::Reader bufReader(bufStart, buf->DataLength());

    ReturnErrorOnFailure(bufReader.Read16(&mGeneralCode).Read32(&mProtocolId).Read16(&mProtocolCode).StatusCode());

    // Any data that exists after the required fields is considered protocol-specific data.
    if (bufReader.OctetsRead() < buf->DataLength())
    {
        mProtocolData = System::PacketBufferHandle::NewWithData(buf->Start() + bufReader.OctetsRead(),
                                                                buf->DataLength() - bufReader.OctetsRead());
        if (mProtocolData.IsNull())
            return CHIP_ERROR_NO_MEMORY;
    }
    else
    {
        mProtocolData = nullptr;
    }

    return CHIP_NO_ERROR;
}

Encoding::LittleEndian::BufferWriter & StatusReport::WriteToBuffer(Encoding::LittleEndian::BufferWriter & buf) const
{
    buf.Put16(mGeneralCode).Put32(mProtocolId).Put16(mProtocolCode);
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
