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
#include <lib/core/ErrorStr.h>
#include <lib/support/CodeUtils.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Test {

using namespace TestCerts;

CHIP_ERROR MessagingContext::Init(TransportMgrBase * transport, IOContext * ioContext)
{
    VerifyOrReturnError(mpData->mInitialized == false, CHIP_ERROR_INTERNAL);
    mpData->mInitialized = true;

    mpData->mIOContext = ioContext;
    mpData->mTransport = transport;

    ReturnErrorOnFailure(PlatformMemoryUser::Init());

    // Make sure the storage is clean, so we will not reuse any stale data.
    mpData->mStorage.ClearStorage();

    ReturnErrorOnFailure(mpData->mOpKeyStore.Init(&mpData->mStorage));
    ReturnErrorOnFailure(mpData->mOpCertStore.Init(&mpData->mStorage));

    chip::FabricTable::InitParams initParams;
    initParams.storage             = &mpData->mStorage;
    initParams.operationalKeystore = &mpData->mOpKeyStore;
    initParams.opCertStore         = &mpData->mOpCertStore;

    ReturnErrorOnFailure(mpData->mFabricTable.Init(initParams));

    ReturnErrorOnFailure(mpData->mSessionManager.Init(&GetSystemLayer(), transport, &mpData->mMessageCounterManager,
                                                      &mpData->mStorage, &mpData->mFabricTable, mpData->mSessionKeystore));

    ReturnErrorOnFailure(mpData->mExchangeManager.Init(&mpData->mSessionManager));
    ReturnErrorOnFailure(mpData->mMessageCounterManager.Init(&mpData->mExchangeManager));

    if (mpData->mInitializeNodes)
    {
        ReturnErrorOnFailure(CreateAliceFabric());
        ReturnErrorOnFailure(CreateBobFabric());

        ReturnErrorOnFailure(CreateSessionBobToAlice());
        ReturnErrorOnFailure(CreateSessionAliceToBob());
        ReturnErrorOnFailure(CreateSessionBobToFriends());

        ReturnErrorOnFailure(CreatePASESessionCharlieToDavid());
        ReturnErrorOnFailure(CreatePASESessionDavidToCharlie());
    }

    // Set the additional MRP backoff to zero so that it does not affect the test execution time.
    Messaging::ReliableMessageMgr::SetAdditionalMRPBackoffTime(MakeOptional(System::Clock::kZero));

    return CHIP_NO_ERROR;
}

// Shutdown all layers, finalize operations
void MessagingContext::Shutdown()
{
    VerifyOrDie(mpData->mInitialized);
    mpData->mInitialized = false;

    mpData->mMessageCounterManager.Shutdown();
    mpData->mExchangeManager.Shutdown();
    mpData->mSessionManager.Shutdown();
    mpData->mFabricTable.Shutdown();
    mpData->mOpCertStore.Finish();
    mpData->mOpKeyStore.Finish();

    // Reset the default additional MRP backoff.
    Messaging::ReliableMessageMgr::SetAdditionalMRPBackoffTime(NullOptional);
}

CHIP_ERROR MessagingContext::InitFromExisting(const MessagingContext & existing)
{
    return Init(existing.mpData->mTransport, existing.mpData->mIOContext);
}

void MessagingContext::ShutdownAndRestoreExisting(MessagingContext & existing)
{
    Shutdown();
    // Point the transport back to the original session manager, since we had
    // pointed it to ours.
    existing.mpData->mTransport->SetSessionManager(&existing.GetSecureSessionManager());
}

using namespace System::Clock::Literals;

constexpr chip::System::Clock::Timeout MessagingContext::kResponsiveIdleRetransTimeout;
constexpr chip::System::Clock::Timeout MessagingContext::kResponsiveActiveRetransTimeout;

void MessagingContext::SetMRPMode(MRPMode mode)
{
    if (mode == MRPMode::kDefault)
    {
        mpData->mSessionBobToAlice->AsSecureSession()->SetRemoteSessionParameters(GetDefaultMRPConfig());
        mpData->mSessionAliceToBob->AsSecureSession()->SetRemoteSessionParameters(GetDefaultMRPConfig());
        mpData->mSessionCharlieToDavid->AsSecureSession()->SetRemoteSessionParameters(GetDefaultMRPConfig());
        mpData->mSessionDavidToCharlie->AsSecureSession()->SetRemoteSessionParameters(GetDefaultMRPConfig());

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

        mpData->mSessionBobToAlice->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
        mpData->mSessionAliceToBob->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
        mpData->mSessionCharlieToDavid->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
        mpData->mSessionDavidToCharlie->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig(
            MessagingContext::kResponsiveIdleRetransTimeout, MessagingContext::kResponsiveActiveRetransTimeout));
    }
}

CHIP_ERROR MessagingContext::CreateAliceFabric()
{
    return mpData->mFabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                                      GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey,
                                                                      &mpData->mAliceFabricIndex);
}

CHIP_ERROR MessagingContext::CreateBobFabric()
{
    return mpData->mFabricTable.AddNewFabricForTestIgnoringCollisions(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                                      GetNodeA2CertAsset().mCert, GetNodeA2CertAsset().mKey,
                                                                      &mpData->mBobFabricIndex);
}

CHIP_ERROR MessagingContext::CreateSessionBobToAlice()
{
    return mpData->mSessionManager.InjectPaseSessionWithTestKey(mpData->mSessionBobToAlice, kBobKeyId,
                                                                GetAliceFabric()->GetNodeId(), kAliceKeyId, mpData->mBobFabricIndex,
                                                                mpData->mAliceAddress, CryptoContext::SessionRole::kInitiator);
}

