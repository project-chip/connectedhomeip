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

/**
 *    @file
 *          Provides an implementation of the NFCCommissioningManager singleton object
 *          for Android platforms.
 */

#include <stdio.h>
#include <stdlib.h>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/internal/NFCCommissioningManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

using namespace chip;
using namespace ::nl;
using namespace ::chip::Nfc;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {} // namespace

#define MAX_APDU_SIZE 256
#define MAX_MESSAGE_SIZE 1280

#define APDU_HEADER_SIZE_IN_BYTES 7
#define APDU_LE_SIZE_IN_BYTES 3

#define TYPE4_SIMPLE_APDU_MAX_TX_SIZE 245 // NB: 245 is the optimum size to get the biggest TX chunks and thus the max throughput
#define TYPE4_SIMPLE_APDU_MAX_RX_SIZE 250 // NB: 250 is the optimum size to get the biggest RX chunks and thus the max throughput

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#define CHECK(f, rv)                                                                                                               \
    if (SCARD_S_SUCCESS != rv)                                                                                                     \
    {                                                                                                                              \
        ChipLogError(DeviceLayer, "%s : %s", f, pcsc_stringify_error(rv));                                                         \
        return CHIP_ERROR_INTERNAL;                                                                                                \
    }

static SCARD_IO_REQUEST pioSendPci;

// Message to send to an NFC Tag.
class NFCMessage
{
private:
    // Pointer to the NFC Tag instance to communicate with
    TagInstance * pTagInstance;

    uint8_t * pMessage;
    size_t messageSize = 0;

public:
    NFCMessage(TagInstance * instance, System::PacketBufferHandle && msgBuf) : pTagInstance(instance)
    {
        messageSize = msgBuf->DataLength();
        pMessage    = new uint8_t[messageSize];
        std::memcpy(pMessage, msgBuf->Start(), messageSize);
    }

    ~NFCMessage() { delete[] pMessage; }

    TagInstance * GetTagInstance() { return pTagInstance; }

    uint8_t * GetMessage() { return pMessage; }

    size_t GetMessageSize() { return messageSize; }
};

// This NFC Tag contains all the variables, handles and buffers related to one NFC tag instance
class TagInstance
{
private:
    Transport::NFCBase * nfcBase;
    const Transport::PeerAddress peerAddress;
    char * readerName;
    SCARDHANDLE cardHandle;
    uint16_t discriminator;

    // Buffer containing a single APDU command to send to the tag
    uint8_t mAPDUTxBuffer[MAX_APDU_SIZE];

    // Buffer used to receive the response to an APDU command
    uint8_t mAPDURxBuffer[MAX_APDU_SIZE];
    uint32_t mAPDUResponseLength = 0;

    // Buffer storing the chained APDU messages received from the tag
    uint8_t mChainedResponseBuffer[MAX_MESSAGE_SIZE];
    uint32_t mChainedResponseLength = 0;

public:
    TagInstance(Transport::NFCBase * base, const Transport::PeerAddress address, const char * name, SCARDHANDLE handle) :
        nfcBase(base), peerAddress(address), cardHandle(handle)
    {
        readerName    = strdup(name); // Allocate memory and copy the string
        discriminator = 0;            // Will be retrieved when RetrieveDiscriminator() is called

        memset(mAPDUTxBuffer, 0, sizeof(mAPDUTxBuffer));
        memset(mAPDURxBuffer, 0, sizeof(mAPDURxBuffer));
        memset(mChainedResponseBuffer, 0, sizeof(mChainedResponseBuffer));
    }

    ~TagInstance()
    {
        if (readerName != nullptr)
        {
            free(readerName);
        }
    }

    void Print()
    {
        ChipLogProgress(DeviceLayer, "TagInstance %p", this);
        ChipLogProgress(DeviceLayer, "cardHandle 0x" ChipLogFormatX64, ChipLogValueX64(cardHandle));
    }

