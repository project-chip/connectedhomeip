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

CHIP_ERROR MessagingContext::Init(nlTestSuite * suite, TransportMgrBase * transport, IOContext * ioContext)
{
    VerifyOrReturnError(mInitialized == false, CHIP_ERROR_INTERNAL);
    mInitialized = true;

    mIOContext = ioContext;

    ReturnErrorOnFailure(mSessionManager.Init(&GetSystemLayer(), transport, &mMessageCounterManager));

    ReturnErrorOnFailure(mExchangeManager.Init(&mSessionManager));
    ReturnErrorOnFailure(mMessageCounterManager.Init(&mExchangeManager));

    ReturnErrorOnFailure(mSessionManager.NewPairing(Optional<Transport::PeerAddress>::Value(mAliceAddress), GetAliceNodeId(),
                                                    &mPairingBobToAlice, CryptoContext::SessionRole::kInitiator, mSrcFabricIndex));

    return mSessionManager.NewPairing(Optional<Transport::PeerAddress>::Value(mBobAddress), GetBobNodeId(), &mPairingAliceToBob,
                                      CryptoContext::SessionRole::kResponder, mDestFabricIndex);
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

SessionHandle MessagingContext::GetSessionBobToAlice()
{
    // TODO: temporarily create a SessionHandle from node id, will be fixed in PR 3602
    return SessionHandle(GetAliceNodeId(), GetBobKeyId(), GetAliceKeyId(), GetFabricIndex());
}

SessionHandle MessagingContext::GetSessionAliceToBob()
{
    // TODO: temporarily create a SessionHandle from node id, will be fixed in PR 3602
    return SessionHandle(GetBobNodeId(), GetAliceKeyId(), GetBobKeyId(), mDestFabricIndex);
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToAlice(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(mSessionManager.CreateUnauthenticatedSession(mAliceAddress).Value(), delegate);
}

Messaging::ExchangeContext * MessagingContext::NewUnauthenticatedExchangeToBob(Messaging::ExchangeDelegate * delegate)
{
    return mExchangeManager.NewContext(mSessionManager.CreateUnauthenticatedSession(mBobAddress).Value(), delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToAlice(Messaging::ExchangeDelegate * delegate)
{
    // TODO: temprary create a SessionHandle from node id, will be fix in PR 3602
    return mExchangeManager.NewContext(GetSessionBobToAlice(), delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToBob(Messaging::ExchangeDelegate * delegate)
{
    // TODO: temprary create a SessionHandle from node id, will be fix in PR 3602
    return mExchangeManager.NewContext(GetSessionAliceToBob(), delegate);
}

} // namespace Test
} // namespace chip
