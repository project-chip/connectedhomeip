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
    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init();

    // Shutdown all layers, finalize operations
    void Shutdown();

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

class LoopbackTransportDelegate
{
public:
    virtual ~LoopbackTransportDelegate() {}

    // Called by the loopback transport when it drops one of a configurable number of messages (mDroppedMessageCount) after a
    // configurable allowed number of messages (mNumMessagesToAllowBeforeDropping)
    virtual void OnMessageDropped() {}
};

class LoopbackTransport : public Transport::Base
{
public:
    void InitLoopbackTransport(System::Layer * systemLayer) { mSystemLayer = systemLayer; }
    void ShutdownLoopbackTransport()
    {
        // Make sure no one left packets hanging out that they thought got
        // delivered but actually didn't.
        VerifyOrDie(mPendingMessageQueue.empty());
    }

    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char *) { return CHIP_NO_ERROR; }

    bool HasPendingMessages() { return !mPendingMessageQueue.empty(); }

    void SetLoopbackTransportDelegate(LoopbackTransportDelegate * delegate) { mDelegate = delegate; }

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

    static constexpr uint32_t kUnlimitedMessageCount = std::numeric_limits<uint32_t>::max();

    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override
    {
        ReturnErrorOnFailure(mMessageSendError);
        mSentMessageCount++;
        bool dropMessage = false;
        if (mNumMessagesToAllowBeforeDropping > 0)
        {
            --mNumMessagesToAllowBeforeDropping;
        }
        else if (mNumMessagesToDrop > 0)
        {
            dropMessage = true;
            --mNumMessagesToDrop;
        }

        if (dropMessage)
        {
            ChipLogProgress(Test, "Dropping message...");
            mDroppedMessageCount++;
            if (mDelegate != nullptr)
            {
                mDelegate->OnMessageDropped();
            }
        }
        else
        {
            System::PacketBufferHandle receivedMessage = msgBuf.CloneData();
            mPendingMessageQueue.push(PendingMessageItem(address, std::move(receivedMessage)));
            mSystemLayer->ScheduleWork(OnMessageReceived, this);
        }

        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const Transport::PeerAddress & address) override { return true; }

    void Reset()
    {
        mNumMessagesToDrop                = 0;
        mDroppedMessageCount              = 0;
        mSentMessageCount                 = 0;
        mNumMessagesToAllowBeforeDropping = 0;
        mMessageSendError                 = CHIP_NO_ERROR;
    }

    struct PendingMessageItem
    {
        PendingMessageItem(const Transport::PeerAddress destinationAddress, System::PacketBufferHandle && pendingMessage) :
            mDestinationAddress(destinationAddress), mPendingMessage(std::move(pendingMessage))
        {}

        const Transport::PeerAddress mDestinationAddress;
        System::PacketBufferHandle mPendingMessage;
    };

    System::Layer * mSystemLayer = nullptr;
    std::queue<PendingMessageItem> mPendingMessageQueue;
    Transport::PeerAddress mTxAddress;
    uint32_t mNumMessagesToDrop                = 0;
    uint32_t mDroppedMessageCount              = 0;
    uint32_t mSentMessageCount                 = 0;
    uint32_t mNumMessagesToAllowBeforeDropping = 0;
    CHIP_ERROR mMessageSendError               = CHIP_NO_ERROR;
    LoopbackTransportDelegate * mDelegate      = nullptr;
};

} // namespace Test
} // namespace chip
