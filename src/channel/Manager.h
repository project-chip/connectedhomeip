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

#include <array>

#include <channel/Channel.h>
#include <channel/ChannelContext.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Pool.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Messaging {

class ChannelContext;

/**
 *  @brief
 *    This class is used to manage Channel Contexts with other CHIP nodes.
 */
class DLL_EXPORT ChannelManager : public ExchangeMgrDelegate
{
public:
    ChannelManager(ExchangeManager * exchangeManager) : mExchangeManager(exchangeManager) { exchangeManager->SetDelegate(this); }
    ChannelManager(const ChannelManager &) = delete;
    ChannelManager operator=(const ChannelManager &) = delete;

    ChannelHandle EstablishChannel(const ChannelBuilder & builder, ChannelDelegate * delegate);

    // Internal APIs used for channel
    void ReleaseChannelContext(ChannelContext * channel) { mChannelContexts.ReleaseObject(channel); }

    void ReleaseChannelHandle(ChannelContextHandleAssociation * association) { mChannelHandles.ReleaseObject(association); }

    template <typename Event>
    void NotifyChannelEvent(ChannelContext * channel, Event event)
    {
        mChannelHandles.ForEachActiveObject([&](ChannelContextHandleAssociation * association) {
            if (association->mChannelContext == channel)
                event(association->mChannelDelegate);
            return true;
        });
    }

    void OnNewConnection(SessionHandle session, ExchangeManager * mgr) override
    {
        mChannelContexts.ForEachActiveObject([&](ChannelContext * context) {
            if (context->MatchesSession(session, mgr->GetSessionMgr()))
            {
                context->OnNewConnection(session);
                return false;
            }
            return true;
        });
    }

    void OnConnectionExpired(SessionHandle session, ExchangeManager * mgr) override
    {
        mChannelContexts.ForEachActiveObject([&](ChannelContext * context) {
            if (context->MatchesSession(session, mgr->GetSessionMgr()))
            {
                context->OnConnectionExpired(session);
                return false;
            }
            return true;
        });
    }

private:
    BitMapObjectPool<ChannelContext, CHIP_CONFIG_MAX_ACTIVE_CHANNELS> mChannelContexts;
    BitMapObjectPool<ChannelContextHandleAssociation, CHIP_CONFIG_MAX_CHANNEL_HANDLES> mChannelHandles;
    ExchangeManager * mExchangeManager;
};

} // namespace Messaging
} // namespace chip
