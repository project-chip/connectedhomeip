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

#include <lib/dnssd/wire/RecordData.h>

namespace MdnsExample {

namespace {

const char * ToString(chip::Dnssd::QType qtype)
{
    static char buff[32];

    switch (qtype)
    {
    case chip::Dnssd::QType::A:
        return "A";
    case chip::Dnssd::QType::NS:
        return "NS";
    case chip::Dnssd::QType::CNAME:
        return "CNAME";
    case chip::Dnssd::QType::SOA:
        return "SOA";
    case chip::Dnssd::QType::WKS:
        return "WKS";
    case chip::Dnssd::QType::PTR:
        return "PTR";
    case chip::Dnssd::QType::MX:
        return "MX";
    case chip::Dnssd::QType::SRV:
        return "SRV";
    case chip::Dnssd::QType::AAAA:
        return "AAAA";
    case chip::Dnssd::QType::ANY:
        return "ANY";
    case chip::Dnssd::QType::TXT:
        return "TXT";
    default:
        sprintf(buff, "UNKNOWN (%d)!!", static_cast<int>(qtype));
        return buff;
    }
}

const char * ToString(chip::Dnssd::QClass qclass)
{
    static char buff[32];

    switch (qclass)
    {
    case chip::Dnssd::QClass::IN_UNICAST:
        return "IN(UNICAST)";
    case chip::Dnssd::QClass::IN:
        return "IN";
    default:
        sprintf(buff, "UNKNOWN (%d)!!", static_cast<int>(qclass));
        return buff;
    }
}

const char * ToString(chip::Dnssd::ResourceType type)
{
    static char buff[32];

    switch (type)
    {
    case chip::Dnssd::ResourceType::kAnswer:
        return "ANSWER";
    case chip::Dnssd::ResourceType::kAdditional:
        return "ADDITIONAL";
    case chip::Dnssd::ResourceType::kAuthority:
        return "AUTHORITY";
    default:
        sprintf(buff, "UNKNOWN (%d)!!", static_cast<int>(type));
        return buff;
    }
}

class TxtReport : public chip::Dnssd::TxtRecordDelegate
{
public:
    TxtReport(const char * prefix) : mPrefix(prefix) {}

    void OnRecord(const chip::Dnssd::BytesRange & name, const chip::Dnssd::BytesRange & value) override
    {
        std::string sname(reinterpret_cast<const char *>(name.Start()), name.Size());
        std::string svalue(reinterpret_cast<const char *>(value.Start()), value.Size());

        printf("%s    TXT:  '%s' = '%s'\n", mPrefix, sname.c_str(), svalue.c_str());
    }

private:
    const char * mPrefix;
};

void PrintQName(chip::Dnssd::SerializedQNameIterator it)
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

void PacketReporter::OnHeader(chip::Dnssd::ConstHeaderRef & header)
{
    printf("%s%s %d (%d, %d, %d, %d):\n", mPrefix, header.GetFlags().IsQuery() ? "QUERY" : "REPLY", header.GetMessageId(),
           header.GetQueryCount(), header.GetAnswerCount(), header.GetAuthorityCount(), header.GetAdditionalCount());
}

void PacketReporter::OnQuery(const chip::Dnssd::QueryData & data)
{
    printf("%s    QUERY %s/%s%s: ", mPrefix, ToString(data.GetType()), ToString(data.GetClass()),
           data.RequestedUnicastAnswer() ? " UNICAST" : "");
    PrintQName(data.GetName());
}

void PacketReporter::OnResource(chip::Dnssd::ResourceType type, const chip::Dnssd::ResourceData & data)
{
    printf("%s    %s %s/%s ttl %ld: ", mPrefix, ToString(type), ToString(data.GetType()), ToString(data.GetClass()),
           static_cast<long>(data.GetTtlSeconds()));
    PrintQName(data.GetName());

    if (data.GetType() == chip::Dnssd::QType::TXT)
    {
        TxtReport txtReport(mPrefix);
        if (!chip::Dnssd::ParseTxtRecord(data.GetData(), &txtReport))
        {
            printf("FAILED TO PARSE TXT RECORD\n");
        }
    }
    else if (data.GetType() == chip::Dnssd::QType::SRV)
    {
        chip::Dnssd::SrvRecord srv;

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
    else if (data.GetType() == chip::Dnssd::QType::A)
    {
        chip::Inet::IPAddress addr;

        if (!chip::Dnssd::ParseARecord(data.GetData(), &addr))
        {
            printf("FAILED TO PARSE A RECORD\n");
        }
        else
        {
            char buff[128];
            printf("%s      IP:  %s\n", mPrefix, addr.ToString(buff, sizeof(buff)));
        }
    }
    else if (data.GetType() == chip::Dnssd::QType::AAAA)
    {
        chip::Inet::IPAddress addr;

        if (!chip::Dnssd::ParseAAAARecord(data.GetData(), &addr))
        {
            printf("FAILED TO PARSE AAAA RECORD\n");
        }
        else
        {
            char buff[128];
            printf("%s      IP:  %s\n", mPrefix, addr.ToString(buff, sizeof(buff)));
        }
    }
    else if (data.GetType() == chip::Dnssd::QType::PTR)
    {
        chip::Dnssd::SerializedQNameIterator name;
        if (!chip::Dnssd::ParsePtrRecord(data.GetData(), mPacketRange, &name))
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
