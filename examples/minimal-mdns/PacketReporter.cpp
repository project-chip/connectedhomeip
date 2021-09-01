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

#include "PacketReporter.h"

#include <memory>
#include <stdio.h>
#include <string>

#include <lib/mdns/minimal/RecordData.h>

namespace MdnsExample {

namespace {

const char * ToString(mdns::Minimal::QType qtype)
{
    static char buff[32];

    switch (qtype)
    {
    case mdns::Minimal::QType::A:
        return "A";
    case mdns::Minimal::QType::NS:
        return "NS";
    case mdns::Minimal::QType::CNAME:
        return "CNAME";
    case mdns::Minimal::QType::SOA:
        return "SOA";
    case mdns::Minimal::QType::WKS:
        return "WKS";
    case mdns::Minimal::QType::PTR:
        return "PTR";
    case mdns::Minimal::QType::MX:
        return "MX";
    case mdns::Minimal::QType::SRV:
        return "SRV";
    case mdns::Minimal::QType::AAAA:
        return "AAAA";
    case mdns::Minimal::QType::ANY:
        return "ANY";
    case mdns::Minimal::QType::TXT:
        return "TXT";
    default:
        sprintf(buff, "UNKNOWN (%d)!!", static_cast<int>(qtype));
        return buff;
    }
}

const char * ToString(mdns::Minimal::QClass qclass)
{
    static char buff[32];

    switch (qclass)
    {
    case mdns::Minimal::QClass::IN_UNICAST:
        return "IN(UNICAST)";
    case mdns::Minimal::QClass::IN:
        return "IN";
    default:
        sprintf(buff, "UNKNOWN (%d)!!", static_cast<int>(qclass));
        return buff;
    }
}

const char * ToString(mdns::Minimal::ResourceType type)
{
    static char buff[32];

    switch (type)
    {
    case mdns::Minimal::ResourceType::kAnswer:
        return "ANSWER";
    case mdns::Minimal::ResourceType::kAdditional:
        return "ADDITIONAL";
    case mdns::Minimal::ResourceType::kAuthority:
        return "AUTHORITY";
    default:
        sprintf(buff, "UNKNOWN (%d)!!", static_cast<int>(type));
        return buff;
    }
}

class TxtReport : public mdns::Minimal::TxtRecordDelegate
{
public:
    TxtReport(const char * prefix) : mPrefix(prefix) {}

    void OnRecord(const mdns::Minimal::BytesRange & name, const mdns::Minimal::BytesRange & value) override
    {
        std::string sname(reinterpret_cast<const char *>(name.Start()), name.Size());
        std::string svalue(reinterpret_cast<const char *>(value.Start()), value.Size());

        printf("%s    TXT:  '%s' = '%s'\n", mPrefix, sname.c_str(), svalue.c_str());
    }

private:
    const char * mPrefix;
};

void PrintQName(mdns::Minimal::SerializedQNameIterator it)
{
    while (it.Next())
    {
        printf("%s.", it.Value());
    }
    if (!it.IsValid())
    {
        printf("   (INVALID!)");
    }
    printf("\n");
}

} // namespace

void PacketReporter::OnHeader(mdns::Minimal::ConstHeaderRef & header)
{
    printf("%s%s %d (%d, %d, %d, %d):\n", mPrefix, header.GetFlags().IsQuery() ? "QUERY" : "REPLY", header.GetMessageId(),
           header.GetQueryCount(), header.GetAnswerCount(), header.GetAuthorityCount(), header.GetAdditionalCount());
}

void PacketReporter::OnQuery(const mdns::Minimal::QueryData & data)
{
    printf("%s    QUERY %s/%s%s: ", mPrefix, ToString(data.GetType()), ToString(data.GetClass()),
           data.RequestedUnicastAnswer() ? " UNICAST" : "");
    PrintQName(data.GetName());
}

void PacketReporter::OnResource(mdns::Minimal::ResourceType type, const mdns::Minimal::ResourceData & data)
{
    printf("%s    %s %s/%s ttl %ld: ", mPrefix, ToString(type), ToString(data.GetType()), ToString(data.GetClass()),
           static_cast<long>(data.GetTtlSeconds()));
    PrintQName(data.GetName());

    if (data.GetType() == mdns::Minimal::QType::TXT)
    {
        TxtReport txtReport(mPrefix);
        if (!mdns::Minimal::ParseTxtRecord(data.GetData(), &txtReport))
        {
            printf("FAILED TO PARSE TXT RECORD\n");
        }
    }
    else if (data.GetType() == mdns::Minimal::QType::SRV)
    {
        mdns::Minimal::SrvRecord srv;

        if (!srv.Parse(data.GetData(), mPacketRange))
        {
            printf("Failed to parse SRV record!!!!");
        }
        else
        {
            printf("%s      SRV on port %d, priority %d, weight %d:  ", mPrefix, srv.GetPort(), srv.GetPriority(), srv.GetWeight());
            PrintQName(srv.GetName());
        }
    }
    else if (data.GetType() == mdns::Minimal::QType::A)
    {
        chip::Inet::IPAddress addr;

        if (!mdns::Minimal::ParseARecord(data.GetData(), &addr))
        {
            printf("FAILED TO PARSE A RECORD\n");
        }
        else
        {
            char buff[128];
            printf("%s      IP:  %s\n", mPrefix, addr.ToString(buff, sizeof(buff)));
        }
    }
    else if (data.GetType() == mdns::Minimal::QType::AAAA)
    {
        chip::Inet::IPAddress addr;

        if (!mdns::Minimal::ParseAAAARecord(data.GetData(), &addr))
        {
            printf("FAILED TO PARSE AAAA RECORD\n");
        }
        else
        {
            char buff[128];
            printf("%s      IP:  %s\n", mPrefix, addr.ToString(buff, sizeof(buff)));
        }
    }
    else if (data.GetType() == mdns::Minimal::QType::PTR)
    {
        mdns::Minimal::SerializedQNameIterator name;
        if (!mdns::Minimal::ParsePtrRecord(data.GetData(), mPacketRange, &name))
        {
            printf("FAILED TO PARSE AAAA RECORD\n");
        }
        else
        {
            printf("%s      PTR:  ", mPrefix);
            PrintQName(name);
        }
    }
    else
    {
        // Unknown packet, just print data size.
        printf("%s  Data size: %ld\n", mPrefix, static_cast<long>(data.GetData().Size()));
    }
}

} // namespace MdnsExample
