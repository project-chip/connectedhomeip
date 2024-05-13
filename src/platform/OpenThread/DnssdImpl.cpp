/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <platform/OpenThread/OpenThreadDnssdImpl.h>

namespace chip {
namespace Dnssd {

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    return OpenThreadDnssdInit(initCallback, errorCallback, context);
}

void ChipDnssdShutdown() {}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    return OpenThreadDnssdPublishService(service, callback, context);
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    return OpenThreadDnssdRemoveServices();
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return OpenThreadDnssdFinalizeServiceUpdate();
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                           Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context, intptr_t * browseIdentifier)
{
    return OpenThreadDnssdBrowse(type, protocol, addressType, interface, callback, context, browseIdentifier);
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * browseResult, Inet::InterfaceId interface, DnssdResolveCallback callback, void * context)
{
    return OpenThreadDnssdResolve(browseResult, interface, callback, context);
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName) {}
CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
