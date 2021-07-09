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

#include <core/CHIPError.h>
#include <inet/InetLayer.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>
#include <transport/raw/PeerAddress.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

namespace chip {
namespace Test {

class IOContext
{
public:
    IOContext() {}

    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init(nlTestSuite * suite);

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    /// Perform a single short IO Loop
    void DriveIO();

    /// DriveIO until the specified number of milliseconds has passed or until
    /// completionFunction returns true
    void DriveIOUntil(unsigned maxWaitMs, std::function<bool(void)> completionFunction);

    nlTestSuite * GetTestSuite() { return mSuite; }
    System::Layer & GetSystemLayer() { return *mSystemLayer; }
    Inet::InetLayer & GetInetLayer() { return *mInetLayer; }

private:
    nlTestSuite * mSuite         = nullptr;
    System::Layer * mSystemLayer = nullptr;
    Inet::InetLayer * mInetLayer = nullptr;
};

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char *) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf) override
    {
        ReturnErrorOnFailure(mMessageSendError);
        mSentMessageCount++;

        if (mNumMessagesToDrop == 0)
        {
            System::PacketBufferHandle receivedMessage = msgBuf.CloneData();
            HandleMessageReceived(address, std::move(receivedMessage));
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

    // Hook for subclasses to perform custom logic on message drops.
    virtual void MessageDropped() {}

    uint32_t mNumMessagesToDrop   = 0;
    uint32_t mDroppedMessageCount = 0;
    uint32_t mSentMessageCount    = 0;
    CHIP_ERROR mMessageSendError  = CHIP_NO_ERROR;
};

} // namespace Test
} // namespace chip
