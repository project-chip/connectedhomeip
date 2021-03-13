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

StatusReport::StatusReport() : mGeneralCode(0), mProtocolId(0), mProtocolCode(0), mProtocolData(nullptr), mProtocolDataLength(0) {}

StatusReport::StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode) :
    mGeneralCode(generalCode), mProtocolId(protocolId), mProtocolCode(protocolCode), mProtocolData(nullptr), mProtocolDataLength(0)
{}

StatusReport::StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode, uint8_t * protocolData,
                           uint16_t dataLen) :
    mGeneralCode(generalCode),
    mProtocolId(protocolId), mProtocolCode(protocolCode), mProtocolData(protocolData), mProtocolDataLength(dataLen)
{}

CHIP_ERROR StatusReport::Parse(System::PacketBufferHandle buf)
{
    ReturnErrorCodeIf(buf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t * bufStart = buf->Start();
    LittleEndian::Reader bufReader(bufStart, buf->DataLength());

    ReturnErrorOnFailure(bufReader.Read16(&mGeneralCode).Read32(&mProtocolId).Read16(&mProtocolCode).StatusCode());

    if (bufReader.OctetsRead() < buf->DataLength())
    {
        mProtocolData       = buf->Start() + bufReader.OctetsRead();
        mProtocolDataLength = static_cast<uint16_t>(buf->DataLength() - bufReader.OctetsRead());
        mMsgHandle          = std::move(buf);
    }
    else
    {
        mProtocolData       = nullptr;
        mProtocolDataLength = 0;
        mMsgHandle          = nullptr;
    }

    return CHIP_NO_ERROR;
}

Encoding::LittleEndian::BufferWriter & StatusReport::WriteToBuffer(Encoding::LittleEndian::BufferWriter & buf) const
{
    buf.Put16(mGeneralCode).Put32(mProtocolId).Put16(mProtocolCode);
    if (mProtocolData != nullptr)
    {
        buf.Put(mProtocolData, mProtocolDataLength);
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
