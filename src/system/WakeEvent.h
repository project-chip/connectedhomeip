/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file declares the abstraction of socket (file descriptor) events.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

#include <core/CHIPError.h>
#include <system/WatchableSocket.h>

namespace chip {
namespace System {

class WakeEventTest;
class WatchableEventManager;

/**
 * @class WakeEvent
 *
 * An instance of this type can be used by a WatchableEventManager to allow other threads
 * to wake its event loop thread via WatchableEventManager::Signal().
 */
class WakeEvent
{
public:
    CHIP_ERROR Open(WatchableEventManager & watchState); /**< Initialize the pipeline */
    void Close();                                        /**< Close both ends of the pipeline. */

    CHIP_ERROR Notify(); /**< Set the event. */
    void Confirm();      /**< Clear the event. */

private:
    friend class WakeEventTest;

    int GetReadFD() const { return mFD.GetFD(); }
    static void Confirm(WatchableSocket & socket) { reinterpret_cast<WakeEvent *>(socket.GetCallbackData())->Confirm(); }

#if CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE
    int mWriteFD;
#endif
    WatchableSocket mFD;
};

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
