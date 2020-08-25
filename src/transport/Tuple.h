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

/**
 * @file
 *
 * Defines a compound transport type (tuple) that can merge several transports
 * to look like a single one.
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
 *
 * The usage intent is to be able to group several distinct transport types and make them look
 * as a single transport.
 *
 * Having an example class definition of `Tuple<UDP, UDP, TCP>`
 *
 * Is equivalent to:
 *
 * ~~~~~~~~~
 *    class TupleOfUdpUdpTcp : public BASE {
 *       private:
 *          UDP mUdp1;
 *          UDP mUdp2;
 *          TCP mTcp3;
 *       public:
 *          Init(UDPListenParameters &, UDPListenParameters&, TCPListenParameters) {...}
 *          CHIP_ERROR SendMessage(...) override;
 *          bool CanSendToPeer(...) override;
 *    }
 * ~~~~~~~~~
 *
 * The intent of this is to allow applications to use any transport types without CHIP pre-defining
 * popular mappings (like UDP only, UDP and BLE, BLE only etc.) and without using #ifdefs to create
 * a single 'standard transport'.
 *
 * Transport logic:
 *   - Every transport can decide if a 'PeerAddress' can be sent over 'self'
 *
 *   - Within a mixed tuple, if several transports support a peer address (e.g. TCP and UDP both
 *     support IP), the first one found wins.
 *
 *   - Expected message sending logic:
 *     - BLE transports only support BLE. TCP/UDP support IP addresses
 *     - Multicasts only supported by UDP
 *
 * @tparam TransportTypes the ORDERED list of transport types grouped together. Order matters in
 *         determining what transport is used when multiple transports can reach a Peer.
 *
 * Transports (TransportTypes) are assumed to have an Init call with EXACTLY one argument and returning
 * a CHIP_ERROR, with a signature like:
 *
 *    CHIP_ERROR Init(AnyType);
 *
 */
template <typename... TransportTypes>
class Tuple : public Base
{
public:
    CHIP_ERROR SendMessage(const MessageHeader & header, const PeerAddress & address, System::PacketBuffer * msgBuf) override
    {
        return SendMessageImpl<0>(header, address, msgBuf);
    }

    bool CanSendToPeer(const PeerAddress & address) override { return CanSendToPeerImpl<0>(address); }

    /**
     * Initialization method that forwards arguments for initialization to each of the underlying
     * transports.
     *
     * Transports are assumed to have an Init call with EXACTLY one argument. This method MUST initialize
     * all underlying transports.
     *
     * @param args initialization arguments, forwarded as-is to the underlying transports.
     */
    template <typename... Args, typename std::enable_if<(sizeof...(Args) == sizeof...(TransportTypes))>::type * = nullptr>
    CHIP_ERROR Init(Args &&... args)
    {
        return InitImpl(std::forward<Args>(args)...);
    }

private:
    /**
     * Recursive cansend implementation iterating through transport members.
     *
     * Will return true if any transport with index N and up can CanSendToPeer(address);
     *
     * @tparam N the index of the underlying transport to check for CanSendToPeer
     *
     * @param address what address to check.
     */
    template <size_t N, typename std::enable_if<(N < sizeof...(TransportTypes))>::type * = nullptr>
    bool CanSendToPeerImpl(const PeerAddress & address)
    {
        return std::get<N>(mTransports).CanSendToPeer(address) || CanSendToPeerImpl<N + 1>(address);
    }

    /**
     * CanSend template for out of range N. Always returns false.
     */
    template <size_t N, typename std::enable_if<(N >= sizeof...(TransportTypes))>::type * = nullptr>
    bool CanSendToPeerImpl(const PeerAddress & address)
    {
        return false;
    }

    /**
     * Recursive sendmessage implementation iterating through transport members.
     *
     * Message is sent through the first transport from index N or above, which returns 'CanSendToPeer'
     *
     * @tparam N the index of the underlying transport to run SendMessage throug.
     *
     * @param header the message header to send
     * @param address where to send the message
     * @param msgBuf the data to send.
     */
    template <size_t N, typename std::enable_if<(N < sizeof...(TransportTypes))>::type * = nullptr>
    CHIP_ERROR SendMessageImpl(const MessageHeader & header, const PeerAddress & address, System::PacketBuffer * msgBuf)
    {
        Base * base = &std::get<N>(mTransports);
        if (base->CanSendToPeer(address))
        {
            return base->SendMessage(header, address, msgBuf);
        }
        return SendMessageImpl<N + 1>(header, address, msgBuf);
    }

    /**
     * SendMessageImpl when N is out of range. Always returns an error code.
     */
    template <size_t N, typename std::enable_if<(N >= sizeof...(TransportTypes))>::type * = nullptr>
    CHIP_ERROR SendMessageImpl(const MessageHeader & header, const PeerAddress & address, System::PacketBuffer * msgBuf)
    {
        System::PacketBuffer::Free(msgBuf);
        return CHIP_ERROR_NO_MESSAGE_HANDLER;
    }

    /**
     * Recursive init implementation iterating through transport members
     *
     * Given a set of arguments 'a1, a2, a3, ... aN' will call an Init method on the last N
     * transports.
     *
     * Method is expected to be called initially with exactly sizeof(TransportTypes) to initialize
     * all transports.
     *
     * @param arg the next initialize argument to pass to the transport Init method
     * @param rest tail arguments to be passed to the rest of transport Init methods.
     */
    template <typename InitArg, typename... Rest>
    CHIP_ERROR InitImpl(InitArg && arg, Rest &&... rest)
    {
        auto transport = &std::get<sizeof...(TransportTypes) - sizeof...(Rest) - 1>(mTransports);

        CHIP_ERROR err = transport->Init(std::forward<InitArg>(arg));
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        transport->SetMessageReceiveHandler(&OnMessageReceive, this);

        return InitImpl(std::forward<Rest>(rest)...);
    }

    /**
     * Base case where initialization finishes.
     *
     * Provided to ensure that recursive InitImpl finishes compiling.
     */
    CHIP_ERROR InitImpl() { return CHIP_NO_ERROR; }

    /**
     * Handler passed to all underlying transports at init time.
     *
     * Calls the underlying Base message receive handler whenever any of the underlying transports
     * receives a message.
     */
    static void OnMessageReceive(MessageHeader & header, const PeerAddress & source, System::PacketBuffer * msg, Tuple * t)
    {
        t->HandleMessageReceived(header, source, msg);
    }

    std::tuple<TransportTypes...> mTransports;
};

} // namespace Transport
} // namespace chip

#endif // TRANSPORT_TUPLE_H_