CHIP_ERROR MessagingContext::CreateCASESessionBobToAlice()
{
    return mpData->mSessionManager.InjectCaseSessionWithTestKey(
        mpData->mSessionBobToAlice, kBobKeyId, kAliceKeyId, GetBobFabric()->GetNodeId(), GetAliceFabric()->GetNodeId(),
        mpData->mBobFabricIndex, mpData->mAliceAddress, CryptoContext::SessionRole::kInitiator);
}

CHIP_ERROR MessagingContext::CreateCASESessionBobToAlice(const CATValues & cats)
{
    return mpData->mSessionManager.InjectCaseSessionWithTestKey(
        mpData->mSessionBobToAlice, kBobKeyId, kAliceKeyId, GetBobFabric()->GetNodeId(), GetAliceFabric()->GetNodeId(),
        mpData->mBobFabricIndex, mpData->mAliceAddress, CryptoContext::SessionRole::kInitiator, cats);
}

CHIP_ERROR MessagingContext::CreateSessionAliceToBob()
{
    return mpData->mSessionManager.InjectPaseSessionWithTestKey(mpData->mSessionAliceToBob, kAliceKeyId,
                                                                GetBobFabric()->GetNodeId(), kBobKeyId, mpData->mAliceFabricIndex,
                                                                mpData->mBobAddress, CryptoContext::SessionRole::kResponder);
}

CHIP_ERROR MessagingContext::CreateCASESessionAliceToBob()
{
    return mpData->mSessionManager.InjectCaseSessionWithTestKey(
        mpData->mSessionAliceToBob, kAliceKeyId, kBobKeyId, GetAliceFabric()->GetNodeId(), GetBobFabric()->GetNodeId(),
        mpData->mAliceFabricIndex, mpData->mBobAddress, CryptoContext::SessionRole::kResponder);
}

CHIP_ERROR MessagingContext::CreateCASESessionAliceToBob(const CATValues & cats)
{
    return mpData->mSessionManager.InjectCaseSessionWithTestKey(
        mpData->mSessionAliceToBob, kAliceKeyId, kBobKeyId, GetAliceFabric()->GetNodeId(), GetBobFabric()->GetNodeId(),
        mpData->mAliceFabricIndex, mpData->mBobAddress, CryptoContext::SessionRole::kResponder, cats);
}

CHIP_ERROR MessagingContext::CreatePASESessionCharlieToDavid()
{
    return mpData->mSessionManager.InjectPaseSessionWithTestKey(mpData->mSessionCharlieToDavid, kCharlieKeyId, 0xdeadbeef,
                                                                kDavidKeyId, kUndefinedFabricIndex, mpData->mDavidAddress,
                                                                CryptoContext::SessionRole::kInitiator);
}

CHIP_ERROR MessagingContext::CreatePASESessionDavidToCharlie()
{
    return mpData->mSessionManager.InjectPaseSessionWithTestKey(mpData->mSessionDavidToCharlie, kDavidKeyId, 0xcafe, kCharlieKeyId,
                                                                kUndefinedFabricIndex, mpData->mCharlieAddress,
                                                                CryptoContext::SessionRole::kResponder);
}

CHIP_ERROR MessagingContext::CreateSessionBobToFriends()
{
    mpData->mSessionBobToFriends.Emplace(GetFriendsGroupId(), mpData->mBobFabricIndex);
    return CHIP_NO_ERROR;
}

SessionHandle MessagingContext::GetSessionBobToAlice()
{
    auto sessionHandle = mpData->mSessionBobToAlice.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionAliceToBob()
{
    auto sessionHandle = mpData->mSessionAliceToBob.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionCharlieToDavid()
{
    auto sessionHandle = mpData->mSessionCharlieToDavid.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionDavidToCharlie()
{
    auto sessionHandle = mpData->mSessionDavidToCharlie.Get();
    return std::move(sessionHandle.Value());
}

SessionHandle MessagingContext::GetSessionBobToFriends()
{
    return SessionHandle(mpData->mSessionBobToFriends.Value());
}

void MessagingContext::ExpireSessionBobToAlice()
{
    if (mpData->mSessionBobToAlice)
    {
        mpData->mSessionBobToAlice.Get().Value()->AsSecureSession()->MarkForEviction();
    }
}

void MessagingContext::ExpireSessionAliceToBob()
{
    if (mpData->mSessionAliceToBob)
    {
        mpData->mSessionAliceToBob.Get().Value()->AsSecureSession()->MarkForEviction();
    }
}

void MessagingContext::ExpireSessionBobToFriends()
{
    mpData->mSessionBobToFriends.ClearValue();
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate)
{
    return mpData->mExchangeManager.NewContext(
        mpData->mSessionManager
            .CreateUnauthenticatedSession(mpData->mAliceAddress, GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()))
            .Value(),
        delegate);
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate)
{
    return mpData->mExchangeManager.NewContext(
        mpData->mSessionManager
            .CreateUnauthenticatedSession(mpData->mBobAddress, GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()))
            .Value(),
        delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToAlice(Messaging::ExchangeDelegate * delegate, bool isInitiator)
{
    return mpData->mExchangeManager.NewContext(GetSessionBobToAlice(), delegate, isInitiator);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToBob(Messaging::ExchangeDelegate * delegate, bool isInitiator)
{
    return mpData->mExchangeManager.NewContext(GetSessionAliceToBob(), delegate, isInitiator);
}

LoopbackTransportManager * LoopbackMessagingContext::spLoopbackTransportManager = nullptr;

UDPTransportManager * UDPMessagingContext::spUDPTransportManager = nullptr;

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
