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

#pragma once

#include <lib/support/BufferWriter.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <system/SystemClock.h>
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
    StatusReport(GeneralStatusCode generalCode, Protocols::Id protocolId, uint16_t protocolCode);

    //
    /**
     *  Construct a StatusReport with additional ProtocolData.
     *
     *  @param generalCode Must specify a GeneralCode (see \c GeneralStatusCode )
     *  @param protocolId Must specify a ProtocolId which consists of Vendor Id (upper 16 bits) and ProtocolId (lower 16 bits)
     *  @param protocolCode A code defined by the specified protocol which provides more information about the status
     *  @param protocolData A \c PacketBufferHandle containing the protocol-specific data
     */
    StatusReport(GeneralStatusCode generalCode, Protocols::Id protocolId, uint16_t protocolCode,
                 System::PacketBufferHandle protocolData);

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

    GeneralStatusCode GetGeneralCode() const { return mGeneralCode; }
    Protocols::Id GetProtocolId() const { return mProtocolId; }
    uint16_t GetProtocolCode() const { return mProtocolCode; }
    const System::PacketBufferHandle & GetProtocolData() const { return mProtocolData; }

    /**
     * Builds a busy status report with protocol data containing the minimum wait time.
     *
     * @param[in] minimumWaitTime Time in milliseconds before initiator retries the request
     *
     * @return Packet buffer handle which can be passed to SendMessage.
     */
    static System::PacketBufferHandle MakeBusyStatusReportMessage(System::Clock::Milliseconds16 minimumWaitTime);

private:
    GeneralStatusCode mGeneralCode;
    Protocols::Id mProtocolId;
    uint16_t mProtocolCode;

    System::PacketBufferHandle mProtocolData;
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip
