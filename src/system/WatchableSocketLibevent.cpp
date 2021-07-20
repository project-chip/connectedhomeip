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

/**
 *    @file
 *      This file implements WatchableSocket using libevent.
 */

#include <platform/CHIPDeviceBuildConfig.h>
#include <support/CodeUtils.h>
#include <system/SystemLayer.h>
#include <system/WatchableEventManager.h>
#include <system/WatchableSocket.h>

namespace chip {
namespace System {

void WatchableSocket::OnInit()
{
    mEvent      = nullptr;
    mActiveNext = nullptr;
}

void WatchableSocket::OnAttach()
{
    evutil_make_socket_nonblocking(mFD);
}

void WatchableSocket::OnClose()
{
    UpdateWatch(0);
    mSharedState->RemoveFromQueueIfPresent(this);
}

void WatchableSocket::SetWatch(short eventFlags)
{
    const short oldFlags = mEvent ? event_get_events(mEvent) : 0;
    const short newFlags = static_cast<short>(EV_PERSIST | oldFlags | eventFlags);
    if (oldFlags != newFlags)
    {
        UpdateWatch(newFlags);
    }
}

void WatchableSocket::ClearWatch(short eventFlags)
{
    const short oldFlags = mEvent ? event_get_events(mEvent) : 0;
    const short newFlags = static_cast<short>(EV_PERSIST | (oldFlags & ~eventFlags));
    if (oldFlags != newFlags)
    {
        UpdateWatch(newFlags);
    }
}

void WatchableSocket::UpdateWatch(short eventFlags)
{
    if (mEvent)
    {
        event_del(mEvent);
        event_free(mEvent);
        mEvent = nullptr;
    }
    if (eventFlags)
    {
        event_base * const base = mSharedState->mEventBase;
        mEvent                  = event_new(base, mFD, eventFlags, WatchableEventManager::LibeventCallbackHandler, this);
        event_add(mEvent, nullptr);
    }
}

} // namespace System
} // namespace chip
