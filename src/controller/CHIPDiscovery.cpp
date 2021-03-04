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
#include <stdarg.h>

#include <cstdint>
#include <string>

#include <controller/CHIPDiscovery.h>
#include <lib/mdns/minimal/core/Constants.h>

namespace chip {
namespace Controller {

namespace {
std::string ToString(const mdns::Minimal::BytesRange & bytes)
{
    std::string str;
    str.reserve(bytes.Size() + 1);
    str.assign(reinterpret_cast<const char *>(bytes.Start()), bytes.Size());
    return str;
}

} // namespace

void TxtRecordDelegateImpl::OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value)
{
    if (dnsSdInfo == nullptr)
    {
        return;
    }
    // Names are just 1 letter.
    if (name.Size() < 1)
    {
        return;
    }
    std::string key = ToString(name);
    std::string val = ToString(value);
    if (key == "D")
    {
        // For some reason there is no standard string -> short converstion.
        unsigned long temp = std::stoul(val);
        if (temp < UINT16_MAX)
        {
            dnsSdInfo->longDiscriminator = static_cast<uint16_t>(temp);
        }
    }
    else if (key == "VP")
    {
        // Fist value is the vendor id, second (after the +) is the product.
        size_t plusSignIdx = val.find('+');
        unsigned long temp = std::stoul(val.substr(0, plusSignIdx));
        if (temp < UINT16_MAX)
        {
            dnsSdInfo->vendorId = static_cast<uint16_t>(temp);
        }
        if (plusSignIdx == std::string::npos || (plusSignIdx + 1) >= val.length())
        {
            return;
        }
        temp = std::stoul(val.substr(plusSignIdx + 1, std::string::npos));
        if (temp < UINT16_MAX)
        {
            dnsSdInfo->productId = static_cast<uint16_t>(temp);
        }
    }
}

void ParserDelegateImpl::OnHeader(mdns::Minimal::ConstHeaderRef & header)
{
    // Controller doesn't care about headers.
}
void ParserDelegateImpl::OnQuery(const mdns::Minimal::QueryData & data)
{
    // Controller doesn't care about queries.
}

int ParserDelegateImpl::FindDnsSdByHostName(const char * hostName) const
{
    for (int i = 0; i < kNumDnsSdEntries; ++i)
    {
        if (dnsSdInfo[i].IsInstance(hostName))
        {
            return i;
        }
    }
    return -1;
}

int ParserDelegateImpl::FindDnsSdByInstanceName(const char * instanceName) const
{
    for (int i = 0; i < kNumDnsSdEntries; ++i)
    {
        if (dnsSdInfo[i].IsInstance(instanceName))
        {
            return i;
        }
    }
    return -1;
}

DiscoveryType GetDiscoveryTypeFromQname(mdns::Minimal::SerializedQNameIterator it)
{
    while (it.Next())
    {
        if (strncmp(it.Value(), "_chipc", strlen("_chipc")) == 0)
        {
            return DiscoveryType::COMMISSIONING;
        }
        else if (strncmp(it.Value(), "_chip", strlen("_chip")) == 0)
        {
            return DiscoveryType::OPERATIONAL;
        }
    }
    return DiscoveryType::UNKNOWN;
}

void ParserDelegateImpl::OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data)
{
    if (data.GetType() == mdns::Minimal::QType::PTR) {
        ++currIdx;
    }
    if (currIdx < 0 || currIdx >= kNumDnsSdEntries)
    {
        return;
    }
    switch (data.GetType())
    {
    case mdns::Minimal::QType::PTR: {
        mdns::Minimal::SerializedQNameIterator it;
        if (!ParsePtrRecord(data.GetData(), data.GetData(), &it))
        {
            printf("Error parsing PTR record\n");
            return;
        }
        DiscoveryType discoveryType = GetDiscoveryTypeFromQname(it);
        if (discoveryType == DiscoveryType::UNKNOWN)
        {
            return;
        }
        // The first part of the QName is the instance name.
        it.Next();
        if (FindDnsSdByInstanceName(it.Value()) != -1)
        {
            // Getting back a duplicate record - ignore for now.
            // TODO(cecille): We may actually want to refresh this, but not
            // more than once per request.
            return;
        }
        strncpy(dnsSdInfo[currIdx].instanceName, it.Value(), DnsSdInfo::kInstanceNameSize);
        dnsSdInfo[currIdx].discoveryType = discoveryType;
        break;
    }
    case mdns::Minimal::QType::SRV: {
        mdns::Minimal::SrvRecord srvRecord;
        if (currIdx < 0 || !srvRecord.Parse(data.GetData(), data.GetData()))
        {
            printf("Error parsing SRV record\n");
            return;
        }
        // Host name is the first part of the qname
        mdns::Minimal::SerializedQNameIterator it = srvRecord.GetName();
        it.Next();
        strncpy(dnsSdInfo[currIdx].hostName, it.Value(), DnsSdInfo::kHostNameSize);
        break;
    }
    case mdns::Minimal::QType::TXT: {
        TxtRecordDelegateImpl textRecordDelegate(&dnsSdInfo[currIdx]);
        if (currIdx < 0 || !ParseTxtRecord(data.GetData(), &textRecordDelegate))
        {
            printf("Error parsing TXT record\n");
            return;
        }
        break;
    }
    case mdns::Minimal::QType::A: {
        int ipIdx = dnsSdInfo[currIdx].numIps;
        ParseARecord(data.GetData(), &dnsSdInfo[currIdx].ipAddress[ipIdx]);
        dnsSdInfo[currIdx].numIps++;
        break;
    }
    case mdns::Minimal::QType::AAAA: {
        int ipIdx = dnsSdInfo[currIdx].numIps;
        ParseAAAARecord(data.GetData(), &dnsSdInfo[currIdx].ipAddress[ipIdx]);
        dnsSdInfo[currIdx].numIps++;
        break;
    }
    default:
        break;
    }
}

