/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
 *  @file
 *    Contains definitions and delegates for controller-side discovery.
 */

#pragma once

#include <lib/mdns/AllInterfacesListenIterator.h>
#include <lib/mdns/minimal/Parser.h>
#include <lib/mdns/minimal/QueryBuilder.h>
#include <lib/mdns/minimal/RecordData.h>
#include <lib/mdns/minimal/Server.h>
#include <lib/mdns/minimal/core/DnsHeader.h>
#include <lib/mdns/minimal/core/FlatAllocatedQName.h>
#include <lib/mdns/minimal/core/QName.h>

namespace chip {
namespace Controller {

enum class DiscoveryType
{
    UNKNOWN,
    OPERATIONAL,
    COMMISSIONING,
    // TODO(cecille): Add commissionable if that ends up staying in the spec.
};

enum class DiscoverySubtype
{
    NONE,
    SHORT,
    LONG,
    VENDOR,
};

struct DiscoverySubtypeInfo
{
    DiscoverySubtype subtype;
    uint16_t code;
    DiscoverySubtypeInfo() : subtype(DiscoverySubtype::NONE), code(0) {}
    DiscoverySubtypeInfo(DiscoverySubtype newSubtype, uint16_t newCode) : subtype(newSubtype), code(newCode) {}
};

struct DnsSdInfo
{
    // TODO(cecille): is 4 OK? IPv6 LL, GUA, ULA, IPv4?
    static constexpr int kNumIpAddresses = 5;
    // Host name is 64-bits in hex.
    static constexpr int kInstanceNameSize = 16;
    // Largest host name is 64-bits in hex.
    static constexpr int kHostNameSize = 16;
    // +1 for nulls.
    char instanceName[kInstanceNameSize + 1];
    char hostName[kHostNameSize + 1];
    DiscoveryType discoveryType;
    uint16_t longDiscriminator;
    uint16_t vendorId;
    uint16_t productId;
    int numIps;
    chip::Inet::IPAddress ipAddress[kNumIpAddresses];
    // TODO(cecille): Missing RI - fix once this settles in the spec.
    DnsSdInfo() { Reset(); }
    void Reset()
    {
        memset(instanceName, 0, kInstanceNameSize + 1);
        memset(hostName, 0, kHostNameSize + 1);
        discoveryType     = DiscoveryType::UNKNOWN;
        longDiscriminator = 0;
        vendorId          = 0;
        productId         = 0;
        numIps            = 0;
        for (int i = 0; i < kNumIpAddresses; ++i)
        {
            ipAddress[i] = chip::Inet::IPAddress::Any;
        }
    }
    bool IsHost(const char * host) const { return strncmp(host, hostName, kHostNameSize) == 0; }
    bool IsInstance(const char * instance) const { return strncmp(instance, instanceName, kInstanceNameSize) == 0; }
    bool IsValid() const { return !IsInstance("") && ipAddress[0] != chip::Inet::IPAddress::Any; }
};

class TxtRecordDelegateImpl : public mdns::Minimal::TxtRecordDelegate
{
public:
    TxtRecordDelegateImpl(DnsSdInfo * info) : dnsSdInfo(info) {}
    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value);

private:
    DnsSdInfo * dnsSdInfo;
};

class ParserDelegateImpl : public mdns::Minimal::ParserDelegate
{
public:
    ParserDelegateImpl() {}
    void OnHeader(mdns::Minimal::ConstHeaderRef & header);
    void OnQuery(const mdns::Minimal::QueryData & data);
    void OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data);
    const DnsSdInfo * GetDiscoveredDevice(int idx) const;
    void ClearDiscoveredDevices();

private:
    // TODO(cecille): make this adjustable?
    static constexpr int kNumDnsSdEntries = 5;
    DnsSdInfo dnsSdInfo[kNumDnsSdEntries];
    int currIdx = -1;

    int FindDnsSdByHostName(const char * hostName) const;
    int FindDnsSdByInstanceName(const char * instanceName) const;
};

class ServerDelegateImpl : public mdns::Minimal::ServerDelegate
{
public:
    ServerDelegateImpl() {}
    void OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info);
    void OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info);
    const DnsSdInfo * GetDiscoveredDevice(int idx) const;
    void ClearDiscoveredDevices();

private:
    ParserDelegateImpl parser;
};

class DeviceDiscovery
{
public:
    DeviceDiscovery() {}
    void SetInetLayer(chip::Inet::InetLayer * new_inetLayer) { inetLayer = new_inetLayer; }
    void FindAvailableDevices(DiscoveryType type);
    void FindAvailableDevicesLongDiscriminator(DiscoveryType type, uint16_t longDiscriminator);
    const DnsSdInfo * GetDiscoveredDevice(int idx) const;

private:
    static constexpr uint16_t kMdnsPort = 5353;
    static constexpr int kMaxQnameSize  = 100;
    // TODO(cecille): Template this?
    mdns::Minimal::Server<5> mdnsServer;
    ServerDelegateImpl serverDelegate;
    Inet::InetLayer * inetLayer;
    Mdns::AllInterfaces listenIterator;
    char qnameStorage[kMaxQnameSize];

    void FindAvailableDevices(DiscoveryType type, DiscoverySubtypeInfo subtype);
    CHIP_ERROR SendQuery(mdns::Minimal::FullQName qname, mdns::Minimal::QType type);

    template <typename... Args>
    mdns::Minimal::FullQName CheckAndAllocateQName(Args &&... parts)
    {
        size_t requiredSize = mdns::Minimal::FlatAllocatedQName::RequiredStorageSize(parts...);
        if (requiredSize > kMaxQnameSize)
        {
            printf("Unable to allocate QName");
            return mdns::Minimal::FullQName();
        }
        return mdns::Minimal::FlatAllocatedQName::Build(qnameStorage, parts...);
    }
};

} // namespace Controller
} // namespace chip
