/**
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

#include <lib/core/CHIPError.h>
#include <nfc/NFCTag.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

namespace chip {
namespace Nfc {

////////////////////////////////////////////////////////////////////////////////////////////////////
// NFCMessage
////////////////////////////////////////////////////////////////////////////////////////////////////

NFCMessage::NFCMessage(System::PacketBufferHandle && msgBuf)
{
    if (msgBuf.IsNull())
    {
        ChipLogError(DeviceLayer, "Input message is null");
        return;
    }

    // Duplicate the data from the PacketBufferHandle
    size_t dataSize = msgBuf->DataLength();
    mDataToSendBuffer.reset(new (std::nothrow) uint8_t[dataSize]);

    if (mDataToSendBuffer)
    {
        std::memcpy(mDataToSendBuffer.get(), msgBuf->Start(), dataSize);

        // Initialize mDataToSend ByteSpan to point to the duplicated buffer
        mDataToSend = chip::ByteSpan(mDataToSendBuffer.get(), dataSize);
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to allocate memory for NFCMessage");
    }
}

NFCMessage::NFCMessage(NFCMessage && other) noexcept :
    mDataToSend(other.mDataToSend), mDataToSendBuffer(std::move(other.mDataToSendBuffer))
{
    other.mDataToSend = chip::ByteSpan();
}

// Move Assignment Operator
NFCMessage & NFCMessage::operator=(NFCMessage && other) noexcept
{
    if (this != &other)
    {
        mDataToSend       = other.mDataToSend;
        mDataToSendBuffer = std::move(other.mDataToSendBuffer);
        other.mDataToSend = chip::ByteSpan();
    }
    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// NFCTag
////////////////////////////////////////////////////////////////////////////////////////////////////

NFCTag::NFCTag(const Identifier & identifier, size_t simpleADPUMaxTxSize, size_t simpleADPUMaxRxSize) :
    mID(identifier), mType4SimpleADPUMaxTxSize(std::min(simpleADPUMaxTxSize, kMaxADPUSize)),
    mType4SimpleADPUMaxRxSize(std::min(simpleADPUMaxRxSize, kMaxADPUSize))
{}

NFCTag::NFCTag(Identifier && identifier, size_t simpleADPUMaxTxSize, size_t simpleADPUMaxRxSize) :
    mID(std::move(identifier)), mType4SimpleADPUMaxTxSize(std::min(simpleADPUMaxTxSize, kMaxADPUSize)),
    mType4SimpleADPUMaxRxSize(std::min(simpleADPUMaxRxSize, kMaxADPUSize))
{}

NFCTag::~NFCTag() {}

void NFCTag::Print() const
{
    ChipLogProgress(DeviceLayer, "Identifier %u", mID.discriminator);
}

// SW1=0x90 SW2=0x00 indicate a successful command
static inline bool IsStatusSuccess(uint8_t sw1, uint8_t sw2)
{
    return ((sw1 == static_cast<uint8_t>(0x90)) && (sw2 == static_cast<uint8_t>(0x00)));
}

static inline System::PacketBufferHandle MakeResponse(uint8_t * response, size_t responseLen)
{
    System::PacketBufferHandle buffer;
    if (response && responseLen > 0)
    {
        buffer =
            System::PacketBufferHandle::NewWithData(reinterpret_cast<const uint8_t *>(response), static_cast<size_t>(responseLen));
    }
    return buffer;
}

// SW1=0x61 SW2=0xXX indicate that the last command was successful and that the tag is transmitting
// a chained response. It is used when the response is too long to be transmitted in one shot.
// SW2 indicates the size of the data in the next block. It can be read by calling
// 'GetResponse' command.
static inline bool IsResponseBlockAvailable(uint8_t sw1)
{
    return (sw1 == static_cast<uint8_t>(0x61));
}

static inline void PrintSw1Sw2(uint8_t sw1, uint8_t sw2)
{
    ChipLogProgress(DeviceLayer, "SW1=0x%x SW2=0x%x", sw1, sw2);
}

CHIP_ERROR NFCTag::SendMessage(const NFCMessage & message, System::PacketBufferHandle & response)
{
    return SendChainedAPDUs(message.GetDataToSend(), response);
}

// Function sending a message to an NFC Tag.
// If the data doesn't fit in a single APDU, it will be split in
// several chained APDUs.
//
// Same thing for the tag's response:
// If the response doesn't fit in 255 bytes, it will be chained.
//
// When the tag's response is fully received, OnNfcTagResponse() notification
// will be called.
CHIP_ERROR NFCTag::SendChainedAPDUs(const ByteSpan & dataToSend, System::PacketBufferHandle & response)
{
    // Make sure max buffer sizes are not set to 0 and ID is valid
    if (mType4SimpleADPUMaxTxSize == 0 || mType4SimpleADPUMaxRxSize == 0 || !mID.IsValid())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR result;
    size_t totalLength             = dataToSend.size();
    size_t nbrOfBytesRemaining     = dataToSend.size();
    const uint8_t * nextDataToSend = dataToSend.data();

    if (!nextDataToSend)
    {
        return CHIP_ERROR_INVALID_ADDRESS;
    }

    while (nbrOfBytesRemaining > 0)
    {

        // Size of the next APDU
        size_t nbrOfBytesToSend = std::min(nbrOfBytesRemaining, mType4SimpleADPUMaxTxSize);

        bool isLastBlock = (nbrOfBytesToSend == nbrOfBytesRemaining);

        mAPDUResponseLength = sizeof(mAPDURxBuffer); // Initialized with with mAPDURxBuffer size to indicate to the low level
                                                     // driver the capacity of the RX buffer
        result = SendTransportAPDU(nextDataToSend, nbrOfBytesToSend, isLastBlock, totalLength, mAPDURxBuffer, &mAPDUResponseLength);
        if ((result != CHIP_NO_ERROR) || (mAPDUResponseLength < 2))
        {
            ProcessError("Invalid NFC Type4 response");
            return result;
        }

        uint8_t sw1 = mAPDURxBuffer[mAPDUResponseLength - 2];
        uint8_t sw2 = mAPDURxBuffer[mAPDUResponseLength - 1];

        if (isLastBlock)
        {
            // The last block has been sent.
            // The possible response are:
            //   SW1=0x90 SW2=0x00 if the transmission was successful and the recipient has
            //     transmitted a short response (less than 256 bytes).
            //     transmitted the first part of a chained response (SW2 indicates the size of
            //   SW1=0x61 SW2=0xXX if the transmission was successful and the recipient has
            //     the data in the next block).
            if (IsStatusSuccess(sw1, sw2) || IsResponseBlockAvailable(sw1))
            {
                // Command successful. Response will be processed outside of the while loop
            }
            else
            {
                // Any others values are an error
                PrintSw1Sw2(sw1, sw2);
                ProcessError("Error during chained APDUs");
                return CHIP_ERROR_MESSAGE_INCOMPLETE;
            }
        }
        else
        {
            // This is an intermediate block so the only valid response is 0x90 0x00
            if (IsStatusSuccess(sw1, sw2))
            {
                // The command was successfully sent
                // Continue with the next block
            }
            else
            {
                PrintSw1Sw2(sw1, sw2);
                ProcessError("Error during chained APDUs");
                return CHIP_ERROR_MESSAGE_INCOMPLETE;
            }
        }

        // APDU sent successfully
        nbrOfBytesRemaining -= nbrOfBytesToSend;
        nextDataToSend += nbrOfBytesToSend;
    }

    // The command has been successfully sent and we have received the first response block in mAPDURxBuffer.
    // Process it and read the next blocks (if any)
    return ProcessAPDUResponse(response);
}

// Process an APDU response contained in mAPDURxBuffer
CHIP_ERROR NFCTag::ProcessAPDUResponse(System::PacketBufferHandle & response)
{

    // mAPDURxBuffer should contain at least  the 2 status bytes
    if (mAPDUResponseLength < 2)
    {
        ProcessError("Error! Invalid Type4 response");
        return CHIP_ERROR_MESSAGE_INCOMPLETE;
    }

    ResetChainedResponseBuffer();

    uint8_t sw1 = mAPDURxBuffer[mAPDUResponseLength - 2];
    uint8_t sw2 = mAPDURxBuffer[mAPDUResponseLength - 1];

    if (IsStatusSuccess(sw1, sw2))
    {
        // Response fits in a single block.
        // Drop the 2 status bytes and return it
        response = MakeResponse(mAPDURxBuffer, mAPDUResponseLength - 2);
        return CHIP_NO_ERROR;
    }

    if (!IsResponseBlockAvailable(sw1))
    {
        PrintSw1Sw2(sw1, sw2);
        ProcessError("Error! Invalid APDU response");
        return CHIP_ERROR_MESSAGE_INCOMPLETE;
    }

    // SW1=0x61 indicates a chained response. it means that the response is too big to be transmitted in a single packet.

    // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
    CHIP_ERROR result = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
    if (result != CHIP_NO_ERROR)
    {
        ProcessError("Too many data!");
        return result;
    }

    // Loop until there is no more response block to read
    // SW2 indicates the size of the data in the next response block.
    // It can be read thanks to a call to getResponse() command.
    while (IsResponseBlockAvailable(sw1))
    {
        // If SW2 is 0x00 or if it is higher than mType4SimpleADPUMaxRxSize, we clamp it to
        // mType4SimpleADPUMaxRxSize.
        size_t nextBlockLength = ((sw2 == 0x00) || (sw2 > mType4SimpleADPUMaxRxSize)) ? mType4SimpleADPUMaxRxSize : sw2;

        mAPDUResponseLength = sizeof(mAPDURxBuffer); // Initialized with with mAPDURxBuffer size to indicate to the low
                                                     // level driver the capacity of the RX buffer
        // Response will be written into mAPDURxBuffer
        result = GetResponse(nextBlockLength, mAPDURxBuffer, &mAPDUResponseLength);
        if ((result != CHIP_NO_ERROR) || (mAPDUResponseLength < 2))
        {
            ProcessError("Invalid NFC Type4 response");
            return result;
        }

        sw1 = mAPDURxBuffer[mAPDUResponseLength - 2];
        sw2 = mAPDURxBuffer[mAPDUResponseLength - 1];

        if (IsStatusSuccess(sw1, sw2))
        {
            // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
            result = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
            if (result != CHIP_NO_ERROR)
            {
                ProcessError("Response has too much data!");
                return result;
            }
        }
        else if (IsResponseBlockAvailable(sw1))
        {
            // We have successfully received a block and it is not the last one

            // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
            result = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
            if (result != CHIP_NO_ERROR)
            {
                ProcessError("Response has too much data!");
                return result;
            }
        }
        else
        {
            PrintSw1Sw2(sw1, sw2);
            ProcessError("Error! Invalid APDU response");
            return CHIP_ERROR_MESSAGE_INCOMPLETE;
        }

        // If SW1 still equals to 0x61, continue to read the next response block
    }

    // Chained response received successfully
    response = MakeResponse(mChainedResponseBuffer, mChainedResponseLength);
    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCTag::SelectMatterApplet()
{
    VerifyOrReturnLogError(mID.IsValid(), CHIP_ERROR_INCORRECT_STATE);

    uint8_t dataReceived[10]       = { 0 };
    size_t receivedLength          = sizeof(dataReceived);
    uint8_t select_matter_applet[] = {
        0x00, 0xA4, 0x04, 0x0C, 0x09,                         // CLA INS P1 P2 Lc
        0xA0, 0x00, 0x00, 0x09, 0x09, 0x8A, 0x77, 0xE4, 0x01, // Matter AID
        0x00                                                  // Le
    };

    ChipLogDetail(DeviceLayer, "NFCTag: selecting AID on tag %u", mID.discriminator);
    CHIP_ERROR result = Transceive(select_matter_applet, sizeof(select_matter_applet), dataReceived, &receivedLength);
    if (result == CHIP_NO_ERROR)
    {
        if (receivedLength >= 10)
        {
            mDiscriminator = static_cast<uint16_t>((static_cast<uint16_t>(dataReceived[2]) << 8) | dataReceived[3]);
            ChipLogDetail(DeviceLayer, "NFCTag: received SELECT AID response on tag %u with discriminatorToMatch %u",
                          mID.discriminator, mDiscriminator);
        }
        else
        {
            ChipLogError(DeviceLayer, "Response to Select Matter AID cmd is too small (%lu bytes), 0x%x, 0x%x, 0x%x, 0x%x",
                         static_cast<unsigned long>(receivedLength), dataReceived[0], dataReceived[1], dataReceived[2],
                         dataReceived[3]);
            result = CHIP_ERROR_INVALID_MESSAGE_LENGTH;
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to Select Matter AID due to error: %s", result.AsString());
    }
    return result;
}

void NFCTag::ProcessError(const char * msg)
{
    ChipLogError(DeviceLayer, "NFCTag: %s", msg);
}

void NFCTag::ResetChainedResponseBuffer()
{
    mChainedResponseLength = 0;
}

CHIP_ERROR NFCTag::AddDataToChainedResponseBuffer(uint8_t * data, size_t dataLen)
{
    if (!data)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Check that mChainedResponseBuffer will not overflow
    VerifyOrReturnLogError((mChainedResponseLength + dataLen) <= sizeof(mChainedResponseBuffer), CHIP_ERROR_MESSAGE_TOO_LONG);

    memcpy(&(mChainedResponseBuffer[mChainedResponseLength]), data, dataLen);

    mChainedResponseLength += dataLen;

    return CHIP_NO_ERROR;
}

/////////////////////////////////////////////////////////////////

CHIP_ERROR NFCTag::SendTransportAPDU(const uint8_t * dataToSend, size_t dataToSendLength, bool isLastBlock, size_t totalLength,
                                     uint8_t * recvBuffer, size_t * recvLength)
{

    VerifyOrReturnLogError(dataToSend != nullptr, CHIP_ERROR_INVALID_ADDRESS);
    VerifyOrReturnLogError(recvBuffer != nullptr && recvLength != nullptr, CHIP_ERROR_INVALID_ADDRESS);
    VerifyOrReturnLogError((dataToSendLength + 6) <= sizeof(mAPDUTxBuffer), CHIP_ERROR_BUFFER_TOO_SMALL);

    mAPDUTxBuffer[0] = isLastBlock ? static_cast<uint8_t>(0x80) : static_cast<uint8_t>(0x90); // CLA
    mAPDUTxBuffer[1] = 0x20;                                                                  // INS
    mAPDUTxBuffer[2] = static_cast<uint8_t>((totalLength >> 8) & 0xFF);                       // P1 (contains the totalLength's MSB)
    mAPDUTxBuffer[3] = static_cast<uint8_t>(totalLength & 0xFF);                              // P2 (contains the totalLength's LSB)
    mAPDUTxBuffer[4] = static_cast<uint8_t>(dataToSendLength);                                // Lc
    memcpy(&(mAPDUTxBuffer[5]), dataToSend, dataToSendLength);
    mAPDUTxBuffer[5 + dataToSendLength] = static_cast<uint8_t>(mType4SimpleADPUMaxRxSize); // Le

    size_t apduLength = 6 + dataToSendLength;

    CHIP_ERROR result = Transceive("SendTransportAPDU", mAPDUTxBuffer, apduLength, recvBuffer, recvLength);

    return result;
}

CHIP_ERROR NFCTag::GetResponse(size_t length, uint8_t * recvBuffer, size_t * recvBufferLength)
{
    uint8_t frame[5];

    frame[0] = 0x00;                         // CLA
    frame[1] = static_cast<uint8_t>(0xC0);   // INS
    frame[2] = 0x00;                         // P1
    frame[3] = 0x00;                         // P2
    frame[4] = static_cast<uint8_t>(length); // Le

    CHIP_ERROR result = Transceive("GetResponse", frame, sizeof(frame), recvBuffer, recvBufferLength);

    return result;
}

CHIP_ERROR NFCTag::Transceive(const char * commandName, uint8_t * sendBuffer, size_t sendBufferLength, uint8_t * recvBuffer,
                              size_t * recvLength)
{
    // Before the transceive action, recvLength contains the size of recvBuffer buffer.
    // After the transceive, it will contain the length of the received data.
    CHIP_ERROR result = Transceive(sendBuffer, sendBufferLength, recvBuffer, recvLength);

    if ((result == CHIP_NO_ERROR) && (*recvLength >= 2))
    {
        ChipLogDetail(DeviceLayer, "Transceive succeeded and %lu bytes were received from tag %u",
                      static_cast<unsigned long>(*recvLength), mDiscriminator);
    }
    else
    {
        ChipLogError(DeviceLayer, "Transceive failed with error: %s", result.AsString());
        result = CHIP_ERROR_INVALID_MESSAGE_LENGTH;
    }

    return result;
}

} // namespace Nfc
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
