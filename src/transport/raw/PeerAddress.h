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

#pragma once

#include <stdio.h>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/PeerId.h>
#include <lib/support/CHIPMemString.h>

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

/**
 * Here we specified Type to be uint8_t, so the PeerAddress can be serialized easily.
 */
enum class Type : uint8_t
{
    kUndefined,
    kUdp,
    kBle,
    kTcp,
};

/**
 * Describes how a peer on a CHIP network can be addressed.
 */
class PeerAddress
{
public:
    // When using DNSSD, a peer may be discovered at multiple addresses.
    // This controls how many addresses can be kept.
    static constexpr unsigned kMaxPeerIPAddresses = 5;

    PeerAddress() : mTransportType(Type::kUndefined) {}
    PeerAddress(const Inet::IPAddress & addr, Type type) : mNumValidIPAddresses(1), mTransportType(type) { mIPAddresses[0] = addr; }
    PeerAddress(Type type) : mTransportType(type) {}

    PeerAddress(PeerAddress &&)      = default;
    PeerAddress(const PeerAddress &) = default;
    PeerAddress & operator=(const PeerAddress &) = default;
    PeerAddress & operator=(PeerAddress &&) = default;

    const Inet::IPAddress & GetIPAddress() const
    {
        if (mNumValidIPAddresses > 0)
        {
            return mIPAddresses[0];
        }
        else
        {
            return Inet::IPAddress::Any; // not initialized really
        }
    }

    /// DEPRECATED: this method does not take into account that peers may be
    /// reachable via multiple IP addresses via DNSsd.
    PeerAddress & SetSingleIPAddress(const Inet::IPAddress & addr)
    {
        mIPAddresses[0]      = addr;
        mNumValidIPAddresses = 1;
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

    Inet::InterfaceId GetInterface() const { return mInterface; }
    PeerAddress & SetInterface(Inet::InterfaceId interface)
    {
        mInterface = interface;
        return *this;
    }

    bool IsInitialized() const { return mTransportType != Type::kUndefined; }

    bool IsMulticast()
    {
        if (Type::kUdp != mTransportType)
        {
            return false;
        }

        if (mNumValidIPAddresses == 0)
        {
            return false;
        }

        // Assumption here is that if any IP address is multicast, the entire object should
        // be multicast
        return mIPAddresses[0].IsIPv6Multicast();
    }

    bool operator==(const PeerAddress & other) const
    {
        if ((mTransportType != other.mTransportType) || (mPort != other.mPort) || (mInterface != other.mInterface) ||
            mNumValidIPAddresses != other.mNumValidIPAddresses)
        {
            return false;
        }

        // NOTE: we do NOT try to oder IP addresses here, we check that the valid
        // IP addresses are identical only.
        for (unsigned i = 0; i < mNumValidIPAddresses; i++)
        {
            if (mIPAddresses[i] != other.mIPAddresses[i])
            {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const PeerAddress & other) const { return !(*this == other); }

    /// Maximum size of the string outputes by ToString. Format is of the form:
    /// "UDP:<ip>:<port> (+N more)"
    static constexpr size_t kMaxToStringSize = 3 // type: UDP/TCP/BLE
        + 1                                      // splitter :
        + 2                                      // brackets around address
        + Inet::IPAddress::kMaxStringLength      // address
        + 1                                      // splitter %
        + Inet::InterfaceId::kMaxIfNameLength    // interface
        + 1                                      // splitter :
        + 5                                      // port: 16 bit interger
        + 3                                      // " (+"
        + 2                                      // integer for "more", low digits expected
        + 6                                      // " more)"
        + 1;                                     // NullTerminator

    static_assert(kMaxPeerIPAddresses < 100, "for maxStringSize to be correct for the 'more' part");

    template <size_t N>
    inline void ToString(char (&buf)[N]) const
    {
        ToString(buf, N);
    }

    void ToString(char * buf, size_t bufSize) const;

    /****** Factory methods for convenience ******/

    static PeerAddress Uninitialized() { return PeerAddress(Type::kUndefined); }

    static PeerAddress BLE() { return PeerAddress(Type::kBle); }
    static PeerAddress UDP(const Inet::IPAddress & addr) { return PeerAddress(addr, Type::kUdp); }
    static PeerAddress UDP(const Inet::IPAddress & addr, uint16_t port) { return UDP(addr).SetPort(port); }
    static PeerAddress UDP(const Inet::IPAddress & addr, uint16_t port, Inet::InterfaceId interface)
    {
        return UDP(addr).SetPort(port).SetInterface(interface);
    }
    static PeerAddress TCP(const Inet::IPAddress & addr) { return PeerAddress(addr, Type::kTcp); }
    static PeerAddress TCP(const Inet::IPAddress & addr, uint16_t port) { return TCP(addr).SetPort(port); }
    static PeerAddress TCP(const Inet::IPAddress & addr, uint16_t port, Inet::InterfaceId interface)
    {
        return TCP(addr).SetPort(port).SetInterface(interface);
    }

    static PeerAddress Multicast(chip::FabricId fabric, chip::GroupId group)
    {
        constexpr uint8_t scope        = 0x05; // Site-Local
        constexpr uint8_t prefixLength = 0x40; // 64-bit long network prefix field
        // The network prefix portion of the Multicast Address is the 64-bit bitstring formed by concatenating:
        // * 0xFD to designate a locally assigned ULA prefix
        // * The upper 56-bits of the Fabric ID for the network in big-endian order
        const uint64_t prefix = 0xfd00000000000000 | ((fabric >> 8) & 0x00ffffffffffffff);
        // The 32-bit group identifier portion of the Multicast Address is the 32-bits formed by:
        // * The lower 8-bits of the Fabric ID
        // * 0x00
        // * The 16-bits Group Identifier in big-endian order
        uint32_t groupId = static_cast<uint32_t>((fabric << 24) & 0xff000000) | group;
        return UDP(Inet::IPAddress::MakeIPv6PrefixMulticast(scope, prefixLength, prefix, groupId));
    }

private:
    Inet::IPAddress mIPAddresses[kMaxPeerIPAddresses] = {};
    unsigned mNumValidIPAddresses                     = 0;
    Type mTransportType                               = Type::kUndefined;
    uint16_t mPort                                    = CHIP_PORT; ///< Relevant for UDP data sending.
    Inet::InterfaceId mInterface                      = Inet::InterfaceId::Null();
};

} // namespace Transport
} // namespace chip
