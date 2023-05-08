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

#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <system/SystemClock.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/tests/LoopbackTransportManager.h>
#include <transport/tests/UDPTransportManager.h>

#include <nlunit-test.h>

#include <vector>

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
 * @brief The context of test cases for messaging layer. It will initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class MessagingContext : public PlatformMemoryUser
{
public:
    enum MRPMode
    {
        kDefault = 1, // This adopts the default MRP values for idle/active as per the spec.
                      //      i.e IDLE = 4s, ACTIVE = 300ms

        kResponsive = 2, // This adopts values that are better suited for loopback tests that
                         // don't actually go over a network interface, and are tuned much lower
                         // to permit more responsive tests.
                         //      i.e IDLE = 10ms, ACTIVE = 10ms
    };

    //
    // See above for a description of the values used.
    //
    static constexpr System::Clock::Timeout kResponsiveIdleRetransTimeout   = System::Clock::Milliseconds32(10);
    static constexpr System::Clock::Timeout kResponsiveActiveRetransTimeout = System::Clock::Milliseconds32(10);

    MessagingContext() :
        mInitialized(false), mAliceAddress(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT + 1)),
        mBobAddress(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT))
    {}
    ~MessagingContext() { VerifyOrDie(mInitialized == false); }

    // Whether Alice and Bob are initialized, must be called before Init
    void ConfigInitializeNodes(bool initializeNodes) { mInitializeNodes = initializeNodes; }

    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init(TransportMgrBase * transport, IOContext * io);

    // Shutdown all layers, finalize operations
    void Shutdown();

    // Initialize from an existing messaging context.  Useful if we want to
    // share some state (like the transport).
    CHIP_ERROR InitFromExisting(const MessagingContext & existing);

    // The shutdown method to use if using InitFromExisting.  Must pass in the
    // same existing context as was passed to InitFromExisting.
    void ShutdownAndRestoreExisting(MessagingContext & existing);

    static Inet::IPAddress GetAddress()
    {
        Inet::IPAddress addr;
        Inet::IPAddress::FromString("::1", addr);
        return addr;
    }

    static const uint16_t kBobKeyId     = 1;
    static const uint16_t kAliceKeyId   = 2;
    static const uint16_t kCharlieKeyId = 3;
    static const uint16_t kDavidKeyId   = 4;
    GroupId GetFriendsGroupId() const { return mFriendsGroupId; }

    SessionManager & GetSecureSessionManager() { return mSessionManager; }
    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeManager; }
    secure_channel::MessageCounterManager & GetMessageCounterManager() { return mMessageCounterManager; }
    FabricTable & GetFabricTable() { return mFabricTable; }
    Crypto::DefaultSessionKeystore & GetSessionKeystore() { return mSessionKeystore; }

    FabricIndex GetAliceFabricIndex() { return mAliceFabricIndex; }
    FabricIndex GetBobFabricIndex() { return mBobFabricIndex; }
    const FabricInfo * GetAliceFabric() { return mFabricTable.FindFabricWithIndex(mAliceFabricIndex); }
    const FabricInfo * GetBobFabric() { return mFabricTable.FindFabricWithIndex(mBobFabricIndex); }

    CHIP_ERROR CreateSessionBobToAlice();
    CHIP_ERROR CreateSessionAliceToBob();
    CHIP_ERROR CreateSessionBobToFriends();
    CHIP_ERROR CreatePASESessionCharlieToDavid();
    CHIP_ERROR CreatePASESessionDavidToCharlie();

    void ExpireSessionBobToAlice();
    void ExpireSessionAliceToBob();
    void ExpireSessionBobToFriends();

    void SetMRPMode(MRPMode mode);

    SessionHandle GetSessionBobToAlice();
    SessionHandle GetSessionAliceToBob();
    SessionHandle GetSessionCharlieToDavid();
    SessionHandle GetSessionDavidToCharlie();
    SessionHandle GetSessionBobToFriends();

    CHIP_ERROR CreateAliceFabric();
    CHIP_ERROR CreateBobFabric();

    const Transport::PeerAddress & GetAliceAddress() { return mAliceAddress; }
    const Transport::PeerAddress & GetBobAddress() { return mBobAddress; }

    Messaging::ExchangeContext * NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate);

    Messaging::ExchangeContext * NewExchangeToAlice(Messaging::ExchangeDelegate * delegate, bool isInitiator = true);
    Messaging::ExchangeContext * NewExchangeToBob(Messaging::ExchangeDelegate * delegate, bool isInitiator = true);

    System::Layer & GetSystemLayer() { return mIOContext->GetSystemLayer(); }

