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

#include "lib/mdns/platform/Mdns.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CHIPMem.h"
#include "support/CodeUtils.h"
#include "support/logging/CHIPLogging.h"

#include <platform/platform_stdlib.h>
#include <mDNS/mDNS.h>
#include <lwip_netconf.h>
#include <lwip/netif.h>

extern struct netif xnetif[];

namespace {

static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;

} // namespace

namespace chip {
namespace Mdns {

CHIP_ERROR ChipMdnsInit(MdnsAsyncReturnCallback initCallback, MdnsAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int ameba_error = 0;

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

const char * GetProtocolString(MdnsServiceProtocol protocol)
{
    return protocol == MdnsServiceProtocol::kMdnsProtocolTcp ? "_tcp" : "_udp";
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
    CHIP_ERROR error        = CHIP_NO_ERROR;

    DNSServiceRef dnsServiceRef = NULL;
    TXTRecordRef txtRecord;
    unsigned char txt_buf[100];

    TXTRecordCreate(&txtRecord, sizeof(txt_buf), txt_buf);

    for (size_t i = 0; i < (service -> mTextEntrySize); i++) {
    const char *value = reinterpret_cast <const char *> (service -> mTextEntries[i].mData);
        TXTRecordSetValue(&txtRecord, service->mTextEntries[i].mKey, strlen(value), value);
    }

    char *name = const_cast<char*>(service->mName);
    char *service_type = strcat(strcat(const_cast<char*>(service->mType), "."), GetProtocolString(service->mProtocol));
    char *domain = "local";
    unsigned short port = service->mPort;

    dnsServiceRef = mDNSRegisterService(name, service_type, domain, port,&txtRecord);
    VerifyOrExit(dnsServiceRef != NULL, error = CHIP_ERROR_INTERNAL);

exit:

    return error;
}

CHIP_ERROR ChipMdnsStopPublish()
{
    mDNSResponderDeinit();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipMdnsStopPublishService(const MdnsService * service)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsBrowse(const char * /*type*/, MdnsServiceProtocol /*protocol*/, chip::Inet::IPAddressType addressType,
                          chip::Inet::InterfaceId /*interface*/, MdnsBrowseCallback /*callback*/, void * /*context*/)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipMdnsResolve(MdnsService * /*service*/, chip::Inet::InterfaceId /*interface*/, MdnsResolveCallback /*callback*/,
                           void * /*context*/)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Mdns
} // namespace chip
