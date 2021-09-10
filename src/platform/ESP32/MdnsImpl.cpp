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

#include <esp_err.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>
#include <mdns.h>

#include "platform/CHIPDeviceLayer.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace {

static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;

} // namespace

namespace chip {
namespace Mdns {

CHIP_ERROR ChipMdnsInit(MdnsAsyncReturnCallback initCallback, MdnsAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    esp_err_t espError;

    espError = mdns_init();
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);

exit:
    if (espError != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp mdns internal error: %s", esp_err_to_name(espError));
    }
    initCallback(context, error);

    return error;
}

CHIP_ERROR ChipMdnsShutdown()
{
    return CHIP_NO_ERROR;
}

static const char * GetProtocolString(MdnsServiceProtocol protocol)
{
    return protocol == MdnsServiceProtocol::kMdnsProtocolTcp ? "_tcp" : "_udp";
}

CHIP_ERROR ChipMdnsPublishService(const MdnsService * service)
{
    CHIP_ERROR error        = CHIP_NO_ERROR;
    mdns_txt_item_t * items = nullptr;
    esp_err_t espError;

    if (strcmp(service->mHostName, "") != 0)
    {
        VerifyOrExit(mdns_hostname_set(service->mHostName) == ESP_OK, error = CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit(service->mTextEntrySize <= UINT8_MAX, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (service->mTextEntries)
    {
        items = static_cast<mdns_txt_item_t *>(chip::Platform::MemoryCalloc(service->mTextEntrySize, sizeof(mdns_txt_item_t)));
        VerifyOrExit(items != nullptr, error = CHIP_ERROR_NO_MEMORY);
        for (size_t i = 0; i < service->mTextEntrySize; i++)
        {
            items[i].key = service->mTextEntries[i].mKey;
            // Unfortunately ESP mdns stack doesn't support arbitrary binary data
            items[i].value = reinterpret_cast<const char *>(service->mTextEntries[i].mData);
        }
    }

    espError = mdns_service_add(service->mName, service->mType, GetProtocolString(service->mProtocol), service->mPort, items,
                                service->mTextEntrySize);
    // The mdns_service_add will return error if we try to add an existing service
    if (espError != ESP_OK && espError != ESP_ERR_NO_MEM)
    {
        espError = mdns_service_txt_set(service->mType, GetProtocolString(service->mProtocol), items,
                                        static_cast<uint8_t>(service->mTextEntrySize));
    }
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);

exit:
    if (items != nullptr)
    {
        chip::Platform::MemoryFree(items);
    }

    return error;
}

CHIP_ERROR ChipMdnsStopPublish()
{
    return mdns_service_remove_all() == ESP_OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
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
