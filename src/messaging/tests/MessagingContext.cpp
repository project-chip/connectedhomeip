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
    ReturnErrorOnFailure(mSessionManager.Init(&GetSystemLayer(), transport, &mMessageCounterManager, &mStorage));

    ReturnErrorOnFailure(mExchangeManager.Init(&mSessionManager));
    ReturnErrorOnFailure(mMessageCounterManager.Init(&mExchangeManager));

    ReturnErrorOnFailure(CreateSessionBobToAlice());
    ReturnErrorOnFailure(CreateSessionAliceToBob());
    ReturnErrorOnFailure(CreateSessionBobToFriends());

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
    return mSessionManager.NewPairing(mSessionBobToAlice, Optional<Transport::PeerAddress>::Value(mAliceAddress), GetAliceNodeId(),
                                      &mPairingBobToAlice, CryptoContext::SessionRole::kInitiator, mSrcFabricIndex);
}

CHIP_ERROR MessagingContext::CreateSessionAliceToBob()
{
    return mSessionManager.NewPairing(mSessionAliceToBob, Optional<Transport::PeerAddress>::Value(mBobAddress), GetBobNodeId(),
                                      &mPairingAliceToBob, CryptoContext::SessionRole::kResponder, mDestFabricIndex);
}

CHIP_ERROR MessagingContext::CreateSessionBobToFriends()
{
    mSessionBobToFriends.Grab(mSessionManager.CreateGroupSession(GetFriendsGroupId(), mSrcFabricIndex, GetBobNodeId()).Value());
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
    return mSessionBobToFriends.Get();
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
    mSessionManager.RemoveGroupSession(mSessionBobToFriends.Get()->AsGroupSession());
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
