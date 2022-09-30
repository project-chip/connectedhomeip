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

#include "MessagingContext.h"
#include "system/SystemClock.h"

#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Test {

using namespace TestCerts;

CHIP_ERROR MessagingContext::Init(TransportMgrBase * transport, IOContext * ioContext)
{
    VerifyOrReturnError(mInitialized == false, CHIP_ERROR_INTERNAL);
    mInitialized = true;

    mIOContext = ioContext;
    mTransport = transport;

    ReturnErrorOnFailure(PlatformMemoryUser::Init());

    ReturnErrorOnFailure(mOpKeyStore.Init(&mStorage));
    ReturnErrorOnFailure(mOpCertStore.Init(&mStorage));

    chip::FabricTable::InitParams initParams;
    initParams.storage             = &mStorage;
    initParams.operationalKeystore = &mOpKeyStore;
    initParams.opCertStore         = &mOpCertStore;

    ReturnErrorOnFailure(mFabricTable.Init(initParams));

    ReturnErrorOnFailure(mSessionManager.Init(&GetSystemLayer(), transport, &mMessageCounterManager, &mStorage, &mFabricTable));

    ReturnErrorOnFailure(mExchangeManager.Init(&mSessionManager));
    ReturnErrorOnFailure(mMessageCounterManager.Init(&mExchangeManager));

    if (mInitializeNodes)
    {
        ReturnErrorOnFailure(CreateAliceFabric());
        ReturnErrorOnFailure(CreateBobFabric());

        ReturnErrorOnFailure(CreateSessionBobToAlice());
        ReturnErrorOnFailure(CreateSessionAliceToBob());
        ReturnErrorOnFailure(CreateSessionBobToFriends());

        ReturnErrorOnFailure(CreatePASESessionCharlieToDavid());
        ReturnErrorOnFailure(CreatePASESessionDavidToCharlie());
    }

    return CHIP_NO_ERROR;
}

// Shutdown all layers, finalize operations
void MessagingContext::Shutdown()
{
    VerifyOrDie(mInitialized);
    mInitialized = false;

    mExchangeManager.Shutdown();
    mSessionManager.Shutdown();
    mFabricTable.Shutdown();
    mOpCertStore.Finish();
    mOpKeyStore.Finish();
}

CHIP_ERROR MessagingContext::InitFromExisting(const MessagingContext & existing)
{
    return Init(existing.mTransport, existing.mIOContext);
}

void MessagingContext::ShutdownAndRestoreExisting(MessagingContext & existing)
{
    Shutdown();
    // Point the transport back to the original session manager, since we had
    // pointed it to ours.
    existing.mTransport->SetSessionManager(&existing.GetSecureSessionManager());
}

using namespace System::Clock::Literals;

constexpr chip::System::Clock::Timeout MessagingContext::kResponsiveIdleRetransTimeout;
constexpr chip::System::Clock::Timeout MessagingContext::kResponsiveActiveRetransTimeout;

