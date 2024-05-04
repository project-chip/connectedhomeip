/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <transport/raw/TCP.h>

using namespace chip::Inet;

namespace chip {
namespace Transport {
/**
 * @brief Class acts as an accessor to private members of the TCPBase class without needing to give
 *        friend access to each individual test.
 */
template <size_t kActiveConnectionsSize, size_t kPendingPacketSize>
class TCPBaseTestAccess
{
public:
    using TCPImpl = Transport::TCP<kActiveConnectionsSize, kPendingPacketSize>;

    static void * FindActiveConnection(TCPImpl & tcp, Transport::PeerAddress & peerAddress)
    {
        return tcp.FindActiveConnection(peerAddress);
    }
    static TCPEndPoint * GetEndpoint(void * state) { return static_cast<TCPBase::ActiveConnectionState *>(state)->mEndPoint; }

    static CHIP_ERROR ProcessReceivedBuffer(TCPImpl & tcp, TCPEndPoint * endPoint, const PeerAddress & peerAddress,
                                            System::PacketBufferHandle && buffer)
    {
        return tcp.ProcessReceivedBuffer(endPoint, peerAddress, std::move(buffer));
    }
};
} // namespace Transport
} // namespace chip
