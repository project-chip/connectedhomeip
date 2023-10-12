#pragma once

#include <app/DeviceProxy.h>
#include <lib/core/GroupId.h>
#include <messaging/ExchangeMgr.h>
#include <transport/SessionManager.h>

namespace chip {

class GroupDeviceProxy : public DeviceProxy
{
public:
    GroupDeviceProxy(GroupId groupId, FabricIndex fabricIndex, Messaging::ExchangeManager * exchangeMgr) :
        mGroupId(groupId), mExchangeMgr(exchangeMgr)
    {
        chip::Transport::OutgoingGroupSession * session = new chip::Transport::OutgoingGroupSession(groupId, fabricIndex);
        mSecureSession                                  = chip::SessionHandle(*session);
    }
    GroupDeviceProxy() {}

    void Disconnect() override {}

    Messaging::ExchangeManager * GetExchangeManager() const override { return mExchangeMgr; }
    chip::Optional<SessionHandle> GetSecureSession() const override { return mSecureSession.Get(); }
    NodeId GetDeviceId() const override { return NodeIdFromGroupId(mGroupId); }

private:
    bool IsSecureConnected() const override { return static_cast<bool>(mSecureSession); }

    GroupId mGroupId                          = chip::kUndefinedGroupId;
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    SessionHolder mSecureSession;
};

} // namespace chip
