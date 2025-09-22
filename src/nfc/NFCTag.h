/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <memory>

#include <lib/core/CHIPError.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemPacketBuffer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

namespace chip {
namespace Nfc {

// Message to send to an NFC Tag.
class DLL_EXPORT NFCMessage
{
private:
    // Data buffer referencing message to send to the tag
    chip::ByteSpan mDataToSend;

    // Buffer to store the actual message data
    std::unique_ptr<uint8_t[]> mDataToSendBuffer;

public:
    // Constructor and assignment
    NFCMessage(System::PacketBufferHandle && msgBuf);
    NFCMessage(NFCMessage && other) noexcept;
    NFCMessage & operator=(NFCMessage && other) noexcept;

    // Deleted Copy Constructor
    NFCMessage(const NFCMessage &) = delete;

    // Deleted Copy Assignment Operator
    NFCMessage & operator=(const NFCMessage &) = delete;

    // Destructor
    ~NFCMessage() = default;

    // Get the data to send
    const chip::ByteSpan & GetDataToSend() const { return mDataToSend; }

    // Check if the message is valid
    bool IsValid() const { return mDataToSendBuffer != nullptr; }
};

/**
 * Abstract class that defines an NFCTag that sends messages and receives responses from a tag.
 */
class DLL_EXPORT NFCTag
{
public:
    struct Identifier
    {
        uint16_t discriminator = 0;

        constexpr Identifier()                     = default;
        constexpr Identifier(const Identifier &)   = default;
        constexpr Identifier(Identifier &&)        = default;
        Identifier & operator=(const Identifier &) = default;
        Identifier & operator=(Identifier &&)      = default;

        bool IsValid() const
        {
            static constexpr Identifier _invalidID;
            return *this != _invalidID;
        }

        friend bool operator==(const Identifier & lhs, const Identifier & rhs)
        {
            return &lhs == &rhs || lhs.discriminator == rhs.discriminator;
        }

        friend bool operator!=(const Identifier & lhs, const Identifier & rhs) { return !(lhs == rhs); }
    };

public:
    explicit NFCTag(const Identifier & identifier, size_t simpleADPUMaxTxSize, size_t simpleADPUMaxRxSize);
    explicit NFCTag(Identifier && identifier, size_t simpleADPUMaxTxSize, size_t simpleADPUMaxRxSize);
    virtual ~NFCTag();

    NFCTag & operator=(const NFCTag &) = delete;
    NFCTag & operator=(NFCTag &&)      = delete;

    bool IsValid() const { return mID.IsValid(); }
    const Identifier & identifier() const { return mID; }

    size_t GetType4SimpleADPUMaxTxSize() const { return mType4SimpleADPUMaxTxSize; }
    size_t GetType4SimpleADPUMaxRxSize() const { return mType4SimpleADPUMaxRxSize; }

    CHIP_ERROR SelectMatterApplet();
    CHIP_ERROR SendMessage(const NFCMessage & message, System::PacketBufferHandle & response);
    CHIP_ERROR SendChainedAPDUs(const ByteSpan & dataToSend, System::PacketBufferHandle & response);
    void Print() const;

protected:
    virtual CHIP_ERROR Transceive(uint8_t * sendBuffer, size_t sendBufferLength, uint8_t * recvBuffer, size_t * recvLength) = 0;

private:
    void ProcessError(const char * msg);
    CHIP_ERROR ProcessAPDUResponse(System::PacketBufferHandle & response);
    void ResetChainedResponseBuffer();
    CHIP_ERROR AddDataToChainedResponseBuffer(uint8_t * data, size_t dataLen);
    CHIP_ERROR SendTransportAPDU(const uint8_t * dataToSend, size_t dataToSendLength, bool isLastBlock, size_t totalLength,
                                 uint8_t * recvBuffer, size_t * recvBufferLength);
    CHIP_ERROR GetResponse(size_t length, uint8_t * recvBuffer, size_t * recvBufferLength);
    CHIP_ERROR Transceive(const char * commandName, uint8_t * sendBuffer, size_t sendBufferLength, uint8_t * recvBuffer,
                          size_t * recvBufferLength);

private:
    Identifier mID = {};

    static constexpr size_t kMaxADPUSize    = 256;
    static constexpr size_t kMaxMessageSize = 1280;

    size_t mType4SimpleADPUMaxTxSize = 0;
    size_t mType4SimpleADPUMaxRxSize = 0;

    uint16_t mDiscriminator = 0;

    // Buffer containing a single APDU command to send to the tag
    uint8_t mAPDUTxBuffer[kMaxADPUSize] = { 0 };

    // Buffer used to receive the response to an APDU command
    uint8_t mAPDURxBuffer[kMaxADPUSize] = { 0 };
    size_t mAPDUResponseLength          = 0;

    // Buffer storing the chained APDU messages received from the tag
    uint8_t mChainedResponseBuffer[kMaxMessageSize] = { 0 };
    size_t mChainedResponseLength                   = 0;
};

} // namespace Nfc
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
