/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/support/BitFlags.h>

namespace chip {
namespace System {

enum class SocketEventFlags : uint8_t
{
    kRead   = 0x1, /**< Bit flag indicating if there is a read event on a socket. */
    kWrite  = 0x2, /**< Bit flag indicating if there is a write event on a socket. */
    kExcept = 0x4, /**< Bit flag indicating if there is an exceptional condition on a socket (e.g. out-of-band data). */
    kError  = 0x8, /**< Bit flag indicating if there is an error event on a socket. */
};

using SocketEvents = BitFlags<SocketEventFlags>;

using SocketWatchToken    = intptr_t;
using SocketWatchCallback = void (*)(SocketEvents events, intptr_t data);

constexpr int kInvalidFd = -1;

} // namespace System
} // namespace chip
