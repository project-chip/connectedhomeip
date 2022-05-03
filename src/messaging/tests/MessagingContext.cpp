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

#include <credentials/tests/CHIPCert_test_vectors.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>

namespace chip {
namespace Test {

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

        aliceFabric.TestOnlyBuildFabric(
            ByteSpan(TestCerts::sTestCert_Root01_Chip, TestCerts::sTestCert_Root01_Chip_Len),
            ByteSpan(TestCerts::sTestCert_ICA01_Chip, TestCerts::sTestCert_ICA01_Chip_Len),
            ByteSpan(TestCerts::sTestCert_Node01_01_Chip, TestCerts::sTestCert_Node01_01_Chip_Len),
            ByteSpan(TestCerts::sTestCert_Node01_01_PublicKey, TestCerts::sTestCert_Node01_01_PublicKey_Len),
            ByteSpan(TestCerts::sTestCert_Node01_01_PrivateKey, TestCerts::sTestCert_Node01_01_PrivateKey_Len));
        ReturnErrorOnFailure(mFabricTable.AddNewFabric(aliceFabric, &mAliceFabricIndex));

        bobFabric.TestOnlyBuildFabric(
            ByteSpan(TestCerts::sTestCert_Root02_Chip, TestCerts::sTestCert_Root02_Chip_Len),
            ByteSpan(TestCerts::sTestCert_ICA02_Chip, TestCerts::sTestCert_ICA02_Chip_Len),
            ByteSpan(TestCerts::sTestCert_Node02_01_Chip, TestCerts::sTestCert_Node02_01_Chip_Len),
            ByteSpan(TestCerts::sTestCert_Node02_01_PublicKey, TestCerts::sTestCert_Node02_01_PublicKey_Len),
            ByteSpan(TestCerts::sTestCert_Node02_01_PrivateKey, TestCerts::sTestCert_Node02_01_PrivateKey_Len));
        ReturnErrorOnFailure(mFabricTable.AddNewFabric(bobFabric, &mBobFabricIndex));

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
    return mSessionBobToAlice.Get();
}

SessionHandle MessagingContext::GetSessionAliceToBob()
{
    return mSessionAliceToBob.Get();
}

SessionHandle MessagingContext::GetSessionBobToFriends()
{
    return SessionHandle(mSessionBobToFriends.Value());
}

void MessagingContext::ExpireSessionBobToAlice()
{
    mSessionManager.ExpirePairing(mSessionBobToAlice.Get());
}

void MessagingContext::ExpireSessionAliceToBob()
{
    mSessionManager.ExpirePairing(mSessionAliceToBob.Get());
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