    const char * GetReaderName() const { return readerName; }

    SCARDHANDLE GetCardHandle() const { return cardHandle; }

    uint16_t GetDiscriminator() const { return discriminator; }

    // SW1=0x90 SW2=0x00 indicate a successful command
    bool IsStatusSuccess(uint8_t sw1, uint8_t sw2)
    {
        return ((sw1 == static_cast<uint8_t>(0x90)) && (sw2 == static_cast<uint8_t>(0x00)));
    }

    // SW1=0x61 SW2=0xXX indicate that the last command was successful and that the tag is transmitting
    // a chained response. It is used when the response is too long to be transmitted in one shot.
    // SW2 indicates the size of the data in the next block. It can be read by calling
    // 'GetResponse' command.
    bool IsResponseBlockAvailable(uint8_t sw1) { return (sw1 == static_cast<uint8_t>(0x61)); }

    CHIP_ERROR RetrieveDiscriminator()
    {
        // Discriminator can be found in the response to select application
        CHIP_ERROR res = SelectMatterApplet();
        return res;
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
    void SendChainedAPDUs(uint8_t * pMessage, size_t messageSize)
    {
        CHIP_ERROR res;
        uint32_t totalLength         = static_cast<uint32_t>(messageSize);
        uint32_t nbrOfBytesRemaining = static_cast<uint32_t>(messageSize);
        uint8_t * pNextDataToSend    = pMessage;

        while (nbrOfBytesRemaining > 0)
        {

            // Size of the next APDU
            uint32_t nbrOfBytesToSend = MIN(nbrOfBytesRemaining, TYPE4_SIMPLE_APDU_MAX_TX_SIZE);

            bool isLastBlock = (nbrOfBytesToSend == nbrOfBytesRemaining);

            mAPDUResponseLength = sizeof(mAPDURxBuffer); // Initialized with with mAPDURxBuffer size to indicate to the low level
                                                         // driver the capacity of the RX buffer
            res =
                SendTransportAPDU(pNextDataToSend, nbrOfBytesToSend, isLastBlock, totalLength, mAPDURxBuffer, &mAPDUResponseLength);
            if ((res != CHIP_NO_ERROR) || (mAPDUResponseLength < 2))
            {
                ProcessError("Invalid NFC Type4 response");
                return;
            }

            uint8_t sw1 = mAPDURxBuffer[mAPDUResponseLength - 2];
            uint8_t sw2 = mAPDURxBuffer[mAPDUResponseLength - 1];

            if (isLastBlock)
            {
                // The last block has been sent.
                // The possible response are:
                //   SW1=0x90 SW2=0x00 if the transmission was successful and the recipient has
                //     transmitted a short response (less than 256 bytes).
                //   SW1=0x61 SW2=0xXX if the transmission was successful and the recipient has
                //     transmitted the first part of a chained response (SW2 indicates the size of
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
                    return;
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
                    return;
                }
            }

            // APDU sent successfully
            nbrOfBytesRemaining -= nbrOfBytesToSend;
            pNextDataToSend += nbrOfBytesToSend;
        }

        if (nbrOfBytesRemaining != 0)
        {
            ProcessError("Error during APDUs transmission!");
            return;
        }

        // The command has been successfully sent and we have received the first response block in mAPDURxBuffer.
        // Process it and read the next blocks (if any)
        ProcessAPDUResponse();
    }

