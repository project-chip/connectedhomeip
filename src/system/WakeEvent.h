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

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && !CHIP_SYSTEM_CONFIG_USE_LIBEV

#include <lib/core/CHIPError.h>
#include <system/SocketEvents.h>

namespace chip {
namespace System {

class LayerSockets;
class WakeEventTest;

/**
 * @class WakeEvent
 *
 * An instance of this type can be used by a System::Layer to allow other threads
 * to wake its event loop thread via System::Layer::Signal().
 */
class WakeEvent
{
public:
    CHIP_ERROR Open(LayerSockets & systemLayer); /**< Initialize the pipeline */
    void Close(LayerSockets & systemLayer);      /**< Close both ends of the pipeline. */

    CHIP_ERROR Notify() const; /**< Set the event. */
    void Confirm() const;      /**< Clear the event. */

private:
    friend class WakeEventTest;

    int GetReadFD() const { return mReadFD; }
    static void Confirm(System::SocketEvents events, intptr_t data) { reinterpret_cast<WakeEvent *>(data)->Confirm(); }

#if CHIP_SYSTEM_CONFIG_USE_POSIX_PIPE
    int mWriteFD;
#endif
    int mReadFD;
    SocketWatchToken mReadWatch;
};

} // namespace System
} // namespace chip

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && !CHIP_SYSTEM_CONFIG_USE_LIBEV
