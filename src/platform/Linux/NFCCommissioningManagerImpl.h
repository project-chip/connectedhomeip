/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for the Android platforms.
 */

#pragma once

#include <nfc/NfcApplicationDelegate.h>

#include <platform/internal/NFCCommissioningManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_COMMISSIONING

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NFCCommissioningManagerImpl singleton object for the Linux platforms.
 */
class NFCCommissioningManagerImpl final : public NFCCommissioningManager,
                             private Nfc::NfcApplicationDelegate
{
    // Allow the NFCCommissioningManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend NFCCommissioningManager;

public:
    CHIP_ERROR ConfigureNfc(uint32_t aNodeId, bool aIsCentral);

    CHIP_ERROR OnNfcTagResponse(System::PacketBufferHandle && buffer);
    CHIP_ERROR OnNfcTagError();

    // ===== Members that implement virtual methods on NfcApplicationDelegate.

    void SetNFCBase(Transport::NFCBase * nfcBase) override;

    CHIP_ERROR SelectMatterApplet(void);
    CHIP_ERROR SendToNfcTag(System::PacketBufferHandle && msgBuf) override;

    void SendChainedAPDUs(void);

private:
    // ===== Members that implement the NFCCommissioningManager internal interface.

    CHIP_ERROR _Init();

    // ===== Members for internal use by the following friends.

    friend NFCCommissioningManager & NFCCommissioningMgr();
    friend NFCCommissioningManagerImpl & NFCCommissioningMgrImpl();

    static NFCCommissioningManagerImpl sInstance;
    static void SendChainedAPDUs(intptr_t arg);

    CHIP_ERROR SendTransportAPDU(uint8_t * dataToSend,
                                 uint32_t dataToSendLength,
                                 bool isLastBlock,
                                 uint32_t totalLength,
                                 uint8_t * pRcvBuffer,
                                 uint32_t * pRcvLength);
    CHIP_ERROR GetResponse(uint8_t length,
                           uint8_t * pRcvBuffer,
                           uint32_t *pRcvLength);
    void ProcessAPDUResponse(void);
    CHIP_ERROR Transceive(const char * commandName,
                          uint8_t * pSendBuffer,
                          uint32_t sendBufferLength,
                          uint8_t * pRcvBuffer,
                          uint32_t *pRcvLength);

    void PrintSw1Sw2(uint8_t sw1, uint8_t sw2);
    void ProcessError(const char * msg);
    void NotifyResponse(uint8_t * response, uint32_t responseLen);

    void ResetChainedResponseBuffer(void);
    CHIP_ERROR AddDataToChainedResponseBuffer(uint8_t * data, int dataLen);

    Transport::NFCBase * mNFCBase = nullptr;

    // Buffer containing the message to send to the tag
    uint8_t mDataToSend[1280];
    uint32_t mDataToSendLength;

    // Buffer containing a single APDU command sent to the tag
    uint8_t mAPDUTxBuffer[256];
    // Buffer used to receive the response to an APDU command
    uint8_t mAPDURxBuffer[256];
    uint32_t mAPDUResponseLength;

    // Buffer storing the chained APDU messages received from the tag
    uint8_t mChainedResponseBuffer[1280];
    uint32_t mChainedResponseLength;
};

/**
 * Returns a reference to the public interface of the NFCCommissioningManager singleton object.
 *
 * Internal components should use this to access features of the NFCCommissioningManager object
 * that are common to all platforms.
 */
inline NFCCommissioningManager & NFCCommissioningMgr()
{
    return NFCCommissioningManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the NFCCommissioningManager singleton object.
 *
 * Internal components can use this to gain access to features of the NFCCommissioningManager
 * that are specific to the Linux platforms.
 */
inline NFCCommissioningManagerImpl & NFCCommissioningMgrImpl()
{
    return NFCCommissioningManagerImpl::sInstance;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_COMMISSIONING