    // Process an APDU response contained in mAPDURxBuffer
    void ProcessAPDUResponse()
    {
        CHIP_ERROR res;

        // mAPDURxBuffer should contain at least  the 2 status bytes
        if (mAPDUResponseLength < 2)
        {
            ProcessError("Error! Invalid Type4 response");
            return;
        }

        ResetChainedResponseBuffer();

        uint8_t sw1 = mAPDURxBuffer[mAPDUResponseLength - 2];
        uint8_t sw2 = mAPDURxBuffer[mAPDUResponseLength - 1];

        if (IsStatusSuccess(sw1, sw2))
        {
            // Response fits in a single block.
            // Drop the 2 status bytes and return it
            NotifyResponse(mAPDURxBuffer, mAPDUResponseLength - 2);
            return;
        }
        else if (IsResponseBlockAvailable(sw1))
        {
            // SW1=0x61 indicates a chained response. it means that the response is too big to be transmitted in a single packet.

            // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
            res = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
            if (res != CHIP_NO_ERROR)
            {
                ProcessError("Too many data!");
                return;
            }

            // Loop until there is no more response block to read
            // SW2 indicates the size of the data in the next response block.
            // It can be read thanks to a call to getResponse() command.
            while (IsResponseBlockAvailable(sw1))
            {
                // If SW2 is 0x00 or if it is higher than TYPE4_SIMPLE_APDU_MAX_RX_SIZE, we clamp it to
                // TYPE4_SIMPLE_APDU_MAX_RX_SIZE.
                uint8_t nextBlockLength = ((sw2 == 0x00) || (static_cast<int>(sw2) > TYPE4_SIMPLE_APDU_MAX_RX_SIZE))
                    ? static_cast<uint8_t>(TYPE4_SIMPLE_APDU_MAX_RX_SIZE)
                    : sw2;

                mAPDUResponseLength = sizeof(mAPDURxBuffer); // Initialized with with mAPDURxBuffer size to indicate to the low
                                                             // level driver the capacity of the RX buffer
                // Response will be written into mAPDURxBuffer
                res = GetResponse(nextBlockLength, mAPDURxBuffer, &mAPDUResponseLength);
                if ((res != CHIP_NO_ERROR) || (mAPDUResponseLength < 2))
                {
                    ProcessError("Invalid NFC Type4 response");
                    return;
                }

                sw1 = mAPDURxBuffer[mAPDUResponseLength - 2];
                sw2 = mAPDURxBuffer[mAPDUResponseLength - 1];

                if (IsStatusSuccess(sw1, sw2))
                {
                    // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
                    res = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
                    if (res != CHIP_NO_ERROR)
                    {
                        ProcessError("Too many data!");
                        return;
                    }
                }
                else if (IsResponseBlockAvailable(sw1))
                {
                    // We have successfully received a block and it is not the last one

                    // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
                    res = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
                    if (res != CHIP_NO_ERROR)
                    {
                        ProcessError("Too many data!");
                        return;
                    }
                }
                else
                {
                    PrintSw1Sw2(sw1, sw2);
                    ProcessError("Error! Invalid APDU response");
                    return;
                }

                // If SW1 still equals to 0x61, continue to read the next response block
            }

            // Chained response received successfully
            NotifyResponse(mChainedResponseBuffer, mChainedResponseLength);
            return;
        }
        else
        {
            PrintSw1Sw2(sw1, sw2);
            ProcessError("Error! Invalid APDU response");
            return;
        }
    }

    CHIP_ERROR SelectMatterApplet()
    {
        CHIP_ERROR res;

        VerifyOrReturnLogError(cardHandle != 0, CHIP_ERROR_INTERNAL);

        BYTE dataReceived[10];
        DWORD receivedLength        = sizeof(dataReceived);
        BYTE select_matter_applet[] = {
            0x00, 0xA4, 0x04, 0x0C, 0x09,                         // CLA INS P1 P2 Lc
            0xA0, 0x00, 0x00, 0x09, 0x09, 0x8A, 0x77, 0xE4, 0x01, // Matter AID
            0x00                                                  // Le
        };

        LONG result = SCardTransmit(cardHandle, &pioSendPci, select_matter_applet, sizeof(select_matter_applet), NULL, dataReceived,
                                    &receivedLength);
        if (result == SCARD_S_SUCCESS)
        {
            if (receivedLength >= 10)
            {
                discriminator = static_cast<uint16_t>(dataReceived[2] * 256 + dataReceived[3]);
                res           = CHIP_NO_ERROR;
            }
            else
            {
                ChipLogError(DeviceLayer, "Response to Select Matter AID cmd is too small (%d bytes)",
                             static_cast<unsigned int>(receivedLength));
                res = CHIP_ERROR_INTERNAL;
            }
        }
        else
        {
            res = CHIP_ERROR_INTERNAL;
        }

        return res;
    }

