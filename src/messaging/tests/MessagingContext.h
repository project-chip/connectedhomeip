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

#include <pw_unit_test/framework.h>

#include <credentials/PersistentStorageOpCertStore.h>
#include <crypto/DefaultSessionKeystore.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/core/CASEAuthTag.h>
#include <lib/support/CodeUtils.h>
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
                      //      i.e IDLE = 500ms, ACTIVE = 300ms

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

    MessagingContext() : mpData(std::make_unique<MessagingContextData>()) {}

    // Whether Alice and Bob are initialized, must be called before Init
    void ConfigInitializeNodes(bool initializeNodes) { mpData->mInitializeNodes = initializeNodes; }

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
    GroupId GetFriendsGroupId() const { return mpData->mFriendsGroupId; }

    SessionManager & GetSecureSessionManager() { return mpData->mSessionManager; }
    Messaging::ExchangeManager & GetExchangeManager() { return mpData->mExchangeManager; }
    secure_channel::MessageCounterManager & GetMessageCounterManager() { return mpData->mMessageCounterManager; }
    FabricTable & GetFabricTable() { return mpData->mFabricTable; }
    Crypto::DefaultSessionKeystore & GetSessionKeystore() { return mpData->mSessionKeystore; }

    FabricIndex GetAliceFabricIndex() { return mpData->mAliceFabricIndex; }
    FabricIndex GetBobFabricIndex() { return mpData->mBobFabricIndex; }
    const FabricInfo * GetAliceFabric() { return mpData->mFabricTable.FindFabricWithIndex(mpData->mAliceFabricIndex); }
    const FabricInfo * GetBobFabric() { return mpData->mFabricTable.FindFabricWithIndex(mpData->mBobFabricIndex); }

    CHIP_ERROR CreateSessionBobToAlice(); // Creates PASE session
    CHIP_ERROR CreateCASESessionBobToAlice();
    CHIP_ERROR CreateCASESessionBobToAlice(const CATValues & cats);
    CHIP_ERROR CreateSessionAliceToBob(); // Creates PASE session
    CHIP_ERROR CreateCASESessionAliceToBob();
    CHIP_ERROR CreateCASESessionAliceToBob(const CATValues & cats);
    CHIP_ERROR CreateSessionBobToFriends(); // Creates PASE session
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

    const Transport::PeerAddress & GetAliceAddress() { return mpData->mAliceAddress; }
    const Transport::PeerAddress & GetBobAddress() { return mpData->mBobAddress; }

    Messaging::ExchangeContext * NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate);
    Messaging::ExchangeContext * NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate);

    Messaging::ExchangeContext * NewExchangeToAlice(Messaging::ExchangeDelegate * delegate, bool isInitiator = true);
    Messaging::ExchangeContext * NewExchangeToBob(Messaging::ExchangeDelegate * delegate, bool isInitiator = true);

    System::Layer & GetSystemLayer() { return mpData->mIOContext->GetSystemLayer(); }

private:
    // These members are encapsulated in a struct which is allocated upon construction of MessagingContext and freed upon
    // destruction of MessagingContext.  This is done to save stack space.
    struct MessagingContextData
    {
        MessagingContextData() :
            mAliceAddress(Transport::PeerAddress::UDP(GetAddress(), CHIP_PORT + 1)),
            mBobAddress(LoopbackTransport::LoopbackPeer(mAliceAddress))
        {}
        ~MessagingContextData() { EXPECT_FALSE(mInitialized); }

        bool mInitializeNodes = true;
        bool mInitialized     = false;
        FabricTable mFabricTable;

        SessionManager mSessionManager;
        Messaging::ExchangeManager mExchangeManager;
        secure_channel::MessageCounterManager mMessageCounterManager;
        IOContext * mIOContext        = nullptr;
        TransportMgrBase * mTransport = nullptr;      // Only needed for InitFromExisting.
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
    std::unique_ptr<MessagingContextData> mpData;
};