void MessagingContext::SetMRPMode(MRPMode mode)
{
    if (mode == MRPMode::kDefault)
    {
        mSessionBobToAlice->AsSecureSession()->SetRemoteMRPConfig(GetDefaultMRPConfig());
        mSessionAliceToBob->AsSecureSession()->SetRemoteMRPConfig(GetDefaultMRPConfig());
        mSessionCharlieToDavid->AsSecureSession()->SetRemoteMRPConfig(GetDefaultMRPConfig());
        mSessionDavidToCharlie->AsSecureSession()->SetRemoteMRPConfig(GetDefaultMRPConfig());

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        ClearLocalMRPConfigOverride();
#else
        //
        // A test is calling this function assuming the overrides above are going to work
        // when in fact, they won't because the compile flag is not set correctly.
        //
        VerifyOrDie(false);
#endif
    }
    else
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        OverrideLocalMRPConfig(MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout);
#else
        //
        // A test is calling this function assuming the overrides above are going to work
        // when in fact, they won't because the compile flag is not set correctly.
        //
        VerifyOrDie(false);
#endif

        mSessionBobToAlice->AsSecureSession()->SetRemoteMRPConfig(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
        mSessionAliceToBob->AsSecureSession()->SetRemoteMRPConfig(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
        mSessionCharlieToDavid->AsSecureSession()->SetRemoteMRPConfig(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
        mSessionDavidToCharlie->AsSecureSession()->SetRemoteMRPConfig(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
    }
}

CHIP_ERROR MessagingContext::CreateAliceFabric()
{
    return mFabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                              GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey,
                                                              &mAliceFabricIndex);
}

CHIP_ERROR MessagingContext::CreateBobFabric()
{
    return mFabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                              GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey,
                                                              &mBobFabricIndex);
}

CHIP_ERROR MessagingContext::CreateSessionBobToAlice()
{
    return mSessionManager.InjectPaseSessionWithTestKey(mSessionBobToAlice, kBobKeyId, GetAliceFabric()->GetNodeId(), kAliceKeyId,
                                                        mBobFabricIndex, mAliceAddress, CryptoContext::SessionRole::kInitiator);
}

CHIP_ERROR MessagingContext::CreateSessionAliceToBob()
{
    return mSessionManager.InjectPaseSessionWithTestKey(mSessionAliceToBob, kAliceKeyId, GetBobFabric()->GetNodeId(), kBobKeyId,
                                                        mAliceFabricIndex, mBobAddress, CryptoContext::SessionRole::kResponder);
}

CHIP_ERROR MessagingContext::CreatePASESessionCharlieToDavid()
{
    return mSessionManager.InjectPaseSessionWithTestKey(mSessionCharlieToDavid, kCharlieKeyId, 0xdeadbeef, kDavidKeyId,
                                                        kUndefinedFabricIndex, mDavidAddress,
                                                        CryptoContext::SessionRole::kInitiator);
}

CHIP_ERROR MessagingContext::CreatePASESessionDavidToCharlie()
{
    return mSessionManager.InjectPaseSessionWithTestKey(mSessionDavidToCharlie, kDavidKeyId, 0xcafe, kCharlieKeyId,
                                                        kUndefinedFabricIndex, mCharlieAddress,
                                                        CryptoContext::SessionRole::kResponder);
}

CHIP_ERROR MessagingContext::CreateSessionBobToFriends()
{
    mSessionBobToFriends.Emplace(GetFriendsGroupId(), mBobFabricIndex);
    return CHIP_NO_ERROR;
}

SessionHandle MessagingContext::GetSessionBobToAlice()
{
    auto sessionHandle = mSessionBobToAlice.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionAliceToBob()
{
    auto sessionHandle = mSessionAliceToBob.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionCharlieToDavid()
{
    auto sessionHandle = mSessionCharlieToDavid.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionDavidToCharlie()
{
    auto sessionHandle = mSessionDavidToCharlie.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionBobToFriends()
{
    return SessionHandle(mSessionBobToFriends.Value());
}

void MessagingContext::ExpireSessionBobToAlice()
{
    if (mSessionBobToAlice)
    {
        mSessionBobToAlice.Get().Value()->AsSecureSession()->MarkForEviction();
    }
}

void MessagingContext::ExpireSessionAliceToBob()
{
    if (mSessionAliceToBob)
    {
        mSessionAliceToBob.Get().Value()->AsSecureSession()->MarkForEviction();
    }
}

void MessagingContext::ExpireSessionBobToFriends()
{
    mSessionBobToFriends.ClearValue();
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(
        mSessionManager.CreateUnauthenticatedSession(mAliceAddress, GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig())).Value(),
        delegate);
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(
        mSessionManager.CreateUnauthenticatedSession(mBobAddress, GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig())).Value(),
        delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToAlice(Messaging::ExchangeDelegate * delegate, bool isInitiator)
{
    return mExchangeManager.NewContext(GetSessionBobToAlice(), delegate, isInitiator);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToBob(Messaging::ExchangeDelegate * delegate, bool isInitiator)
{
    return mExchangeManager.NewContext(GetSessionAliceToBob(), delegate, isInitiator);
}

void MessageCapturer::OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                        const SessionHandle & session, DuplicateMessage isDuplicate,
                                        System::PacketBufferHandle && msgBuf)
{
    if (mCaptureStandaloneAcks || !payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck))
    {
        mCapturedMessages.emplace_back(Message{ packetHeader, payloadHeader, isDuplicate, msgBuf.CloneData() });
    }
    mOriginalDelegate.OnMessageReceived(packetHeader, payloadHeader, session, isDuplicate, std::move(msgBuf));
}

} // namespace Test
} // namespace chip
