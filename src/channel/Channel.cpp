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

#include <channel/Channel.h>
#include <channel/ChannelContext.h>
#include <channel/Manager.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Messaging {

ChannelState ChannelHandle::GetState() const
{
    if (mAssociation == nullptr)
        return ChannelState::kNone;
    return mAssociation->mChannelContext->GetState();
}

ExchangeContext * ChannelHandle::NewExchange(ExchangeDelegate * delegate)
{
    assert(mAssociation != nullptr);
    return mAssociation->mChannelContext->NewExchange(delegate);
}

void ChannelHandle::Release()
{
    if (mAssociation == nullptr)
        return;

    mAssociation->mChannelContext->mChannelManager->ReleaseChannelHandle(mAssociation);
    mAssociation = nullptr;
}

} // namespace Messaging
} // namespace chip
