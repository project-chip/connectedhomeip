/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef TRANSPORT_TUPLE_H_
#define TRANSPORT_TUPLE_H_

#include <tuple>
#include <type_traits>

#include <transport/Base.h>

namespace chip {
namespace Transport {

/**
 * Groups together several transports of different types and presents them as a unified transport.
 */
template <typename... TransportTypes>
class Tuple : public Base
{
public:
    CHIP_ERROR SendMessage(const MessageHeader & header, const Transport::PeerAddress & address,
                           System::PacketBuffer * msgBuf) override
    {
        return SendMessageImpl<sizeof...(TransportTypes)>(header, address, msgBuf);
    }

    bool CanSendToPeer(const Transport::PeerAddress & address) override
    {
        return CanSendToPeerImpl<sizeof...(TransportTypes)>(address);
    }

    /**
     * Initialization method that forwards arguments for initialization to each of the underlying
     * transports.
     */
    template <typename... Args>
    CHIP_ERROR Init(Args &&... args)
    {
        return InitImpl(std::forward<Args>(args)...);
    }

private:
    /// Recursive cansend implementation iterating through transport members
    template <size_t N, typename std::enable_if<(N > 0)>::type * = nullptr>
    bool CanSendToPeerImpl(const Transport::PeerAddress & address)
    {
        return std::get<N - 1>(mTransports).CanSendToPeer(address) || CanSendToPeerImpl<N - 1>(address);
    }

    /// Final can send check , where message cannot be sent through any transport
    template <size_t N, typename std::enable_if<(N == 0)>::type * = nullptr>
    bool CanSendToPeerImpl(const Transport::PeerAddress & address)
    {
        return false;
    }

    /// Recursive sendmessage implementation iterating through transport members
    template <size_t N, typename std::enable_if<(N > 0)>::type * = nullptr>
    CHIP_ERROR SendMessageImpl(const MessageHeader & header, const Transport::PeerAddress & address, System::PacketBuffer * msgBuf)
    {
        Base * base = &std::get<N - 1>(mTransports);
        if (base->CanSendToPeer(address))
        {
            return base->SendMessage(header, address, msgBuf);
        }
        return SendMessageImpl<N - 1>(header, address, msgBuf);
    }

    /// Final send message, where message cannot be sent through any transport
    template <size_t N, typename std::enable_if<(N == 0)>::type * = nullptr>
    CHIP_ERROR SendMessageImpl(const MessageHeader & header, const Transport::PeerAddress & address, System::PacketBuffer * msgBuf)
    {
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    /// Recursive init implementation iterating through transport members
    template <typename InitArg, typename... Rest>
    CHIP_ERROR InitImpl(InitArg && arg, Rest &&... rest)
    {
        CHIP_ERROR err = std::get<sizeof...(TransportTypes) - sizeof...(Rest) - 1>(mTransports).Init(std::forward<InitArg>(arg));
        if (err == CHIP_NO_ERROR)
        {
            return err;
        }

        return InitImpl(std::forward<Rest>(rest)...);
    }

    /// Base case where initialization finishes.
    CHIP_ERROR InitImpl() { return CHIP_NO_ERROR; }

    std::tuple<TransportTypes...> mTransports;
};

} // namespace Transport
} // namespace chip

#endif // TRANSPORT_TUPLE_H_
