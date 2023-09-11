/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DnssdImpl.h"

namespace chip {
namespace Dnssd {
namespace Error {

const char * ToString(DNSServiceErrorType errorCode)
{
    switch (errorCode)
    {
    case kDNSServiceErr_NoError:
        return "kDNSServiceErr_NoError";
    case kDNSServiceErr_Unknown:
        return "kDNSServiceErr_Unknown";
    case kDNSServiceErr_NoSuchName:
        return "kDNSServiceErr_NoSuchName";
    case kDNSServiceErr_NoMemory:
        return "kDNSServiceErr_NoMemory";
    case kDNSServiceErr_BadParam:
        return "kDNSServiceErr_BadParam";
    case kDNSServiceErr_BadReference:
        return "kDNSServiceErr_BadReference";
    case kDNSServiceErr_BadState:
        return "kDNSServiceErr_BadState";
    case kDNSServiceErr_BadFlags:
        return "kDNSServiceErr_BadFlags";
    case kDNSServiceErr_Unsupported:
        return "kDNSServiceErr_Unsupported";
    case kDNSServiceErr_NotInitialized:
        return "kDNSServiceErr_NotInitialized";
    case kDNSServiceErr_AlreadyRegistered:
        return "kDNSServiceErr_AlreadyRegistered";
    case kDNSServiceErr_NameConflict:
        return "kDNSServiceErr_NameConflict";
    case kDNSServiceErr_Invalid:
        return "kDNSServiceErr_Invalid";
    case kDNSServiceErr_Firewall:
        return "kDNSServiceErr_Firewall";
    case kDNSServiceErr_Incompatible:
        return "kDNSServiceErr_Incompatible";
    case kDNSServiceErr_BadInterfaceIndex:
        return "kDNSServiceErr_BadInterfaceIndex";
    case kDNSServiceErr_Refused:
        return "kDNSServiceErr_Refused";
    case kDNSServiceErr_NoSuchRecord:
        return "kDNSServiceErr_NoSuchRecord";
    case kDNSServiceErr_NoAuth:
        return "kDNSServiceErr_NoAuth";
    case kDNSServiceErr_NoSuchKey:
        return "kDNSServiceErr_NoSuchKey";
    case kDNSServiceErr_NATTraversal:
        return "kDNSServiceErr_NATTraversal";
    case kDNSServiceErr_DoubleNAT:
        return "kDNSServiceErr_DoubleNAT";
    case kDNSServiceErr_BadTime:
        return "kDNSServiceErr_BadTime";
    case kDNSServiceErr_BadSig:
        return "kDNSServiceErr_BadSig";
    case kDNSServiceErr_BadKey:
        return "kDNSServiceErr_BadKey";
    case kDNSServiceErr_Transient:
        return "kDNSServiceErr_Transient";
    case kDNSServiceErr_ServiceNotRunning:
        return "kDNSServiceErr_ServiceNotRunning";
    case kDNSServiceErr_NATPortMappingUnsupported:
        return "kDNSServiceErr_NATPortMappingUnsupported";
    case kDNSServiceErr_NATPortMappingDisabled:
        return "kDNSServiceErr_NATPortMappingDisabled";
    case kDNSServiceErr_NoRouter:
        return "kDNSServiceErr_NoRouter";
    case kDNSServiceErr_PollingMode:
        return "kDNSServiceErr_PollingMode";
    case kDNSServiceErr_Timeout:
        return "kDNSServiceErr_Timeout";
    default:
        return "Unknown DNSService error code";
    }
}

CHIP_ERROR ToChipError(DNSServiceErrorType errorCode)
{
    switch (errorCode)
    {
    case kDNSServiceErr_NoError:
        return CHIP_NO_ERROR;
    case kDNSServiceErr_NameConflict:
        return CHIP_ERROR_MDNS_COLLISION;
    case kDNSServiceErr_NoMemory:
        return CHIP_ERROR_NO_MEMORY;
    default:
        return CHIP_ERROR_INTERNAL;
    }
}

} // namespace Error
} // namespace Dnssd
} // namespace chip
