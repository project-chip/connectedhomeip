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
 *          for the Android platforms.
 */

#pragma once

#include <nfc/NfcApplicationDelegate.h>

#include <platform/internal/NFCCommissioningManager.h>

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <lib/support/Span.h>
#include <mutex>
#include <queue>
#include <thread>
#include <winscard.h>

#if CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING

namespace chip {
namespace DeviceLayer {
namespace Internal {

class TagInstance;

// Message to send to an NFC Tag.
class NFCMessage
{
private:
    // Pointer to the NFC Tag instance to communicate with
    std::shared_ptr<TagInstance> mTagInstance;

    // Data to send to the NFC Tag
    chip::ByteSpan mDataToSend;

    // Dynamically allocated buffer to store the duplicated message data
    std::unique_ptr<uint8_t[]> mDataToSendBuffer;

    bool mIsMessageValid = false;

public:
    // Constructor
    NFCMessage(std::shared_ptr<TagInstance> instance, System::PacketBufferHandle && msgBuf) : mTagInstance(std::move(instance))
    {
        // Duplicate the data from the PacketBufferHandle
        size_t dataSize = msgBuf->DataLength();
        mDataToSendBuffer.reset(new (std::nothrow) uint8_t[dataSize]);

        if (mDataToSendBuffer != nullptr)
        {
            std::memcpy(mDataToSendBuffer.get(), msgBuf->Start(), dataSize);

            // Initialize mDataToSend ByteSpan to point to the duplicated buffer
            mDataToSend = chip::ByteSpan(mDataToSendBuffer.get(), dataSize);

            mIsMessageValid = true;
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to allocate memory for NFCMessage");
            mIsMessageValid = false;
        }
    }

    // Move Constructor
    NFCMessage(NFCMessage && other) noexcept :
        mTagInstance(other.mTagInstance), mDataToSend(other.mDataToSend), mDataToSendBuffer(std::move(other.mDataToSendBuffer)),
        mIsMessageValid(other.mIsMessageValid)
    {
        other.mTagInstance    = nullptr;
        other.mDataToSend     = chip::ByteSpan();
        other.mIsMessageValid = false;
    }

    // Move Assignment Operator
    NFCMessage & operator=(NFCMessage && other) noexcept
    {
        if (this != &other)
        {
            mTagInstance      = other.mTagInstance;
            mDataToSend       = other.mDataToSend;
            mDataToSendBuffer = std::move(other.mDataToSendBuffer);
            mIsMessageValid   = other.mIsMessageValid;

            other.mTagInstance    = nullptr;
            other.mDataToSend     = chip::ByteSpan();
            other.mIsMessageValid = false;
        }
        return *this;
    }

    // Deleted Copy Constructor
    NFCMessage(const NFCMessage &) = delete;

    // Deleted Copy Assignment Operator
    NFCMessage & operator=(const NFCMessage &) = delete;

    // Destructor
    ~NFCMessage() = default;

    // Get the TagInstance
    std::shared_ptr<TagInstance> GetTagInstance() { return mTagInstance; }

    // Get the data to send
    chip::ByteSpan GetDataToSend() { return mDataToSend; }

    // Check if the message is valid
    bool IsMessageValid() const { return mIsMessageValid; }
};

/**
 * Concrete implementation of the NFCCommissioningManagerImpl singleton object for the Linux platforms.
 */
class NFCCommissioningManagerImpl final : public NFCCommissioningManager, private Nfc::NfcApplicationDelegate
{
    // Allow the NFCCommissioningManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend NFCCommissioningManager;

public:
    // ===== Members that implement virtual methods on NfcApplicationDelegate.

    void SetNFCBase(Transport::NFCBase * nfcBase) override;

    bool CanSendToPeer(const Transport::PeerAddress & address) override;

    CHIP_ERROR SendToNfcTag(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override;

private:
    // ===== Members that implement the NFCCommissioningManager internal interface.

    CHIP_ERROR _Init();
    void _Shutdown();

    // ===== Members for internal use by the following friends.

    friend NFCCommissioningManager & NFCCommissioningMgr();
    friend NFCCommissioningManagerImpl & NFCCommissioningMgrImpl();

    static NFCCommissioningManagerImpl sInstance;

    void EraseAllTagInstancesUsingReaderName(const char * readerName);
    std::shared_ptr<TagInstance> SearchTagInstanceFromReaderNameAndCardHandle(const char * readerName, SCARDHANDLE cardHandle);
    std::shared_ptr<TagInstance> SearchTagInstanceFromDiscriminator(uint16_t discriminator);

    CHIP_ERROR ScanAllReaders(uint16_t nfcShortId);
    CHIP_ERROR ScanReader(uint16_t nfcShortId, char * readerName);

    Transport::NFCBase * mNFCBase = nullptr;

    // Thread and synchronization primitives
    std::thread mNfcThread;
    std::queue<std::unique_ptr<NFCMessage>> mMessageQueue;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCondition;
    std::atomic<bool> mThreadRunning;

    // Private methods
    void NfcThreadMain();
    void EnqueueMessage(std::unique_ptr<NFCMessage> message);
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

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
