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

#include <channel/Manager.h>

namespace chip {
namespace Messaging {

ChannelHandle ChannelManager::EstablishChannel(const ChannelBuilder & builder, ChannelDelegate * delegate)
{
    ChannelContext * channelContext = nullptr;

    // Find an existing Channel matching the builder
    mChannelContexts.ForEachActiveObject([&](ChannelContext * context) {
        if (context->MatchesBuilder(builder))
        {
            channelContext = context;
            return false;
        }
        return true;
    });

    if (channelContext == nullptr)
    {
        // create a new channel if not found
        channelContext = mChannelContexts.CreateObject(mExchangeManager, this);
        if (channelContext == nullptr)
            return ChannelHandle{ nullptr };
        channelContext->Start(builder);
    }
    else
    {
        channelContext->Retain();
    }

    ChannelContextHandleAssociation * association = mChannelHandles.CreateObject(channelContext, delegate);
    channelContext->Release();
    return ChannelHandle{ association };
}

} // namespace Messaging
} // namespace chip
