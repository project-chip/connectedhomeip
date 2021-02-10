/* See Project CHIP LICENSE file for licensing information. */

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
