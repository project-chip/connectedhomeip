/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <messaging/Channel.h>
#include <messaging/ChannelContext.h>
#include <messaging/ExchangeMgr.h>

namespace chip {
namespace Messaging {

ChannelState ChannelHandle::GetState() const
{
    if (mAssociation != nullptr)
        return mAssociation->mChannelContext->GetState();
    else
        return ChannelState::kChanneState_None;
}

ExchangeContext * ChannelHandle::NewExchange(ExchangeDelegate * delegate)
{
    if (mAssociation != nullptr)
        return mAssociation->mChannelContext->NewExchange(delegate);
    else
        return nullptr;
}

void ChannelHandle::Release()
{
    if (mAssociation != nullptr)
        mAssociation->mChannelContext->mExchangeManager->ReleaseChannelHandle(mAssociation);
    mAssociation = nullptr;
}

} // namespace Messaging
} // namespace chip
