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

namespace chip {
namespace Test {

CHIP_ERROR MessagingContext::Init(nlTestSuite * suite, TransportMgrBase * transport)
{
    CHIP_ERROR err = IOContext::Init(suite);
    SuccessOrExit(err);

    err = mSecureSessionMgr.Init(GetSourceNodeId(), &GetSystemLayer(), transport);
    SuccessOrExit(err);

    err = mExchangeManager.Init(GetSourceNodeId(), transport, &mSecureSessionMgr);
    SuccessOrExit(err);

    err = mSecureSessionMgr.NewPairing(mPeer, GetDestinationNodeId(), &mPairingLocalToPeer);
    SuccessOrExit(err);

    err = mSecureSessionMgr.NewPairing(mPeer, GetSourceNodeId(), &mPairingPeerToLocal);
    SuccessOrExit(err);

exit:
    return err;
}

// Shutdown all layers, finalize operations
CHIP_ERROR MessagingContext::Shutdown()
{
    CHIP_ERROR err = IOContext::Shutdown();

    return err;
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
