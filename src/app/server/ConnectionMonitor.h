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

#pragma once

#include "StorablePeerConnection.h"

#include <messaging/ExchangeMgr.h>
#include <messaging/ExchangeMgrDelegate.h>
#include <protocols/secure_channel/SessionIDAllocator.h>
#include <support/CodeUtils.h>

namespace chip {
class PersistentStorageDelegate;

class ConnectionMonitor : public Messaging::ExchangeMgrDelegate
{
public:
    ConnectionMonitor(PersistentStorageDelegate & storage, SessionIDAllocator & idAllocator) :
        mStorage(storage), mIDAllocator(idAllocator)
    {}

    void OnNewConnection(SecureSessionHandle session, Messaging::ExchangeManager * mgr) override
    {
        const auto connState = mgr->GetSessionMgr()->GetPeerConnectionState(session);
        VerifyOrReturn(connState != nullptr);

        StorablePeerConnection connection;
        VerifyOrReturn(connection.Init(*connState) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to init serializable connection state"));
        VerifyOrReturn(connection.StoreIntoKVS(mStorage) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to store connection state"));

        // The Peek() is used to find the smallest key ID that's not been assigned to any session.
        // This value is persisted, and on reboot, it is used to revive any previously
        // active secure sessions.
        // We support one active PASE session at any time. So the key ID should not be updated
        // in another thread, while we retrieve it here.
        VerifyOrReturn(StorablePeerConnection::StoreCountIntoKVS(mStorage, mIDAllocator.Peek()) == CHIP_NO_ERROR,
                       ChipLogError(AppServer, "Failed to store connection count"));
    }

    void OnConnectionExpired(SecureSessionHandle session, Messaging::ExchangeManager * mgr) override
    {
        const auto connState = mgr->GetSessionMgr()->GetPeerConnectionState(session);
        VerifyOrReturn(connState != nullptr);
        StorablePeerConnection::DeleteFromKVS(mStorage, connState->GetLocalKeyID());
    }

private:
    PersistentStorageDelegate & mStorage;
    SessionIDAllocator & mIDAllocator;
};
} // namespace chip
