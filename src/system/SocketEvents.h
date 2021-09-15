/*
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
