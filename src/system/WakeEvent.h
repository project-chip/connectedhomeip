/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