// LoopbackMessagingContext enriches MessagingContext with an async loopback transport
class LoopbackMessagingContext : public ::testing::Test, public MessagingContext
{
public:
    virtual ~LoopbackMessagingContext() {}

    // Pigweed test pure virtual will get overriden
    // TODO: why is a context a Test?
    virtual void PigweedTestBody() {}

    // These functions wrap spLoopbackTransportManager methods
    static auto & GetSystemLayer() { return spLoopbackTransportManager->GetSystemLayer(); }
    static auto & GetLoopback() { return spLoopbackTransportManager->GetLoopback(); }
    static auto & GetTransportMgr() { return spLoopbackTransportManager->GetTransportMgr(); }
    static auto & GetIOContext() { return spLoopbackTransportManager->GetIOContext(); }

    template <typename... Ts>
    static void DrainAndServiceIO(Ts... args)
    {
        return spLoopbackTransportManager->DrainAndServiceIO(args...);
    }

    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        // Initialize memory.
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        // Instantiate the LoopbackTransportManager.
        ASSERT_EQ(spLoopbackTransportManager, nullptr);
        spLoopbackTransportManager = new LoopbackTransportManager();
        ASSERT_NE(spLoopbackTransportManager, nullptr);
        // Initialize the LoopbackTransportManager.
        ASSERT_EQ(spLoopbackTransportManager->Init(), CHIP_NO_ERROR);
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        // Shutdown the LoopbackTransportManager.
        spLoopbackTransportManager->Shutdown();
        // Destroy the LoopbackTransportManager.
        if (spLoopbackTransportManager != nullptr)
        {
            delete spLoopbackTransportManager;
            spLoopbackTransportManager = nullptr;
        }
        // Shutdown memory.
        chip::Platform::MemoryShutdown();
    }

    // Performs setup for each individual test in the test suite
    virtual void SetUp() { ASSERT_EQ(MessagingContext::Init(&GetTransportMgr(), &GetIOContext()), CHIP_NO_ERROR); }

    // Performs teardown for each individual test in the test suite
    virtual void TearDown() { MessagingContext::Shutdown(); }

    static LoopbackTransportManager * spLoopbackTransportManager;
};

// UDPMessagingContext enriches MessagingContext with an UDP transport
class UDPMessagingContext : public ::testing::Test, public MessagingContext
{
public:
    virtual ~UDPMessagingContext() {}

    static auto & GetSystemLayer() { return spUDPTransportManager->GetSystemLayer(); }
    static auto & GetTransportMgr() { return spUDPTransportManager->GetTransportMgr(); }
    static auto & GetIOContext() { return spUDPTransportManager->GetIOContext(); }

    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        // Initialize memory.
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        // Instantiate the UDPTransportManager.
        ASSERT_EQ(spUDPTransportManager, nullptr);
        spUDPTransportManager = new UDPTransportManager();
        ASSERT_NE(spUDPTransportManager, nullptr);
        // Initialize the UDPTransportManager.
        ASSERT_EQ(spUDPTransportManager->Init(), CHIP_NO_ERROR);
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        // Shutdown the UDPTransportManager.
        spUDPTransportManager->Shutdown();
        // Destroy the UDPTransportManager.
        if (spUDPTransportManager != nullptr)
        {
            delete spUDPTransportManager;
            spUDPTransportManager = nullptr;
        }
        // Shutdown memory.
        chip::Platform::MemoryShutdown();
    }

    // Performs setup for each individual test in the test suite
    virtual void SetUp() { ASSERT_EQ(MessagingContext::Init(&GetTransportMgr(), &GetIOContext()), CHIP_NO_ERROR); }

    // Performs teardown for each individual test in the test suite
    virtual void TearDown() { MessagingContext::Shutdown(); }

    static UDPTransportManager * spUDPTransportManager;
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
