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

#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>

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
    ReturnErrorOnFailure(mFabricTable.Init(&mStorage));
    ReturnErrorOnFailure(mSessionManager.Init(&GetSystemLayer(), transport, &mMessageCounterManager, &mStorage, &mFabricTable));

    ReturnErrorOnFailure(mExchangeManager.Init(&mSessionManager));
    ReturnErrorOnFailure(mMessageCounterManager.Init(&mExchangeManager));

    if (mInitializeNodes)
    {
        FabricInfo aliceFabric;
        FabricInfo bobFabric;

        ReturnErrorOnFailure(aliceFabric.TestOnlyBuildFabric(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert,
                                                             GetNodeA1CertAsset().mCert, GetNodeA1CertAsset().mKey));
        ReturnErrorOnFailure(mFabricTable.AddNewFabricForTest(aliceFabric, &mAliceFabricIndex));

        bobFabric.TestOnlyBuildFabric(GetRootACertAsset().mCert, GetIAA1CertAsset().mCert, GetNodeA2CertAsset().mCert,
                                      GetNodeA2CertAsset().mKey);
        ReturnErrorOnFailure(mFabricTable.AddNewFabricForTest(bobFabric, &mBobFabricIndex));

        ReturnErrorOnFailure(CreateSessionBobToAlice());
        ReturnErrorOnFailure(CreateSessionAliceToBob());
        ReturnErrorOnFailure(CreateSessionBobToFriends());
    }

    return CHIP_NO_ERROR;
}

// Shutdown all layers, finalize operations
CHIP_ERROR MessagingContext::Shutdown()
{
    VerifyOrReturnError(mInitialized == true, CHIP_ERROR_INTERNAL);
    mInitialized = false;

    mExchangeManager.Shutdown();
    mSessionManager.Shutdown();
    return CHIP_NO_ERROR;
}

CHIP_ERROR MessagingContext::InitFromExisting(const MessagingContext & existing)
{
    return Init(existing.mTransport, existing.mIOContext);
}

CHIP_ERROR MessagingContext::ShutdownAndRestoreExisting(MessagingContext & existing)
{
    CHIP_ERROR err = Shutdown();
    // Point the transport back to the original session manager, since we had
    // pointed it to ours.
    existing.mTransport->SetSessionManager(&existing.GetSecureSessionManager());
    return err;
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

SessionHandle MessagingContext::GetSessionBobToFriends()
{
    return SessionHandle(mSessionBobToFriends.Value());
}

void MessagingContext::ExpireSessionBobToAlice()
{
    if (mSessionBobToAlice)
    {
        mSessionManager.ExpirePairing(mSessionBobToAlice.Get().Value());
    }
}

void MessagingContext::ExpireSessionAliceToBob()
{
    if (mSessionAliceToBob)
    {
        mSessionManager.ExpirePairing(mSessionAliceToBob.Get().Value());
    }
}

void MessagingContext::ExpireSessionBobToFriends()
{
    mSessionBobToFriends.ClearValue();
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(mSessionManager.CreateUnauthenticatedSession(mAliceAddress, GetLocalMRPConfig()).Value(),
                                       delegate);
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(mSessionManager.CreateUnauthenticatedSession(mBobAddress, GetLocalMRPConfig()).Value(),
                                       delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToAlice(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(GetSessionBobToAlice(), delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToBob(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(GetSessionAliceToBob(), delegate);
}

} // namespace Test
} // namespace chip
