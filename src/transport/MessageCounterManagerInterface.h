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

#pragma once

#include <transport/PeerConnectionState.h>
#include <transport/SecureSessionHandle.h>

namespace chip {
namespace Transport {

class MessageCounterManagerInterface
{
public:
    virtual ~MessageCounterManagerInterface() {}

    /**
     * Start sync if the sync procedure is not started yet.
     */
    virtual CHIP_ERROR StartSync(SecureSessionHandle session, Transport::PeerConnectionState * state) = 0;

    /**
     * Called when have received a message but session message counter is not synced.  It will queue the message and start sync if
     * the sync procedure is not started yet.
     */
    virtual CHIP_ERROR QueueReceivedMessageAndStartSync(const PacketHeader & packetHeader, SecureSessionHandle session,
                                                        Transport::PeerConnectionState * state,
                                                        const Transport::PeerAddress & peerAddress,
                                                        System::PacketBufferHandle && msgBuf) = 0;
};

} // namespace Transport
} // namespace chip
