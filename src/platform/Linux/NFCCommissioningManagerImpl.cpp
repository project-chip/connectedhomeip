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

#include <winscard.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

using namespace chip;
using namespace ::nl;
using namespace ::chip::Nfc;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {} // namespace

NFCCommissioningManagerImpl NFCCommissioningManagerImpl::sInstance;

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

static SCARDHANDLE hCard = 0;
static SCARD_IO_REQUEST pioSendPci;

#define CHECK(f, rv)                                                                                                               \
    if (SCARD_S_SUCCESS != rv)                                                                                                     \
    {                                                                                                                              \
        ChipLogError(DeviceLayer, "%s : %s\n", f, pcsc_stringify_error(rv));                                                       \
        return CHIP_ERROR_INTERNAL;                                                                                                \
    }

// ===== start impl of NFCCommissioningManager internal interface, ref NFCCommissioningManager.h

CHIP_ERROR NFCCommissioningManagerImpl::_Init()
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::_Init()");

    return CHIP_NO_ERROR;
}

// ===== start implement virtual methods on NfcApplicationDelegate.

void NFCCommissioningManagerImpl::SetNFCBase(Transport::NFCBase * nfcBase)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::SetNFCBase()");
    mNFCBase = nfcBase;
}

CHIP_ERROR NFCCommissioningManagerImpl::ConnectToCard()
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::ConnectToCard()");

    long rv;
    SCARDCONTEXT hContext;
    LPTSTR mszReaders;
    DWORD dwReaders, dwActiveProtocol;
    CHIP_ERROR result;

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    CHECK("SCardEstablishContext", rv)

    rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
    CHECK("SCardListReaders", rv)

    mszReaders = (LPTSTR) calloc(dwReaders, sizeof(char));
    rv         = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
    CHECK("SCardListReaders", rv)

    ChipLogProgress(DeviceLayer, "reader name: %s\n", mszReaders);

    rv = SCardConnect(hContext, mszReaders, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
    switch (rv)
    {
    case SCARD_S_SUCCESS:
        ChipLogProgress(DeviceLayer, "hCard 0x" ChipLogFormatX64, ChipLogValueX64(hCard));

        switch (dwActiveProtocol)
        {
        case SCARD_PROTOCOL_T0:
            ChipLogProgress(DeviceLayer, "SCARD_PROTOCOL_T0");
            pioSendPci = *SCARD_PCI_T0;
            break;

        case SCARD_PROTOCOL_T1:
            ChipLogProgress(DeviceLayer, "SCARD_PROTOCOL_T1");
            pioSendPci = *SCARD_PCI_T1;
            break;
        }

        SelectMatterApplet();
        result = CHIP_NO_ERROR;
        break;

    case SCARD_E_NO_SMARTCARD:
        ChipLogProgress(DeviceLayer, "No NFC Tag detected");
        hCard  = 0;
        result = CHIP_NO_ERROR;
        break;

    default:
        // An error happened
        ChipLogError(DeviceLayer, "SCardConnect failed with error 0x" ChipLogFormatX64, ChipLogValueX64(rv));
        result = CHIP_ERROR_INTERNAL;
        break;
    }

    return result;
}

CHIP_ERROR NFCCommissioningManagerImpl::SelectMatterApplet(void)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::SelectMatterApplet()");

    if (hCard == 0)
    {
        CHIP_ERROR ret = ConnectToCard();
        VerifyOrReturnLogError(ret == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
        VerifyOrReturnLogError(hCard != 0, CHIP_ERROR_INTERNAL);
    }

    BYTE dataReceived[10];
    DWORD receivedLength        = sizeof(dataReceived);
    BYTE select_matter_applet[] = { 0x00, 0xA4, 0x04, 0x00, 0x08, 0xA0, 0x00, 0x00, 0x07, 0x08, 0x02, 0x00, 0x12, // AID
                                    0x00 };

    LONG result =
        SCardTransmit(hCard, &pioSendPci, select_matter_applet, sizeof(select_matter_applet), NULL, dataReceived, &receivedLength);
    ChipLogProgress(DeviceLayer, "SCardTransmit result: 0x" ChipLogFormatX64, ChipLogValueX64(result));

    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCCommissioningManagerImpl::SendToNfcTag(System::PacketBufferHandle && msgBuf)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::SendToNfcTag()");

    mDataToSendLength = (uint32_t) msgBuf->DataLength();
    VerifyOrReturnLogError(mDataToSendLength <= sizeof(mDataToSend), CHIP_ERROR_INTERNAL);

    // Data to send is copied to a local buffer
    memcpy(&(mDataToSend[0]), msgBuf->Start(), mDataToSendLength);

    // SendChainedAPDUs will be executed asynchronously
    PlatformMgr().ScheduleWork(SendChainedAPDUs, 0);

    return CHIP_NO_ERROR;
}

