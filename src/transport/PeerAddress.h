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
 * @brief
 *    File contains definitions on how a connection to a peer can be defined.
 *
 */

#ifndef PEER_ADDRESS_H_
#define PEER_ADDRESS_H_

#include <stdio.h>

#include <inet/IPAddress.h>

namespace chip {
namespace Transport {

/**
 * Communication path defines how two peers communicate.
 *
 * When a peer contacts another peer, it defines how the peers communicate.
 *
 * Once communication between two peers is established, the same transport
 * path should be used: a peer contacting another peer over UDP will receive
 * messages back over UDP. A communication channel established over TCP
 * will keep the same TCP channel.
 *
 */
enum class Type
{
    kUndefined,
    kUdp,
    kBle,
    // More constants to be added later, such as TCP
};

/**
 * Describes how a peer on a CHIP network can be addressed.
 */
class PeerAddress
{
public:
    PeerAddress(const Inet::IPAddress & addr, Type type) : mIPAddress(addr), mTransportType(type) {}
    PeerAddress(Type type) : mTransportType(type) {}

    PeerAddress(PeerAddress &&)      = default;
    PeerAddress(const PeerAddress &) = default;
    PeerAddress & operator=(const PeerAddress &) = default;
    PeerAddress & operator=(PeerAddress &&) = default;

    const Inet::IPAddress & GetIPAddress() const { return mIPAddress; }
    PeerAddress & SetIPAddress(const Inet::IPAddress & addr)
    {
        mIPAddress = addr;
        return *this;
    }

    Type GetTransportType() const { return mTransportType; }
    PeerAddress & SetTransportType(Type type)
    {
        mTransportType = type;
        return *this;
    }

    uint16_t GetPort() const { return mPort; }
    PeerAddress & SetPort(uint16_t port)
    {
        mPort = port;
        return *this;
    }

    bool IsInitialized() const { return mTransportType != Type::kUndefined; }

    bool operator==(const PeerAddress & other)
    {
        return (mTransportType == other.mTransportType) && (mIPAddress == other.mIPAddress) && (mPort == other.mPort);
    }

    /// Maximum size of an Inet address ToString format, that can hold both IPV6 and IPV4 addresses.
#ifdef INET6_ADDRSTRLEN
    static constexpr size_t kInetMaxAddrLen = INET6_ADDRSTRLEN;
#else
    static constexpr size_t kInetMaxAddrLen = INET_ADDRSTRLEN;
#endif

    /// Maximum size of the string outputes by ToString. Format is of the form:
    /// "UDP:<ip>:<port>"
    static constexpr size_t kMaxToStringSize = //
        3 /* UDP/TCP/BLE */ + 1 /* : */        //
        + kInetMaxAddrLen + 1 /* : */          //
        + 5 /* 16 bit interger */              //
        + 1 /* NullTerminator */;

    void ToString(char * buf, size_t bufSize) const
    {
        char ip_addr[kInetMaxAddrLen];

        switch (mTransportType)
        {
        case Type::kUndefined:
            snprintf(buf, bufSize, "UNDEFINED");
            break;
        case Type::kUdp:
            mIPAddress.ToString(ip_addr, sizeof(ip_addr));
            snprintf(buf, bufSize, "UDP:%s:%d", ip_addr, mPort);
            break;
        default:
            snprintf(buf, bufSize, "ERROR");
            break;
        }
    }

    /****** Factory methods for convenience ******/

    static PeerAddress Uninitialized() { return PeerAddress(Inet::IPAddress::Any, Type::kUndefined); }

    static PeerAddress BLE() { return PeerAddress(Type::kBle); }
    static PeerAddress UDP(const Inet::IPAddress & addr) { return PeerAddress(addr, Type::kUdp); }
    static PeerAddress UDP(const Inet::IPAddress & addr, uint16_t port) { return UDP(addr).SetPort(port); }

private:
    Inet::IPAddress mIPAddress;
    Type mTransportType;
    uint16_t mPort = CHIP_PORT; ///< Relevant for UDP data sending.
};

} // namespace Transport
} // namespace chip

#endif // PEER_ADDRESS_H_