private:
    bool mInitializeNodes = true;
    bool mInitialized;
    FabricTable mFabricTable;

    SessionManager mSessionManager;
    Messaging::ExchangeManager mExchangeManager;
    secure_channel::MessageCounterManager mMessageCounterManager;
    IOContext * mIOContext;
    TransportMgrBase * mTransport;                // Only needed for InitFromExisting.
    chip::TestPersistentStorageDelegate mStorage; // for SessionManagerInit
    chip::PersistentStorageOperationalKeystore mOpKeyStore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
    chip::Crypto::DefaultSessionKeystore mSessionKeystore;

    FabricIndex mAliceFabricIndex = kUndefinedFabricIndex;
    FabricIndex mBobFabricIndex   = kUndefinedFabricIndex;
    GroupId mFriendsGroupId       = 0x0101;
    Transport::PeerAddress mAliceAddress;
    Transport::PeerAddress mBobAddress;
    Transport::PeerAddress mCharlieAddress;
    Transport::PeerAddress mDavidAddress;
    SessionHolder mSessionAliceToBob;
    SessionHolder mSessionBobToAlice;
    SessionHolder mSessionCharlieToDavid;
    SessionHolder mSessionDavidToCharlie;
    Optional<Transport::OutgoingGroupSession> mSessionBobToFriends;
};

// LoopbackMessagingContext enriches MessagingContext with an async loopback transport
class LoopbackMessagingContext : public LoopbackTransportManager, public MessagingContext
{
public:
    virtual ~LoopbackMessagingContext() {}

    /// Initialize the underlying layers.
    virtual CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(chip::Platform::MemoryInit());
        ReturnErrorOnFailure(LoopbackTransportManager::Init());
        ReturnErrorOnFailure(MessagingContext::Init(&GetTransportMgr(), &GetIOContext()));
        return CHIP_NO_ERROR;
    }

    // Shutdown all layers, finalize operations
    virtual void Shutdown()
    {
        MessagingContext::Shutdown();
        LoopbackTransportManager::Shutdown();
        chip::Platform::MemoryShutdown();
    }

    // Init/Shutdown Helpers that can be used directly as the nlTestSuite
    // initialize/finalize function.
    static int Initialize(void * context)
    {
        auto * ctx = static_cast<LoopbackMessagingContext *>(context);
        return ctx->Init() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    static int Finalize(void * context)
    {
        auto * ctx = static_cast<LoopbackMessagingContext *>(context);
        ctx->Shutdown();
        return SUCCESS;
    }

    using LoopbackTransportManager::GetSystemLayer;
};

// UDPMessagingContext enriches MessagingContext with an UDP transport
class UDPMessagingContext : public UDPTransportManager, public MessagingContext
{
public:
    virtual ~UDPMessagingContext() {}

    /// Initialize the underlying layers.
    virtual CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(chip::Platform::MemoryInit());
        ReturnErrorOnFailure(UDPTransportManager::Init());
        ReturnErrorOnFailure(MessagingContext::Init(&GetTransportMgr(), &GetIOContext()));
        return CHIP_NO_ERROR;
    }

    // Shutdown all layers, finalize operations
    virtual void Shutdown()
    {
        MessagingContext::Shutdown();
        UDPTransportManager::Shutdown();
        chip::Platform::MemoryShutdown();
    }

    // Init/Shutdown Helpers that can be used directly as the nlTestSuite
    // initialize/finalize function.
    static int Initialize(void * context)
    {
        auto * ctx = static_cast<UDPMessagingContext *>(context);
        return ctx->Init() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    static int Finalize(void * context)
    {
        auto * ctx = static_cast<UDPMessagingContext *>(context);
        ctx->Shutdown();
        return SUCCESS;
    }

    using UDPTransportManager::GetSystemLayer;
};

// Class that can be used to capture decrypted message traffic in tests using
// MessagingContext.
class MessageCapturer : public SessionMessageDelegate
{
public:
    MessageCapturer(MessagingContext & aContext) :
        mSessionManager(aContext.GetSecureSessionManager()), mOriginalDelegate(aContext.GetExchangeManager())
    {
        // Interpose ourselves into the message flow.
        mSessionManager.SetMessageDelegate(this);
    }

    ~MessageCapturer()
    {
        // Restore the normal message flow.
        mSessionManager.SetMessageDelegate(&mOriginalDelegate);
    }

    struct Message
    {
        PacketHeader mPacketHeader;
        PayloadHeader mPayloadHeader;
        DuplicateMessage mIsDuplicate;
        System::PacketBufferHandle mPayload;
    };

    size_t MessageCount() const { return mCapturedMessages.size(); }

    template <typename MessageType, typename = std::enable_if_t<std::is_enum<MessageType>::value>>
    bool IsMessageType(size_t index, MessageType type)
    {
        return mCapturedMessages[index].mPayloadHeader.HasMessageType(type);
    }

    System::PacketBufferHandle & MessagePayload(size_t index) { return mCapturedMessages[index].mPayload; }

    bool mCaptureStandaloneAcks = true;

private:
    // SessionMessageDelegate implementation.
    void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader, const SessionHandle & session,
                           DuplicateMessage isDuplicate, System::PacketBufferHandle && msgBuf) override;

    SessionManager & mSessionManager;
    SessionMessageDelegate & mOriginalDelegate;
    std::vector<Message> mCapturedMessages;
};

} // namespace Test
} // namespace chip
