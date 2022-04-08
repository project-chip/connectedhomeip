/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <functional>

#include <inet/TCPEndPoint.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>

#include <nlbyteorder.h>
#include <queue>

namespace chip {
namespace Test {

class IOContext
{
public:
    IOContext() {}

    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init();

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    /// Perform a single short IO Loop
    void DriveIO();

    /// DriveIO until the specified number of milliseconds has passed or until
    /// completionFunction returns true
    void DriveIOUntil(System::Clock::Timeout maxWait, std::function<bool(void)> completionFunction);

    System::Layer & GetSystemLayer() { return *mSystemLayer; }
    Inet::EndPointManager<Inet::TCPEndPoint> * GetTCPEndPointManager() { return mTCPEndPointManager; }
    Inet::EndPointManager<Inet::UDPEndPoint> * GetUDPEndPointManager() { return mUDPEndPointManager; }

private:
    System::Layer * mSystemLayer                                   = nullptr;
    Inet::EndPointManager<Inet::TCPEndPoint> * mTCPEndPointManager = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * mUDPEndPointManager = nullptr;
};

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char *) { return CHIP_NO_ERROR; }

    /*
     * For unit-tests that simulate end-to-end transmission and reception of messages in loopback mode,
     * this mode better replicates a real-functioning stack that correctly handles the processing
     * of a transmitted message as an asynchronous, bottom half handler dispatched after the current execution context has
     * completed. This is achieved using SystemLayer::ScheduleWork.
     */
    void EnableAsyncDispatch(System::Layer * aSystemLayer)
    {
        mSystemLayer          = aSystemLayer;
        mAsyncMessageDispatch = true;
    }

    /*
     * Reset the dispatch back to a model that synchronously dispatches received messages up the stack.
     *
     * NOTE: This results in highly atypical/complex call stacks that are not representative of what happens on real
     * devices and can cause subtle and complex bugs to either appear or get masked in the system. Where possible, please
     * use this sparingly!
     */
    void DisableAsyncDispatch() { mAsyncMessageDispatch = false; }

    bool HasPendingMessages() { return !mPendingMessageQueue.empty(); }

    static void OnMessageReceived(System::Layer * aSystemLayer, void * aAppState)
    {
        LoopbackTransport * _this = static_cast<LoopbackTransport *>(aAppState);

        while (!_this->mPendingMessageQueue.empty())
        {
            auto item = std::move(_this->mPendingMessageQueue.front());
            _this->mPendingMessageQueue.pop();
            _this->HandleMessageReceived(item.mDestinationAddress, std::move(item.mPendingMessage));
        }
    }

    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override
    {
        ReturnErrorOnFailure(mMessageSendError);
        mSentMessageCount++;

        if (mNumMessagesToDrop == 0)
        {
            System::PacketBufferHandle receivedMessage = msgBuf.CloneData();

            if (mAsyncMessageDispatch)
            {
                mPendingMessageQueue.push(PendingMessageItem(address, std::move(receivedMessage)));
                mSystemLayer->ScheduleWork(OnMessageReceived, this);
            }
            else
            {
                HandleMessageReceived(address, std::move(receivedMessage));
            }
        }
        else
        {
            mNumMessagesToDrop--;
            mDroppedMessageCount++;
            MessageDropped();
        }

        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const Transport::PeerAddress & address) override { return true; }

    void Reset()
    {
        mNumMessagesToDrop   = 0;
        mDroppedMessageCount = 0;
        mSentMessageCount    = 0;
        mMessageSendError    = CHIP_NO_ERROR;
    }

    struct PendingMessageItem
    {
        PendingMessageItem(const Transport::PeerAddress destinationAddress, System::PacketBufferHandle && pendingMessage) :
            mDestinationAddress(destinationAddress), mPendingMessage(std::move(pendingMessage))
        {}

        const Transport::PeerAddress mDestinationAddress;
        System::PacketBufferHandle mPendingMessage;
    };

    // Hook for subclasses to perform custom logic on message drops.
    virtual void MessageDropped() {}

    System::Layer * mSystemLayer = nullptr;
    bool mAsyncMessageDispatch   = false;
    std::queue<PendingMessageItem> mPendingMessageQueue;
    Transport::PeerAddress mTxAddress;
    uint32_t mNumMessagesToDrop   = 0;
    uint32_t mDroppedMessageCount = 0;
    uint32_t mSentMessageCount    = 0;
    CHIP_ERROR mMessageSendError  = CHIP_NO_ERROR;
};

} // namespace Test
} // namespace chip