const DnsSdInfo * ParserDelegateImpl::GetDiscoveredDevice(int idx) const
{
    if (idx < 0 || idx > kNumDnsSdEntries || !dnsSdInfo[idx].IsValid())
    {
        return nullptr;
    }
    return &dnsSdInfo[idx];
}

void ParserDelegateImpl::ClearDiscoveredDevices()
{
    for (int i = 0; i < kNumDnsSdEntries; ++i)
    {
        dnsSdInfo[i].Reset();
    }
    currIdx = -1;
}

void ServerDelegateImpl::OnQuery(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    // Controllers don't answer queries.
}
void ServerDelegateImpl::OnResponse(const mdns::Minimal::BytesRange & data, const chip::Inet::IPPacketInfo * info)
{
    ParsePacket(data, &parser);
}
const DnsSdInfo * ServerDelegateImpl::GetDiscoveredDevice(int idx) const
{
    return parser.GetDiscoveredDevice(idx);
}
void ServerDelegateImpl::ClearDiscoveredDevices()
{
    parser.ClearDiscoveredDevices();
}

void DeviceDiscovery::FindAvailableDevices(DiscoveryType type)
{
    FindAvailableDevices(type, DiscoverySubtypeInfo());
}
void DeviceDiscovery::FindAvailableDevicesLongDiscriminator(DiscoveryType type, uint16_t longDiscriminator)
{
    DiscoverySubtypeInfo subtype(DiscoverySubtype::LONG, longDiscriminator);
    FindAvailableDevices(type, subtype);
}

CHIP_ERROR DeviceDiscovery::SendQuery(mdns::Minimal::FullQName qname, mdns::Minimal::QType type)
{
    // TODO(cecille): find real size.
    static constexpr size_t kPacketBufferSize = 512;
    chip::System::PacketBufferHandle buffer   = chip::System::PacketBufferHandle::New(kPacketBufferSize);

    mdns::Minimal::Query query(qname);
    query.SetType(type).SetClass(mdns::Minimal::QClass::IN);
    // TODO(cecille): Not sure why unicast response isn't working - fix.
    query.SetAnswerViaUnicast(false);

    mdns::Minimal::QueryBuilder builder(std::move(buffer));
    builder.AddQuery(query);

    CHIP_ERROR err = mdnsServer.BroadcastSend(builder.ReleasePacket(), kMdnsPort);
    if (err != CHIP_NO_ERROR)
    {
        printf("Error broadcasting query");
    }
    return err;
}

void DeviceDiscovery::FindAvailableDevices(DiscoveryType type, DiscoverySubtypeInfo subtype)
{
    char subtypeStr[] = "_Xdddddd";

    mdns::Minimal::FullQName qname;

    switch (subtype.subtype)
    {
    case DiscoverySubtype::SHORT:
        snprintf(subtypeStr, strlen(subtypeStr), "_S%03u", subtype.code);
        break;
    case DiscoverySubtype::LONG:
        snprintf(subtypeStr, strlen(subtypeStr), "_L%04u", subtype.code);
        break;
    case DiscoverySubtype::VENDOR:
        snprintf(subtypeStr, strlen(subtypeStr), "_V%03u", subtype.code);
        break;
    case DiscoverySubtype::NONE:
        break;
    }

    switch (type)
    {
    case DiscoveryType::OPERATIONAL:
        qname = CheckAndAllocateQName("_chip", "_tcp", "local");
        break;
    case DiscoveryType::COMMISSIONING:
        if (subtype.subtype == DiscoverySubtype::NONE)
        {
            qname = CheckAndAllocateQName("_chipc", "_udp", "local");
        }
        else
        {
            qname = CheckAndAllocateQName(subtypeStr, "_sub", "_chipc", "_udp", "local");
        }
        break;
    case DiscoveryType::UNKNOWN:
        break;
    }
    if (!qname.nameCount)
    {
        return;
    }

    serverDelegate.ClearDiscoveredDevices();
    mdnsServer.SetDelegate(&serverDelegate);
    if (mdnsServer.Listen(inetLayer, &listenIterator, kMdnsPort) != CHIP_NO_ERROR)
    {
        printf("Error starting mdns listener");
        return;
    }

    SendQuery(qname, mdns::Minimal::QType::ANY);
}

const DnsSdInfo * DeviceDiscovery::GetDiscoveredDevice(int idx) const
{
    return serverDelegate.GetDiscoveredDevice(idx);
}

} // namespace Controller
} // namespace chip
