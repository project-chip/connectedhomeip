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
 *      This file declares an implementation of WatchableSocket using select().
 */

#pragma once

#if !INCLUDING_CHIP_SYSTEM_WATCHABLE_SOCKET_CONFIG_FILE
#error "This file should only be included from <system/WatchableSocket.h>"
#include <system/WatchableSocket.h>
#endif //  !INCLUDING_CHIP_SYSTEM_WATCHABLE_SOCKET_CONFIG_FILE

#include <sys/select.h>

#include <support/BitFlags.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace System {

class WatchableSocket : public WatchableSocketBasis<WatchableSocket>
{
public:
    void OnInit() { mAttachedNext = nullptr; }
    CHIP_ERROR OnAttach();
    CHIP_ERROR OnRelease();

    CHIP_ERROR OnRequestCallbackOnPendingRead();
    CHIP_ERROR OnRequestCallbackOnPendingWrite();
    CHIP_ERROR OnClearCallbackOnPendingRead();
    CHIP_ERROR OnClearCallbackOnPendingWrite();

    void SetPendingIO(SocketEvents events) { mPendingIO = events; }
    void SetFDs(int & nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds);

private:
    friend class WatchableEventManager;

    WatchableSocket * mAttachedNext; ///< Next element in the list of sockets attached to the WatchableEventManager.
};

} // namespace System
} // namespace chip
