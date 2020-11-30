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

namespace chip {
namespace Messaging {

ChannelState ChannelHandle::GetState()
{
    if (mChannelContext != nullptr)
        // TODO: not implemented: return mChannelContext->GetState();
        return ChannelState::kChanneState_None;
    else
        return ChannelState::kChanneState_None;
}

void ChannelHandle::Release()
{
    // TODO: not implemented
}

} // namespace Messaging
} // namespace chip