void NFCCommissioningManagerImpl::SendChainedAPDUs(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "NFCCommissioningManagerImpl::SendChainedAPDUs");
    sInstance.SendChainedAPDUs();
}

// Send Transport command via simple APDUs.
// If the data doesn't fit in a single APDU, it will be splitted in
// several chained APDUs.
// If the response doesn't fit in 255 bytes, it will be chained also.
// When the tag's response is fully received, it will be transmitted
// by OnNfcTagResponse() callback.
void NFCCommissioningManagerImpl::SendChainedAPDUs(void)
{
    CHIP_ERROR res;
    uint32_t totalLength         = mDataToSendLength;
    uint32_t nbrOfBytesRemaining = mDataToSendLength;
    uint8_t * pNextDataToSend    = mDataToSend;

    while (nbrOfBytesRemaining > 0)
    {

        // Size of the next APDU
        uint32_t nbrOfBytesToSend = MIN(nbrOfBytesRemaining, TYPE4_SIMPLE_APDU_MAX_TX_SIZE);

        bool isLastBlock = (nbrOfBytesToSend == nbrOfBytesRemaining);

        mAPDUResponseLength = sizeof(mAPDURxBuffer); // Initialized with with mAPDURxBuffer size to indicate to the low level driver
                                                     // the capacity of the RX buffer
        res = SendTransportAPDU(pNextDataToSend, nbrOfBytesToSend, isLastBlock, totalLength, mAPDURxBuffer, &mAPDUResponseLength);
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
            if (((sw1 == ((uint8_t) 0x90)) && (sw2 == 0x00)) || (sw1 == ((uint8_t) 0x61)))
            {
                // Response will be processed outside of the while loop
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
            if ((sw1 == ((uint8_t) 0x90)) && (sw2 == 0x00))
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
void NFCCommissioningManagerImpl::ProcessAPDUResponse(void)
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

    if ((sw1 == ((uint8_t) 0x90)) && (sw2 == 0x00))
    {
        // Response fits in a single block.
        // Drop the 2 status bytes and return it
        NotifyResponse(mAPDURxBuffer, mAPDUResponseLength - 2);
        return;
    }
    else if (sw1 == ((uint8_t) 0x61))
    {
        // SW1=0x61 indicates a chained response. it means that the response is too big to be transmitted in a single packet.

        // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
        res = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
        if (res != CHIP_NO_ERROR)
        {
            ProcessError("Too many data!");
            return;
        }

        // SW2 indicates the size of the data in the next response packet.
        // The next response packet can be read thanks to a call to getResponse() command.
        while (sw1 == 0x61)
        {
            // If SW2 is 0x00 or if it is higher than TYPE4_SIMPLE_APDU_MAX_RX_SIZE, we clamp it to TYPE4_SIMPLE_APDU_MAX_RX_SIZE.
            uint8_t nextBlockLength =
                ((sw2 == 0x00) || ((int) sw2 > TYPE4_SIMPLE_APDU_MAX_RX_SIZE)) ? (uint8_t) TYPE4_SIMPLE_APDU_MAX_RX_SIZE : sw2;

            mAPDUResponseLength = sizeof(mAPDURxBuffer); // Initialized with with mAPDURxBuffer size to indicate to the low level
                                                         // driver the capacity of the RX buffer
            // Response will be written into mAPDURxBuffer
            res = GetResponse(nextBlockLength, mAPDURxBuffer, &mAPDUResponseLength);
            if ((res != CHIP_NO_ERROR) || (mAPDUResponseLength < 2))
            {
                ProcessError("Invalid NFC Type4 response");
                return;
            }

            sw1 = mAPDURxBuffer[mAPDUResponseLength - 2];
            sw2 = mAPDURxBuffer[mAPDUResponseLength - 1];

            if ((sw1 == ((uint8_t) 0x90)) && (sw2 == 0x00))
            {
                ChipLogProgress(DeviceLayer, "Chained response received successfully");

                // Put the received bytes (without the 2 status bytes) into the ChainedResponseBuffer
                res = AddDataToChainedResponseBuffer(mAPDURxBuffer, mAPDUResponseLength - 2);
                if (res != CHIP_NO_ERROR)
                {
                    ProcessError("Too many data!");
                    return;
                }
            }
            else if (sw1 == (uint8_t) 0x61)
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

void NFCCommissioningManagerImpl::PrintSw1Sw2(uint8_t sw1, uint8_t sw2)
{
    ChipLogProgress(DeviceLayer, "SW1=0x%x SW2=0x%x", sw1, sw2);
}

void NFCCommissioningManagerImpl::ProcessError(const char * msg)
{
    ChipLogError(DeviceLayer, "%s", msg);
    OnNfcTagError();
}

void NFCCommissioningManagerImpl::NotifyResponse(uint8_t * response, uint32_t responseLen)
{

    System::PacketBufferHandle buffer =
        System::PacketBufferHandle::NewWithData(reinterpret_cast<const uint8_t *>(response), static_cast<size_t>(responseLen));
    OnNfcTagResponse(std::move(buffer));
}

void NFCCommissioningManagerImpl::ResetChainedResponseBuffer(void)
{
    ChipLogProgress(DeviceLayer, "ResetChainedResponseBuffer()");
    mChainedResponseLength = 0;
}

CHIP_ERROR NFCCommissioningManagerImpl::AddDataToChainedResponseBuffer(uint8_t * data, int dataLen)
{

    ChipLogProgress(DeviceLayer, "Add %d bytes to chainedResponseBuffer", dataLen);

    // Check that mChainedResponseBuffer will not overflow
    VerifyOrReturnLogError((mChainedResponseLength + dataLen) <= sizeof(mChainedResponseBuffer), CHIP_ERROR_MESSAGE_TOO_LONG);

    memcpy(&(mChainedResponseBuffer[mChainedResponseLength]), data, dataLen);

    mChainedResponseLength += dataLen;

    return CHIP_NO_ERROR;
}

/////////////////////////////////////////////////////////////////

CHIP_ERROR NFCCommissioningManagerImpl::SendTransportAPDU(uint8_t * dataToSend, uint32_t dataToSendLength, bool isLastBlock,
                                                          uint32_t totalLength, uint8_t * pRcvBuffer, uint32_t * pRcvLength)
{

    VerifyOrReturnLogError(dataToSendLength <= sizeof(mAPDUTxBuffer), CHIP_ERROR_INTERNAL);

    mAPDUTxBuffer[0] = isLastBlock ? (uint8_t) 0x80 : (uint8_t) 0x90; // CLA
    mAPDUTxBuffer[1] = 0x20;                                          // INS
    mAPDUTxBuffer[2] = (uint8_t) ((totalLength >> 8) & 0xFF);         // P1 (contains the totalLength's MSB)
    mAPDUTxBuffer[3] = (uint8_t) (totalLength & 0xFF);                // P2 (contains the totalLength's LSB)
    mAPDUTxBuffer[4] = (uint8_t) dataToSendLength;                    // Lc
    memcpy(&(mAPDUTxBuffer[5]), dataToSend, dataToSendLength);
    mAPDUTxBuffer[5 + dataToSendLength] = (uint8_t) TYPE4_SIMPLE_APDU_MAX_RX_SIZE; // Le

    uint32_t apduLength = 6 + dataToSendLength;

    CHIP_ERROR result = Transceive("SendTransportAPDU", mAPDUTxBuffer, apduLength, pRcvBuffer, pRcvLength);

    return result;
}

CHIP_ERROR NFCCommissioningManagerImpl::GetResponse(uint8_t length, uint8_t * pRcvBuffer, uint32_t * pRcvLength)
{
    uint8_t frame[5];

    frame[0] = 0x00;           // CLA
    frame[1] = (uint8_t) 0xC0; // INS
    frame[2] = 0x00;           // P1
    frame[3] = 0x00;           // P2
    frame[4] = length;         // Le

    CHIP_ERROR result = Transceive("GetResponse", frame, sizeof(frame), pRcvBuffer, pRcvLength);

    return result;
}

CHIP_ERROR NFCCommissioningManagerImpl::Transceive(const char * commandName, uint8_t * pSendBuffer, uint32_t sendBufferLength,
                                                   uint8_t * pRcvBuffer, uint32_t * pRcvLength)
{
    CHIP_ERROR ret;

    if (hCard == 0)
    {
        ret = ConnectToCard();
        VerifyOrReturnLogError(ret == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
        VerifyOrReturnLogError(hCard != 0, CHIP_ERROR_INTERNAL);
    }

    // Use a local 'dwRecvLength' variable to avoid cast of pRcvLength from 'uint32_t *' to 'DWORD *'.
    // Before the transceive action, those variables contain contain the size of pRcvBuffer buffer.
    // After the transceive, they will contain the length of the received data.
    DWORD dwRecvLength = *pRcvLength;

    LONG result = SCardTransmit(hCard, &pioSendPci, pSendBuffer, sendBufferLength, NULL, pRcvBuffer, &dwRecvLength);

    if ((result == SCARD_S_SUCCESS) && (dwRecvLength >= 2))
    {
        // Copy the length of received data to pRcvLength
        *pRcvLength = (uint32_t) dwRecvLength;

        ChipLogProgress(DeviceLayer, "Received length: %d", *pRcvLength);
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

CHIP_ERROR NFCCommissioningManagerImpl::OnNfcTagResponse(System::PacketBufferHandle && buffer)
{
    mNFCBase->OnNfcTagResponse(std::move(buffer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR NFCCommissioningManagerImpl::OnNfcTagError()
{
    mNFCBase->OnNfcTagError();
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