    void PrintSw1Sw2(uint8_t sw1, uint8_t sw2) { ChipLogProgress(DeviceLayer, "SW1=0x%x SW2=0x%x", sw1, sw2); }

    void ProcessError(const char * msg)
    {
        ChipLogError(DeviceLayer, "%s", msg);
        SendOnNfcTagError();
    }

    void NotifyResponse(uint8_t * response, uint32_t responseLen)
    {

        System::PacketBufferHandle buffer =
            System::PacketBufferHandle::NewWithData(reinterpret_cast<const uint8_t *>(response), static_cast<size_t>(responseLen));
        SendOnNfcTagResponse(std::move(buffer));
    }

    void ResetChainedResponseBuffer(void) { mChainedResponseLength = 0; }

    CHIP_ERROR AddDataToChainedResponseBuffer(uint8_t * data, int dataLen)
    {
        // Check that mChainedResponseBuffer will not overflow
        VerifyOrReturnLogError((mChainedResponseLength + dataLen) <= sizeof(mChainedResponseBuffer), CHIP_ERROR_MESSAGE_TOO_LONG);

        memcpy(&(mChainedResponseBuffer[mChainedResponseLength]), data, dataLen);

        mChainedResponseLength += dataLen;

        return CHIP_NO_ERROR;
    }

    /////////////////////////////////////////////////////////////////

    CHIP_ERROR SendTransportAPDU(uint8_t * dataToSend, uint32_t dataToSendLength, bool isLastBlock, uint32_t totalLength,
                                 uint8_t * pRcvBuffer, uint32_t * pRcvLength)
    {

        VerifyOrReturnLogError(dataToSendLength <= sizeof(mAPDUTxBuffer), CHIP_ERROR_INTERNAL);

        mAPDUTxBuffer[0] = isLastBlock ? static_cast<uint8_t>(0x80) : static_cast<uint8_t>(0x90); // CLA
        mAPDUTxBuffer[1] = 0x20;                                                                  // INS
        mAPDUTxBuffer[2] = static_cast<uint8_t>((totalLength >> 8) & 0xFF); // P1 (contains the totalLength's MSB)
        mAPDUTxBuffer[3] = static_cast<uint8_t>(totalLength & 0xFF);        // P2 (contains the totalLength's LSB)
        mAPDUTxBuffer[4] = static_cast<uint8_t>(dataToSendLength);          // Lc
        memcpy(&(mAPDUTxBuffer[5]), dataToSend, dataToSendLength);
        mAPDUTxBuffer[5 + dataToSendLength] = static_cast<uint8_t>(TYPE4_SIMPLE_APDU_MAX_RX_SIZE); // Le

        uint32_t apduLength = 6 + dataToSendLength;

        CHIP_ERROR result = Transceive("SendTransportAPDU", mAPDUTxBuffer, apduLength, pRcvBuffer, pRcvLength);

        return result;
    }

    CHIP_ERROR GetResponse(uint8_t length, uint8_t * pRcvBuffer, uint32_t * pRcvLength)
    {
        uint8_t frame[5];

        frame[0] = 0x00;                       // CLA
        frame[1] = static_cast<uint8_t>(0xC0); // INS
        frame[2] = 0x00;                       // P1
        frame[3] = 0x00;                       // P2
        frame[4] = length;                     // Le

        CHIP_ERROR result = Transceive("GetResponse", frame, sizeof(frame), pRcvBuffer, pRcvLength);

        return result;
    }

