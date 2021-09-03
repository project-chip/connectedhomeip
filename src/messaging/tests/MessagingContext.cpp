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

    mFabrics.Reset();

    ReturnErrorOnFailure(mSecureSessionMgr.Init(&GetSystemLayer(), transport, &mFabrics, &mMessageCounterManager));

    ReturnErrorOnFailure(mExchangeManager.Init(&mSecureSessionMgr));
    ReturnErrorOnFailure(mMessageCounterManager.Init(&mExchangeManager));

    ReturnErrorOnFailure(mSecureSessionMgr.NewPairing(mPeer, GetDestinationNodeId(), &mPairingLocalToPeer,
                                                      SecureSession::SessionRole::kInitiator, mSrcFabricIndex));

    return mSecureSessionMgr.NewPairing(mPeer, GetSourceNodeId(), &mPairingPeerToLocal, SecureSession::SessionRole::kResponder,
                                        mDestFabricIndex);
}

// Shutdown all layers, finalize operations
CHIP_ERROR MessagingContext::Shutdown()
{
    VerifyOrReturnError(mInitialized == true, CHIP_ERROR_INTERNAL);
    mInitialized = false;

    mExchangeManager.Shutdown();
    mSecureSessionMgr.Shutdown();
    return CHIP_NO_ERROR;
}

SessionHandle MessagingContext::GetSessionLocalToPeer()
{
    // TODO: temporarily create a SessionHandle from node id, will be fixed in PR 3602
    return SessionHandle(GetDestinationNodeId(), GetLocalKeyId(), GetPeerKeyId(), GetFabricIndex());
}

SessionHandle MessagingContext::GetSessionPeerToLocal()
{
    // TODO: temporarily create a SessionHandle from node id, will be fixed in PR 3602
    return SessionHandle(GetSourceNodeId(), GetPeerKeyId(), GetLocalKeyId(), mDestFabricIndex);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToPeer(Messaging::ExchangeDelegate * delegate)
{
    // TODO: temprary create a SessionHandle from node id, will be fix in PR 3602
    return mExchangeManager.NewContext(GetSessionLocalToPeer(), delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToLocal(Messaging::ExchangeDelegate * delegate)
{
    // TODO: temprary create a SessionHandle from node id, will be fix in PR 3602
    return mExchangeManager.NewContext(GetSessionPeerToLocal(), delegate);
}

} // namespace Test
} // namespace chip
