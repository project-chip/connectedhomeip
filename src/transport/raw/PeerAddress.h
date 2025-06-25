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

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
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
    kWiFiPAF,
    kNfc,
    kLast = kNfc, // This is not an actual transport type, it just refers to the last transport type
};

/**
 * Describes how a peer on a CHIP network can be addressed.
 */
class PeerAddress
{
public:
    constexpr PeerAddress() : mTransportType(Type::kUndefined), mId{ .mRemoteId = kUndefinedNodeId } {}
    constexpr PeerAddress(const Inet::IPAddress & addr, Type type) :
        mIPAddress(addr), mTransportType(type), mId{ .mRemoteId = kUndefinedNodeId }
    {}
    constexpr PeerAddress(Type type) : mTransportType(type), mId{ .mRemoteId = kUndefinedNodeId } {}
    constexpr PeerAddress(Type type, NodeId remoteId) : mTransportType(type), mId{ .mRemoteId = remoteId } {}

    constexpr PeerAddress(PeerAddress &&)        = default;
    constexpr PeerAddress(const PeerAddress &)   = default;
    PeerAddress & operator=(const PeerAddress &) = default;
    PeerAddress & operator=(PeerAddress &&)      = default;

    const Inet::IPAddress & GetIPAddress() const { return mIPAddress; }
    PeerAddress & SetIPAddress(const Inet::IPAddress & addr)
    {
        mIPAddress = addr;
        return *this;
    }

    NodeId GetRemoteId() const { return mId.mRemoteId; }

    // NB: 0xFFFF is not allowed for NFC ShortId.
    uint16_t GetNFCShortId() const { return mId.mNFCShortId; }

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

    bool IsMulticast() { return Type::kUdp == mTransportType && mIPAddress.IsIPv6Multicast(); }

    bool operator==(const PeerAddress & other) const;
    bool operator!=(const PeerAddress & other) const { return !(*this == other); }

    /// Maximum size of the string outputes by ToString. Format is of the form:
    /// "UDP:<ip>:<port>"
    static constexpr size_t kMaxToStringSize = 3 // type: UDP/TCP/BLE
        + 1                                      // splitter :
        + 2                                      // brackets around address
        + Inet::IPAddress::kMaxStringLength      // address
        + 1                                      // splitter %
        + Inet::InterfaceId::kMaxIfNameLength    // interface
        + 1                                      // splitter :
        + 5                                      // port: 16 bit interger
        + 1;                                     // NullTerminator

    template <size_t N>
    inline void ToString(char (&buf)[N]) const
    {
        ToString(buf, N);
    }

    void ToString(char * buf, size_t bufSize) const
    {
        char ip_addr[Inet::IPAddress::kMaxStringLength];

        char interface[Inet::InterfaceId::kMaxIfNameLength + 1] = {}; // +1 to prepend '%'
        if (mInterface.IsPresent())
        {
            interface[0]   = '%';
            interface[1]   = 0;
            CHIP_ERROR err = mInterface.GetInterfaceName(interface + 1, sizeof(interface) - 1);
            if (err != CHIP_NO_ERROR)
            {
                Platform::CopyString(interface, sizeof(interface), "%(err)");
            }
        }

        switch (mTransportType)
        {
        case Type::kUndefined:
            snprintf(buf, bufSize, "UNDEFINED");
            break;
        case Type::kUdp:
            mIPAddress.ToString(ip_addr);
#if INET_CONFIG_ENABLE_IPV4
            if (mIPAddress.IsIPv4())
                snprintf(buf, bufSize, "UDP:%s%s:%d", ip_addr, interface, mPort);
            else
#endif
                snprintf(buf, bufSize, "UDP:[%s%s]:%d", ip_addr, interface, mPort);
            break;
        case Type::kTcp:
            mIPAddress.ToString(ip_addr);
#if INET_CONFIG_ENABLE_IPV4
            if (mIPAddress.IsIPv4())
                snprintf(buf, bufSize, "TCP:%s%s:%d", ip_addr, interface, mPort);
            else
#endif
                snprintf(buf, bufSize, "TCP:[%s%s]:%d", ip_addr, interface, mPort);
            break;
        case Type::kWiFiPAF:
            snprintf(buf, bufSize, "Wi-Fi PAF");
            break;
        case Type::kBle:
            // Note that BLE does not currently use any specific address.
            snprintf(buf, bufSize, "BLE");
            break;
        case Type::kNfc:
            snprintf(buf, bufSize, "NFC:%d", mId.mNFCShortId);
            break;
        default:
            snprintf(buf, bufSize, "ERROR");
            break;
        }
    }

