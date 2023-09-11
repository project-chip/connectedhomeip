/*
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DnssdType.h"

#include <sstream>

constexpr const char * kProtocolTcp = "._tcp";
constexpr const char * kProtocolUdp = "._udp";

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