    CHIP_ERROR Transceive(const char * commandName, uint8_t * pSendBuffer, uint32_t sendBufferLength, uint8_t * pRcvBuffer,
                          uint32_t * pRcvLength)
    {
        CHIP_ERROR ret;

        // Use a local 'dwRecvLength' variable to avoid cast of pRcvLength from 'uint32_t *' to 'DWORD *'.
        // Before the transceive action, those variables contain the size of pRcvBuffer buffer.
        // After the transceive, they will contain the length of the received data.
        DWORD dwRecvLength = static_cast<DWORD>(*pRcvLength);

        LONG result = SCardTransmit(cardHandle, &pioSendPci, pSendBuffer, sendBufferLength, NULL, pRcvBuffer, &dwRecvLength);

        if ((result == SCARD_S_SUCCESS) && (dwRecvLength >= 2))
        {
            // Copy the length of received data to pRcvLength
            *pRcvLength = static_cast<uint32_t>(dwRecvLength);

            ret = CHIP_NO_ERROR;
        }
        else
        {
            ChipLogError(DeviceLayer, "SCardTransmit failed with error 0x" ChipLogFormatX64, ChipLogValueX64(result));
            ret = CHIP_ERROR_INTERNAL;
        }

        return ret;
    }

    /////////////////////////////////////////////////////////////////

