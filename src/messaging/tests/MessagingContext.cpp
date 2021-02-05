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

#include <messaging/tests/MessagingContext.h>

#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>

namespace chip {
namespace Test {

CHIP_ERROR MessagingContext::Init(nlTestSuite * suite, TransportMgrBase * transport)
{
    ReturnErrorOnFailure(IOContext::Init(suite));

    mAdmins.Reset();

    chip::Transport::AdminPairingInfo * srcNodeAdmin = mAdmins.AssignAdminId(mSrcAdminId, GetSourceNodeId());
    VerifyOrReturnError(srcNodeAdmin != nullptr, CHIP_ERROR_NO_MEMORY);

    chip::Transport::AdminPairingInfo * destNodeAdmin = mAdmins.AssignAdminId(mDestAdminId, GetDestinationNodeId());
    VerifyOrReturnError(destNodeAdmin != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(mSecureSessionMgr.Init(GetSourceNodeId(), &GetSystemLayer(), transport, &mAdmins));

    ReturnErrorOnFailure(mExchangeManager.Init(&mSecureSessionMgr));

    ReturnErrorOnFailure(mSecureSessionMgr.NewPairing(mPeer, GetDestinationNodeId(), &mPairingLocalToPeer, mSrcAdminId));

    return mSecureSessionMgr.NewPairing(mPeer, GetSourceNodeId(), &mPairingPeerToLocal, mDestAdminId);
}

// Shutdown all layers, finalize operations
CHIP_ERROR MessagingContext::Shutdown()
{
    return IOContext::Shutdown();
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToPeer(Messaging::ExchangeDelegate * delegate)
{
    // TODO: temprary create a SecureSessionHandle from node id, will be fix in PR 3602
    return mExchangeManager.NewContext({ GetDestinationNodeId(), GetPeerKeyId() }, delegate);
}

Messaging::ExchangeContext * MessagingContext::NewExchangeToLocal(Messaging::ExchangeDelegate * delegate)
{
    // TODO: temprary create a SecureSessionHandle from node id, will be fix in PR 3602
    return mExchangeManager.NewContext({ GetSourceNodeId(), GetLocalKeyId() }, delegate);
}

} // namespace Test
} // namespace chip
