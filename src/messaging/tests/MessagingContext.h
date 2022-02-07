/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemClock.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlunit-test.h>

namespace chip {
namespace Test {

/**
 * @brief
 *  Test contexts that use Platform::Memory and might call Free() on destruction can inherit from this class and call its Init().
 *  Platform::MemoryShutdown() will then be called after the subclasses' destructor.
 */
class PlatformMemoryUser
{
public:
    PlatformMemoryUser() : mInitialized(false) {}
    ~PlatformMemoryUser()
    {
        if (mInitialized)
        {
            chip::Platform::MemoryShutdown();
        }
    }
    CHIP_ERROR Init()
    {
        CHIP_ERROR status = CHIP_NO_ERROR;
        if (!mInitialized)
        {
            status       = chip::Platform::MemoryInit();
            mInitialized = (status == CHIP_NO_ERROR);
        }
        return status;
    }

private:
    bool mInitialized;
};

/**
 * @brief The context of test cases for messaging layer. It wil initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class MessagingContext : public PlatformMemoryUser
{
public:
    MessagingContext() :
        mInitialized(false), mAliceAddress(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT + 1)),
        mBobAddress(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT)), mPairingAliceToBob(GetBobKeyId(), GetAliceKeyId()),
        mPairingBobToAlice(GetAliceKeyId(), GetBobKeyId())
    {}
    ~MessagingContext() { VerifyOrDie(mInitialized == false); }

    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init(TransportMgrBase * transport, IOContext * io);

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    // Initialize from an existing messaging context.  Useful if we want to
    // share some state (like the transport).
    CHIP_ERROR InitFromExisting(const MessagingContext & existing);

    // The shutdown method to use if using InitFromExisting.  Must pass in the
    // same existing context as was passed to InitFromExisting.
    CHIP_ERROR ShutdownAndRestoreExisting(MessagingContext & existing);

    static Inet::IPAddress GetAddress()
    {
        Inet::IPAddress addr;
        Inet::IPAddress::FromString("::1", addr);
        return addr;
    }
    NodeId GetBobNodeId() const { return mBobNodeId; }
    NodeId GetAliceNodeId() const { return mAliceNodeId; }

    void SetBobNodeId(NodeId nodeId) { mBobNodeId = nodeId; }
    void SetAliceNodeId(NodeId nodeId) { mAliceNodeId = nodeId; }

    uint16_t GetBobKeyId() const { return mBobKeyId; }
    uint16_t GetAliceKeyId() const { return mAliceKeyId; }
    GroupId GetFriendsGroupId() const { return mFriendsGroupId; }

    void SetBobKeyId(uint16_t id) { mBobKeyId = id; }
    void SetAliceKeyId(uint16_t id) { mAliceKeyId = id; }

    FabricIndex GetFabricIndex() const { return mSrcFabricIndex; }
    void SetFabricIndex(FabricIndex id)
    {
        mSrcFabricIndex  = id;
        mDestFabricIndex = id;
    }

    SessionManager & GetSecureSessionManager() { return mSessionManager; }
    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeManager; }
    secure_channel::MessageCounterManager & GetMessageCounterManager() { return mMessageCounterManager; }

    CHIP_ERROR CreateSessionBobToAlice();
    CHIP_ERROR CreateSessionAliceToBob();
    CHIP_ERROR CreateSessionBobToFriends();

    void ExpireSessionBobToAlice();
    void ExpireSessionAliceToBob();
    void ExpireSessionBobToFriends();

    SessionHandle GetSessionBobToAlice();
    SessionHandle GetSessionAliceToBob();
    SessionHandle GetSessionBobToFriends();

    Messaging::ExchangeContext * NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate);

    Messaging::ExchangeContext * NewExchangeToAlice(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewExchangeToBob(Messaging::ExchangeDelegate * delegate);

    System::Layer & GetSystemLayer() { return mIOContext->GetSystemLayer(); }

private:
    bool mInitialized;
    SessionManager mSessionManager;
    Messaging::ExchangeManager mExchangeManager;
    secure_channel::MessageCounterManager mMessageCounterManager;
    IOContext * mIOContext;
    TransportMgrBase * mTransport; // Only needed for InitFromExisting.

    NodeId mBobNodeId       = 123654;
    NodeId mAliceNodeId     = 111222333;
    uint16_t mBobKeyId      = 1;
    uint16_t mAliceKeyId    = 2;
    GroupId mFriendsGroupId = 0x0101;
    Transport::PeerAddress mAliceAddress;
    Transport::PeerAddress mBobAddress;
    SecurePairingUsingTestSecret mPairingAliceToBob;
    SecurePairingUsingTestSecret mPairingBobToAlice;
    SessionHolder mSessionAliceToBob;
    SessionHolder mSessionBobToAlice;
    SessionHolder mSessionBobToFriends;
    FabricIndex mSrcFabricIndex  = 1;
    FabricIndex mDestFabricIndex = 1;
};

template <typename Transport = LoopbackTransport>
class LoopbackMessagingContext : public MessagingContext
{
public:
    virtual ~LoopbackMessagingContext() {}

    /// Initialize the underlying layers.
    virtual CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(chip::Platform::MemoryInit());
        ReturnErrorOnFailure(mIOContext.Init());
        ReturnErrorOnFailure(mTransportManager.Init("LOOPBACK"));
        ReturnErrorOnFailure(MessagingContext::Init(&mTransportManager, &mIOContext));
        return CHIP_NO_ERROR;
    }

    // Shutdown all layers, finalize operations
    virtual CHIP_ERROR Shutdown()
    {
        ReturnErrorOnFailure(MessagingContext::Shutdown());
        ReturnErrorOnFailure(mIOContext.Shutdown());
        chip::Platform::MemoryShutdown();
        return CHIP_NO_ERROR;
    }

    // Init/Shutdown Helpers that can be used directly as the nlTestSuite
    // initialize/finalize function.
    static int Initialize(void * context)
    {
        auto * ctx = static_cast<LoopbackMessagingContext *>(context);
        return ctx->Init() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    static int InitializeAsync(void * context)
    {
        auto * ctx = static_cast<LoopbackMessagingContext *>(context);

        VerifyOrReturnError(ctx->Init() == CHIP_NO_ERROR, FAILURE);
        ctx->EnableAsyncDispatch();

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        auto * ctx = static_cast<LoopbackMessagingContext *>(context);
        return ctx->Shutdown() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    Transport & GetLoopback() { return mTransportManager.GetTransport().template GetImplAtIndex<0>(); }

    TransportMgrBase & GetTransportMgr() { return mTransportManager; }

    IOContext & GetIOContext() { return mIOContext; }

    /*
     * For unit-tests that simulate end-to-end transmission and reception of messages in loopback mode,
     * this mode better replicates a real-functioning stack that correctly handles the processing
     * of a transmitted message as an asynchronous, bottom half handler dispatched after the current execution context has
     completed.
     * This is achieved using SystemLayer::ScheduleWork.

     * This should be used in conjunction with the DrainAndServiceIO function below to correctly service and drain the event queue.
     *
     */
    void EnableAsyncDispatch()
    {
        auto & impl = GetLoopback();
        impl.EnableAsyncDispatch(&mIOContext.GetSystemLayer());
    }