    CHIP_ERROR SendOnNfcTagResponse(System::PacketBufferHandle && buffer)
    {
        nfcBase->OnNfcTagResponse(peerAddress, std::move(buffer));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendOnNfcTagError()
    {
        nfcBase->OnNfcTagError(peerAddress);
        return CHIP_NO_ERROR;
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////

SCARDCONTEXT hPcscContext;
TagInstance * pLastTagInstanceUsed;

// Empty vector of TagInstance pointers
std::vector<TagInstance *> tagInstances;

NFCCommissioningManagerImpl NFCCommissioningManagerImpl::sInstance;

// ===== start impl of NFCCommissioningManager internal interface, ref NFCCommissioningManager.h

CHIP_ERROR NFCCommissioningManagerImpl::_Init()
{
    // Creates an Application Context to the PC/SC Resource Manager.
    long result = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hPcscContext);
    CHECK("SCardEstablishContext", result)

    pLastTagInstanceUsed = nullptr;

    return CHIP_NO_ERROR;
}

void NFCCommissioningManagerImpl::_Shutdown() {}

// ===== start implement virtual methods on NfcApplicationDelegate.

void NFCCommissioningManagerImpl::SetNFCBase(Transport::NFCBase * nfcBase)
{
    mNFCBase = nfcBase;
}

bool NFCCommissioningManagerImpl::CanSendToPeer(const Transport::PeerAddress & address)
{
    bool canSendToPeer = false;

    // nfcShortId is used to find the peer device
    uint16_t nfcShortId = address.GetNFCShortId();

    // Check if pLastTagInstanceUsed corresponds to the same nfcShortId
    if ((pLastTagInstanceUsed != nullptr) && (pLastTagInstanceUsed->GetDiscriminator() == nfcShortId))
    {
        return true;
    }

    // Check if we already have a TagInstance corresponding to this nfcShortId
    TagInstance * pTagInstance = SearchTagInstanceFromDiscriminator(nfcShortId);

    if (pTagInstance != nullptr)
    {
        // Found a matching cardHandle
        canSendToPeer = true;
    }
    else
    {
        // We don't have yet a cardHandle for this nfcShortId

        // Scan all the readers and tags
        ScanAllReaders(nfcShortId);

        // and check if we now have a TagInstance corresponding to this nfcShortId
        pTagInstance = SearchTagInstanceFromDiscriminator(nfcShortId);

        canSendToPeer = (pTagInstance != nullptr);
    }

    // Save the pointer to this TagInstance for a faster access at next call
    pLastTagInstanceUsed = pTagInstance;

    return canSendToPeer;
}

CHIP_ERROR NFCCommissioningManagerImpl::SendToNfcTag(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    TagInstance * pTargetedTagInstance = nullptr;

    // nfcShortId is used to find the peer device
    uint16_t nfcShortId = address.GetNFCShortId();

    // Check if pLastTagInstanceUsed corresponds to the same nfcShortId
    if ((pLastTagInstanceUsed != nullptr) && (pLastTagInstanceUsed->GetDiscriminator() == nfcShortId))
    {
        pTargetedTagInstance = pLastTagInstanceUsed;
    }
    else
    {
        // Search in all the TagInstance if there is one with this Discriminator
        TagInstance * pTagInstance = SearchTagInstanceFromDiscriminator(nfcShortId);
        if (pTagInstance != nullptr)
        {
            // Found an instance with expected Discriminator
            pTargetedTagInstance = pTagInstance;
        }
    }

    VerifyOrReturnLogError(pTargetedTagInstance != nullptr, CHIP_ERROR_PEER_NODE_NOT_FOUND);

    NFCMessage * nfcMessage = new (std::nothrow) NFCMessage(pTargetedTagInstance, std::move(msgBuf));
    VerifyOrReturnLogError(((nfcMessage != nullptr) && (nfcMessage->GetMessage() != nullptr)), CHIP_ERROR_NO_MEMORY);

    // SendChainedAPDUs(pTagInstance) will be executed asynchronously
    PlatformMgr().ScheduleWork(SendToNfcTag, reinterpret_cast<intptr_t>(nfcMessage));

    return CHIP_NO_ERROR;
}

// Static function called by ScheduleWork()
void NFCCommissioningManagerImpl::SendToNfcTag(intptr_t arg)
{
    sInstance.SendChainedAPDUs(arg);
}

void NFCCommissioningManagerImpl::SendChainedAPDUs(intptr_t arg)
{
    NFCMessage * nfcMessage = reinterpret_cast<NFCMessage *>(arg);
    if (nfcMessage == nullptr)
    {
        ChipLogError(DeviceLayer, "Invalid nfcMessage!");
        return;
    }

    uint8_t * pMessage = nfcMessage->GetMessage();
    size_t messageSize = nfcMessage->GetMessageSize();
    nfcMessage->GetTagInstance()->SendChainedAPDUs(pMessage, messageSize);

    // When we reach this point, the message has been sent, the response has been
    //  received. nfcMessage can be safely deleted.
    delete nfcMessage;
}

// Start scan on all available readers and scan for NFC Tags.
CHIP_ERROR NFCCommissioningManagerImpl::ScanAllReaders(uint16_t nfcShortId)
{
    long result;
    LPTSTR mszReaders; // LPTSTR is a "typedef char *"
    DWORD dwReaders;

    result = SCardListReaders(hPcscContext, NULL, NULL, &dwReaders);
    CHECK("SCardListReaders", result)

    // dwReaders now contains "mszReaders" data size
    // Allocate a buffer where we will store "mszReaders" multi-string.
    mszReaders = static_cast<LPTSTR>(calloc(dwReaders, sizeof(char)));
    if (mszReaders == nullptr)
    {
        ChipLogError(DeviceLayer, "Memory allocation failed");
        return CHIP_ERROR_NO_MEMORY;
    }

    result = SCardListReaders(hPcscContext, NULL, mszReaders, &dwReaders);
    CHECK("SCardListReaders", result)

    // "mszReaders" contains a multi-string with list of readers.
    // Each reader name is separated by a null character ('\0') and ended by a double null character
    // Example : "Reader1\0Reader2\0\0"

    // Pointer to traverse the string
    char * reader = mszReaders;

    // Loop through the string
    while (*reader != '\0' || *(reader + 1) != '\0')
    {
        if (*reader != '\0')
        {
            ScanReader(nfcShortId, reader);

            // Move the pointer to the next substring
            reader += strlen(reader) + 1;
        }
        else
        {
            // Move the pointer to the next character
            reader++;
        }
    }

    free(mszReaders);

    return CHIP_NO_ERROR;
}

// Start scan on a given reader
CHIP_ERROR NFCCommissioningManagerImpl::ScanReader(uint16_t nfcShortId, char * readerName)
{
    SCARDHANDLE cardHandle;
    DWORD dwActiveProtocol;
    TagInstance * pTagInstance = nullptr;

    // Before launching a new scan of a reader, we should discard all the saved instances using this readerName
    DeleteAllTagInstancesUsingReaderName(readerName);

    long result = SCardConnect(hPcscContext, readerName, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &cardHandle,
                               &dwActiveProtocol);
    switch (result)
    {
    case SCARD_S_SUCCESS:
        switch (dwActiveProtocol)
        {
        case SCARD_PROTOCOL_T0:
            pioSendPci = *SCARD_PCI_T0;
            break;

        case SCARD_PROTOCOL_T1:
            pioSendPci = *SCARD_PCI_T1;
            break;
        }

        // Check if we already have this couple (readerName, cardHandle)
        pTagInstance = SearchTagInstanceFromReaderNameAndCardHandle(readerName, cardHandle);
        if (pTagInstance != nullptr)
        {
            // This couple (readerName, cardHandle) is already known: Nothing else to do
        }
        else
        {
            // This couple (readerName, cardHandle) is not known yet: Create a new TagInstance
            TagInstance * newTagInstance =
                new TagInstance(mNFCBase, Transport::PeerAddress::NFC(nfcShortId), readerName, cardHandle);

            ReturnErrorOnFailure(newTagInstance->RetrieveDiscriminator());

            tagInstances.push_back(newTagInstance);
        }
        break;

    case SCARD_E_NO_SMARTCARD:
        ChipLogProgress(DeviceLayer, "No NFC Tag detected");
        break;

    default:
        // An error happened
        ChipLogError(DeviceLayer, "SCardConnect failed with error 0x" ChipLogFormatX64, ChipLogValueX64(result));
        break;
    }

    return CHIP_NO_ERROR;
}

// Function to search for a TagInstance based on readerName and cardHandle
TagInstance * NFCCommissioningManagerImpl::SearchTagInstanceFromReaderNameAndCardHandle(const char * readerName,
                                                                                        SCARDHANDLE cardHandle)
{
    for (auto & instance : tagInstances)
    {
        if (strcmp(instance->GetReaderName(), readerName) == 0 && instance->GetCardHandle() == cardHandle)
        {
            return instance;
        }
    }
    return nullptr; // Return nullptr if not found
}

// Function to search for a TagInstance based on discriminator
TagInstance * NFCCommissioningManagerImpl::SearchTagInstanceFromDiscriminator(uint16_t discriminator)
{
    for (auto & instance : tagInstances)
    {
        if (instance->GetDiscriminator() == discriminator)
        {
            return instance;
        }
    }
    return nullptr; // Return nullptr if not found
}

// Delete all the TagInstance(s) using the given readerName
void NFCCommissioningManagerImpl::DeleteAllTagInstancesUsingReaderName(const char * readerName)
{
    if ((pLastTagInstanceUsed != nullptr) && (strcmp(pLastTagInstanceUsed->GetReaderName(), readerName) == 0))
    {
        pLastTagInstanceUsed = nullptr;
    }

    for (auto it = tagInstances.begin(); it != tagInstances.end();)
    {
        if (strcmp((*it)->GetReaderName(), readerName) == 0)
        {
            delete *it;                  // Free the memory allocated for the TagInstance
            it = tagInstances.erase(it); // Erase the item from the vector and get the next iterator
        }
        else
        {
            ++it;
        }
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
