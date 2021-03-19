/**
 *    @file
 *      Defines the StatusReport class and General Codes for StatusReport messages.
 */

#pragma once

#include <support/BufferWriter.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

/**
 *  Encapsulates the data included in a StatusReport message, and provides methods for writing to and reading from PacketBuffers
 *  that contain StatusReport messages.
 */
class DLL_EXPORT StatusReport
{
public:
    /**
     *  Construct a StatusReport with zero-d out fields (for use before calling \c Parse() ).
     */
    StatusReport();

    /**
     *  Construct a StatusReport with no additional ProtocolData.
     *
     *  @param generalCode Required, one of the \c GeneralStatusCode values listed in \c secure_channel/Constants.h
     *  @param protocolId Must specify a ProtocolId which consists of Vendor Id (upper 16 bits) and ProtocolId (lower 16 bits)
     *  @param protocolCode A code defined by the specified protocol which provides more information about the status
     */
    StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode);

    //
    /**
     *  Construct a StatusReport with additional ProtocolData.
     *
     *  @param generalCode Must specify a GeneralCode (see \c GeneralStatusCode )
     *  @param protocolId Must specify a ProtocolId which consists of Vendor Id (upper 16 bits) and ProtocolId (lower 16 bits)
     *  @param protocolCode A code defined by the specified protocol which provides more information about the status
     *  @param protocolData A \c PacketBufferHandle containing the protocol-specific data
     */
    StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode, System::PacketBufferHandle protocolData);

    /**
     *  Read the contents of a \c PacketBuffer containing a StatusReport message and store the field values in this object.
     *
     *  @note If there is additional data after the Protocol Code field in the message, it is assumed to be protocol-specific data.
     *
     *  @note This method assumes that the Header of the message has already been consumed, and that \c PacketBuffer::Start() points
     *  to the beginning of the StatusReport data.
     *
     *  @param[in] buf A \c PacketBufferHandle containing the StatusReport message. This method will take ownership, and will
     *                 allocate a new PacketBuffer if any protocol-specific data exists.
     *
     *  @return CHIP_ERROR Return an error if the message is malformed or buf is \c NULL
     */
    CHIP_ERROR Parse(System::PacketBufferHandle buf);

    /**
     *  Write the StatusReport contents into a buffer using a \c BufferWriter
     *
     *  @param[out] buf A \c BufferWriter which contains the buffer that will store the message fields.
     *
     *  @return BufferWriter Return a reference to the \c BufferWriter
     */
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & buf) const;

    /**
     *  Returns the minimum size of the buffer needed to write the message.
     */
    size_t Size() const;

    uint16_t GetGeneralCode() const { return mGeneralCode; }
    uint32_t GetProtocolId() const { return mProtocolId; }
    uint16_t GetProtocolCode() const { return mProtocolCode; }
    System::PacketBufferHandle GetProtocolData() const
    {
        if (mProtocolData.IsNull())
        {
            return System::PacketBufferHandle(nullptr);
        }
        else
        {
            return mProtocolData.Retain();
        }
    }

private:
    uint16_t mGeneralCode  = 0;
    uint32_t mProtocolId   = 0;
    uint16_t mProtocolCode = 0;

    System::PacketBufferHandle mProtocolData;
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