    /*
     * Reset the dispatch back to a model that synchronously dispatches received messages up the stack.
     *
     * NOTE: This results in highly atypical/complex call stacks that are not representative of what happens on real
     * devices and can cause subtle and complex bugs to either appear or get masked in the system. Where possible, please
     * use this sparingly!
     *
     */
    void DisableAsyncDispatch()
    {
        auto & impl = GetLoopback();
        impl.DisableAsyncDispatch();
    }

    /*
     * This drives the servicing of events using the embedded IOContext while there are pending
     * messages in the loopback transport's pending message queue. This should run to completion
     * in well-behaved logic (i.e there isn't an indefinite ping-pong of messages transmitted back
     * and forth).
     *
     * Consequently, this is guarded with a user-provided timeout to ensure we don't have unit-tests that stall
     * in CI due to bugs in the code that is being tested.
     *
     * This DOES NOT ensure that all pending events are serviced to completion (i.e timers, any ScheduleWork calls).
     *
     */
    void DrainAndServiceIO(System::Clock::Timeout maxWait = chip::System::Clock::Seconds16(5))
    {
        auto & impl                        = GetLoopback();
        System::Clock::Timestamp startTime = System::SystemClock().GetMonotonicTimestamp();

        while (impl.HasPendingMessages())
        {
            mIOContext.DriveIO();
            if ((System::SystemClock().GetMonotonicTimestamp() - startTime) >= maxWait)
            {
                break;
            }
        }
    }

private:
    TransportMgr<Transport> mTransportManager;
    Test::IOContext mIOContext;
};

} // namespace Test
} // namespace chip
