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

#include "lib/dnssd/platform/Dnssd.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CHIPMem.h"
#include "support/CodeUtils.h"
#include "support/logging/CHIPLogging.h"

#include <lwip/netif.h>
#include <lwip_netconf.h>
#include <mDNS/mDNS.h>
#include <platform/platform_stdlib.h>

extern struct netif xnetif[];

namespace {

static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;

} // namespace

namespace chip {
namespace Dnssd {

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int ameba_error  = 0;

    ameba_error = mDNSResponderInit();
    VerifyOrExit(ameba_error == 0, error = CHIP_ERROR_INTERNAL);

exit:
    if (ameba_error != 0)
    {
        ChipLogError(DeviceLayer, "Ameba mdns internal error");
    }
    initCallback(context, error);

    return error;
}

const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolTcp ? "_tcp" : "_udp";
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    DNSServiceRef dnsServiceRef = NULL;
    TXTRecordRef txtRecord;
    unsigned char txt_buf[100];

    TXTRecordCreate(&txtRecord, sizeof(txt_buf), txt_buf);

    for (size_t i = 0; i < (service->mTextEntrySize); i++)
    {
        const char * value = reinterpret_cast<const char *>(service->mTextEntries[i].mData);
        TXTRecordSetValue(&txtRecord, service->mTextEntries[i].mKey, strlen(value), value);
    }

    char * name         = const_cast<char *>(service->mName);
    char * service_type = strcat(strcat(const_cast<char *>(service->mType), "."), GetProtocolString(service->mProtocol));
    char * domain       = "local";
    unsigned short port = service->mPort;

    dnsServiceRef = mDNSRegisterService(name, service_type, domain, port, &txtRecord);
    VerifyOrExit(dnsServiceRef != NULL, error = CHIP_ERROR_INTERNAL);

exit:

    return error;
}

CHIP_ERROR ChipDnssdStopPublish()
{
    mDNSResponderDeinit();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier);
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * /*service*/, chip::Inet::InterfaceId /*interface*/, DnssdResolveCallback /*callback*/,
                            void * /*context*/)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName) {}

} // namespace Dnssd
} // namespace chip
