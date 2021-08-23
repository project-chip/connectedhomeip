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
 *      This file declares an implementation of WatchableEventManager using libevent.
 */

#pragma once

#if !INCLUDING_CHIP_SYSTEM_WATCHABLE_SOCKET_CONFIG_FILE
#error "This file should only be included from <system/WatchableSocket.h>"
#include <system/WatchableSocket.h>
#endif //  !INCLUDING_CHIP_SYSTEM_WATCHABLE_SOCKET_CONFIG_FILE

#include <event2/event.h>

namespace chip {

namespace System {

class WatchableEventManager;

class WatchableSocket : public WatchableSocketBasis<WatchableSocket>
{
public:
    void OnInit();
    CHIP_ERROR OnAttach();
    CHIP_ERROR OnRelease();
    CHIP_ERROR OnRequestCallbackOnPendingRead() { return SetWatch(EV_READ); }
    CHIP_ERROR OnRequestCallbackOnPendingWrite() { return SetWatch(EV_WRITE); }
    CHIP_ERROR OnClearCallbackOnPendingRead() { return ClearWatch(EV_READ); }
    CHIP_ERROR OnClearCallbackOnPendingWrite() { return ClearWatch(EV_WRITE); }

private:
    friend class WatchableEventManager;

    CHIP_ERROR SetWatch(short eventFlags);
    CHIP_ERROR ClearWatch(short eventFlags);
    CHIP_ERROR UpdateWatch(short eventFlags);

    WatchableSocket * mActiveNext; ///< Next element in the list of sockets activated by libevent.
    struct event * mEvent;         ///< libevent state.
};

} // namespace System
} // namespace chip