    /****** Factory methods for convenience ******/

    static constexpr PeerAddress Uninitialized() { return PeerAddress(Type::kUndefined); }

    static constexpr PeerAddress BLE() { return PeerAddress(Type::kBle); }

    // NB: 0xFFFF is not allowed for NFC ShortId.
    static constexpr PeerAddress NFC() { return PeerAddress(kUndefinedNFCShortId()); }
    static constexpr PeerAddress NFC(const uint16_t shortId) { return PeerAddress(shortId); }

    static PeerAddress UDP(const Inet::IPAddress & addr) { return PeerAddress(addr, Type::kUdp); }
    static PeerAddress UDP(const Inet::IPAddress & addr, uint16_t port) { return UDP(addr).SetPort(port); }

    /**
     * Parses a PeerAddress from the given IP address string with UDP type. For example,
     * "192.168.1.4", "fe80::2", "fe80::1%wlan0". Notably this will also include the network scope
     * ID in either index or name form (e.g. %wlan0, %14).
     */
    static PeerAddress UDP(char * addrStr, uint16_t port) { return PeerAddress::FromString(addrStr, port, Type::kUdp); }
    static PeerAddress UDP(const Inet::IPAddress & addr, uint16_t port, Inet::InterfaceId interface)
    {
        return UDP(addr).SetPort(port).SetInterface(interface);
    }
    static PeerAddress TCP(const Inet::IPAddress & addr) { return PeerAddress(addr, Type::kTcp); }
    static PeerAddress TCP(const Inet::IPAddress & addr, uint16_t port) { return TCP(addr).SetPort(port); }

    /**
     * Parses a PeerAddress from the given IP address string with TCP type. For example,
     * "192.168.1.4", "fe80::2", "fe80::1%wlan0". Notably this will also include the network scope
     * ID in either index or name form (e.g. %wlan0, %14).
     */
    static PeerAddress TCP(char * addrStr, uint16_t port) { return PeerAddress::FromString(addrStr, port, Type::kTcp); }
    static PeerAddress TCP(const Inet::IPAddress & addr, uint16_t port, Inet::InterfaceId interface)
    {
        return TCP(addr).SetPort(port).SetInterface(interface);
    }

    static constexpr PeerAddress WiFiPAF(NodeId remoteId) { return PeerAddress(Type::kWiFiPAF, remoteId); }

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
    constexpr PeerAddress(uint16_t shortId) : mTransportType(Type::kNfc), mId{ .mNFCShortId = shortId } {}

    static PeerAddress FromString(char * addrStr, uint16_t port, Type type)
    {
        Inet::IPAddress addr;
        Inet::InterfaceId interfaceId;
        Inet::IPAddress::FromString(addrStr, addr, interfaceId);
        return PeerAddress(addr, type).SetPort(port).SetInterface(interfaceId);
    }

    static constexpr uint16_t kUndefinedNFCShortId() { return 0xFFFF; }

    Inet::IPAddress mIPAddress   = {};
    Type mTransportType          = Type::kUndefined;
    uint16_t mPort               = CHIP_PORT; ///< Relevant for UDP data sending.
    Inet::InterfaceId mInterface = Inet::InterfaceId::Null();

    union Id
    {
        NodeId mRemoteId;
        uint16_t mNFCShortId;
    } mId;
};

} // namespace Transport
} // namespace chip
