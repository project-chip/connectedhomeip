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

#include <transport/raw/NfcApplicationDelegate.h>

#include <lib/core/Global.h>
#include <platform/internal/NFCCommissioningManager.h>

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <functional>
#include <lib/support/Span.h>
#include <mutex>
#include <queue>
#include <thread>
#ifdef __APPLE__
#include <PCSC/winscard.h>
#else
#include <winscard.h>
#endif

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

    // Dynamically allocated buffer to store the duplicated message data
    std::unique_ptr<uint8_t[]> mDataToSendBuffer;

    // Size of the duplicated message data
    size_t mDataToSendSize = 0;

    bool mIsMessageValid = false;

public:
    // Constructor
    NFCMessage(std::shared_ptr<TagInstance> instance, System::PacketBufferHandle && msgBuf) : mTagInstance(std::move(instance))
    {
        // Duplicate the data from the PacketBufferHandle
        mDataToSendSize = msgBuf->DataLength();
        mDataToSendBuffer.reset(new (std::nothrow) uint8_t[mDataToSendSize]);

        if (mDataToSendBuffer != nullptr)
        {
            std::memcpy(mDataToSendBuffer.get(), msgBuf->Start(), mDataToSendSize);
            mIsMessageValid = true;
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to allocate memory for NFCMessage");
            mIsMessageValid = false;
        }
    }

    // Move Constructor
    NFCMessage(NFCMessage &&) noexcept = default;

    // Move Assignment Operator
    NFCMessage & operator=(NFCMessage &&) noexcept = default;

    // Deleted Copy Constructor
    NFCMessage(const NFCMessage &) = delete;

    // Deleted Copy Assignment Operator
    NFCMessage & operator=(const NFCMessage &) = delete;

    // Destructor
    ~NFCMessage() = default;

    // Get the TagInstance
    std::shared_ptr<TagInstance> GetTagInstance() const { return mTagInstance; }

    // Get the data to send
    chip::ByteSpan GetDataToSend() const { return chip::ByteSpan(mDataToSendBuffer.get(), mDataToSendSize); }

    // Check if the message is valid
    bool IsMessageValid() const { return mIsMessageValid; }
};

// Context used to synchronize a synchronous work item with the caller thread.
struct SyncWorkContext
{
    std::mutex mutex;
    std::condition_variable cv;
    bool done         = false;
    CHIP_ERROR result = CHIP_NO_ERROR;
};

// Work types handled by the processing thread
enum class NfcWorkType
{
    kScan, // Scan every NFC Readers
    kSend, // Send NFC commands to a Card
};

// Work item processed by the NFC worker thread.
struct NfcWorkItem
{
    NfcWorkType type;
    Nfc::NFCTag::Identifier targetIdentifier;
    std::unique_ptr<NFCMessage> message;

    // If not null, the worker thread must signal completion of this work item
    // through this synchronization context.
    SyncWorkContext * syncCtx = nullptr;
};

/**
 * Concrete implementation of the NFCCommissioningManagerImpl singleton object for the Linux platforms.
 */
