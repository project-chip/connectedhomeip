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

#include <lib/core/CHIPConfig.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

class Session
{
public:
    virtual ~Session() {}

    enum class SessionType : uint8_t
    {
        kUndefined       = 0,
        kUnauthenticated = 1,
        kSecure          = 2,
    };

    virtual SessionType GetSessionType() const = 0;
#if CHIP_PROGRESS_LOGGING
    virtual const char * GetSessionTypeString() const = 0;
#endif
    virtual NodeId GetPeerNodeId() const                          = 0;
    virtual const Transport::PeerAddress & GetPeerAddress() const = 0;
};

} // namespace chip
