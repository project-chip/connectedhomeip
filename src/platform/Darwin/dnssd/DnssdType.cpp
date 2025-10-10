/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include "DnssdType.h"

#include <sstream>
#include <string>

constexpr char kProtocolTcp[] = "._tcp";
constexpr char kProtocolUdp[] = "._udp";

namespace chip {
namespace Dnssd {
namespace Internal {

std::string GetFullType(const char * type, DnssdServiceProtocol protocol)
{
    std::ostringstream typeBuilder;
    typeBuilder << type;
    typeBuilder << (protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? kProtocolUdp : kProtocolTcp);
    return typeBuilder.str();
}

std::string GetFullType(const DnssdService * service)
{
    return GetFullType(service->mType, service->mProtocol);
}

std::string GetFullTypeWithSubTypes(const char * type, DnssdServiceProtocol protocol)
{
    auto fullType = GetFullType(type, protocol);

    std::string subtypeDelimiter = "._sub.";
    size_t position              = fullType.find(subtypeDelimiter);
    if (position != std::string::npos)
    {
        fullType = fullType.substr(position + subtypeDelimiter.size()) + "," + fullType.substr(0, position);
    }

    return fullType;
}

std::string GetFullTypeWithSubTypes(const char * type, DnssdServiceProtocol protocol, const char * subTypes[], size_t subTypeSize)
{
    std::ostringstream typeBuilder;
    typeBuilder << type;
    typeBuilder << (protocol == DnssdServiceProtocol::kDnssdProtocolUdp ? kProtocolUdp : kProtocolTcp);
    for (int i = 0; i < (int) subTypeSize; i++)
    {
        typeBuilder << ",";
        typeBuilder << subTypes[i];
    }
    return typeBuilder.str();
}

std::string GetFullTypeWithSubTypes(const DnssdService * service)
{
    return GetFullTypeWithSubTypes(service->mType, service->mProtocol, service->mSubTypes, service->mSubTypeSize);
}

std::string GetFullTypeWithoutSubTypes(std::string fullType)
{
    size_t position = fullType.find(",");
    if (position != std::string::npos)
    {
        fullType.erase(position);
    }

    return fullType;
}

std::string GetBaseType(const char * fulltype)
{
    std::string type(fulltype);
    size_t position = type.find(".");
    if (position != std::string::npos)
    {
        type.erase(position);
    }

    return type;
}

} // namespace Internal
} // namespace Dnssd
} // namespace chip