class NFCCommissioningManagerImpl final : public NFCCommissioningManager,
                                          public Nfc::NFCReaderTransport,
                                          private Nfc::NfcApplicationDelegate
{
    // Allow the NFCCommissioningManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend NFCCommissioningManager;

public:
    // ===== Members that implement virtual methods on NfcApplicationDelegate.

    void SetNFCBase(Transport::NFCBase * nfcBase) override;

    bool CanSendToPeer(const Transport::PeerAddress & address) override;

    CHIP_ERROR SendToNfcTag(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override;

    // ===== Members that implement virtual methods on NFCReaderTransport.

    void SetDelegate(Nfc::NFCReaderTransportDelegate * delegate) override;

    CHIP_ERROR StartDiscoveringTagMatchingAddress(const Nfc::NFCTag::Identifier & tagIdentifier) override;

    CHIP_ERROR StopDiscoveringTags() override;

    bool FindTagMatchingIdentifier(const Nfc::NFCTag::Identifier & tagIdentifier) override;

    CHIP_ERROR SendMessage(System::PacketBufferHandle && message, const Nfc::NFCTag::Identifier & tagIdentifier,
                           std::function<void(System::PacketBufferHandle &&, CHIP_ERROR)> onResponse) override
    {
        static_cast<void>(message);
        static_cast<void>(tagIdentifier);
        static_cast<void>(onResponse);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    // ===== Members that implement the NFCCommissioningManager internal interface.

    CHIP_ERROR _Init();
    void _Shutdown();
    Nfc::NFCReaderTransport * _GetNFCReaderTransport() const { return const_cast<NFCCommissioningManagerImpl *>(this); }
    void _SetNFCReaderTransport(Nfc::NFCReaderTransport * readerTransport) {}

    // ===== Members for internal use by the following friends.

    friend NFCCommissioningManager & NFCCommissioningMgr();
    friend NFCCommissioningManagerImpl & NFCCommissioningMgrImpl();

    static Global<NFCCommissioningManagerImpl> sInstance;

    void EraseAllTagInstancesUsingReaderName(const char * readerName);
    std::shared_ptr<TagInstance> SearchTagInstanceFromReaderNameAndCardHandle(const char * readerName, SCARDHANDLE cardHandle);
    std::shared_ptr<TagInstance> SearchTagInstanceFromDiscriminator(uint16_t discriminator);

    // Start scan on all available readers and scan for NFC Tags.
    CHIP_ERROR ScanAllReaders(void);

    // Start scan on a given reader
    CHIP_ERROR ScanReader(char * readerName);

    // Protected by mNFCBaseMutex
    Transport::NFCBase * mNFCBase = nullptr;
    std::mutex mNFCBaseMutex;

    // PC/SC context owned by the NFC worker thread.
    SCARDCONTEXT mPcscContext = 0;

    // Cache of discovered NFC tag instances.
    std::shared_ptr<TagInstance> mLastTagInstanceUsed;
    std::vector<std::shared_ptr<TagInstance>> mTagInstances;
    std::mutex mTagInstancesMutex;

    // NFC worker Thread and synchronization primitives
    std::thread mNfcWorkerThread;
    std::queue<NfcWorkItem> mWorkQueue;
    std::mutex mWorkQueueMutex;
    std::condition_variable mWorkQueueCondition;
    std::atomic<bool> mNfcWorkerThreadRunning{ false };
    bool mShuttingDown = false;

    // Mutex protecting the worker thread startup sequence
    std::mutex mWorkerStartMutex;

    // Synchronization primitives used to wait until the worker thread has finished
    // its initialization, including PC/SC context creation.
    std::mutex mWorkerInitMutex;
    std::condition_variable mWorkerInitCondition;
    bool mWorkerInitCompleted    = false;
    CHIP_ERROR mWorkerInitResult = CHIP_ERROR_INCORRECT_STATE;

    // NFCReaderTransport state
    Nfc::NFCReaderTransportDelegate * mDelegate = nullptr;
    std::mutex mDelegateMutex;

    // Private methods
    void NfcWorkerThreadMain();
    CHIP_ERROR EnqueueWork(NfcWorkItem && item);

    CHIP_ERROR EnsureWorkerThreadStarted();
    CHIP_ERROR RunSyncOnWorker(NfcWorkItem && item);
    Transport::NFCBase * GetNFCBase();

    static void DispatchTagDiscovery(intptr_t arg);
};

/**
 * Returns a reference to the public interface of the NFCCommissioningManager singleton object.
 *
 * Internal components should use this to access features of the NFCCommissioningManager object
 * that are common to all platforms.
 */
inline NFCCommissioningManager & NFCCommissioningMgr()
{
    return NFCCommissioningManagerImpl::sInstance.get();
}

/**
 * Returns the platform-specific implementation of the NFCCommissioningManager singleton object.
 *
 * Internal components can use this to gain access to features of the NFCCommissioningManager
 * that are specific to the Linux platforms.
 */
inline NFCCommissioningManagerImpl & NFCCommissioningMgrImpl()
{
    return NFCCommissioningManagerImpl::sInstance.get();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_NFC_BASED_COMMISSIONING
