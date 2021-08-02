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
#include <system/SystemError.h>
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

CHIP_ERROR WatchableSocket::OnAttach()
{
    VerifyOrReturnError(evutil_make_socket_nonblocking(mFD) == 0, MapErrorPOSIX(errno));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WatchableSocket::OnRelease()
{
    CHIP_ERROR status = UpdateWatch(0);
    mSharedState->RemoveFromQueueIfPresent(this);
    return status;
}

CHIP_ERROR WatchableSocket::SetWatch(short eventFlags)
{
    const short oldFlags = mEvent ? event_get_events(mEvent) : 0;
    const short newFlags = static_cast<short>(EV_PERSIST | oldFlags | eventFlags);
    if (oldFlags != newFlags)
    {
        return UpdateWatch(newFlags);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WatchableSocket::ClearWatch(short eventFlags)
{
    const short oldFlags = mEvent ? event_get_events(mEvent) : 0;
    const short newFlags = static_cast<short>(EV_PERSIST | (oldFlags & ~eventFlags));
    if (oldFlags != newFlags)
    {
        return UpdateWatch(newFlags);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WatchableSocket::UpdateWatch(short eventFlags)
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
        VerifyOrReturnError(mEvent != nullptr, CHIP_ERROR_NO_MEMORY);
        VerifyOrReturnError(event_add(mEvent, nullptr) == 0, CHIP_ERROR_INTERNAL);
    }
    return CHIP_NO_ERROR;
}

} // namespace System
} // namespace chip
