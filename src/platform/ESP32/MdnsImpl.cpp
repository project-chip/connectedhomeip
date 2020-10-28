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

#include "lib/protocols/mdns/platform/Mdns.h"

#include <esp_err.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>
#include <mdns.h>

#include "platform/CHIPDeviceLayer.h"
#include "support/CHIPMem.h"
#include "support/CodeUtils.h"
#include "support/logging/CHIPLogging.h"

namespace {

static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;

} // namespace

namespace chip {
namespace Protocols {
namespace Mdns {

CHIP_ERROR ChipMdnsInit(MdnsAsnycReturnCallback initCallback, MdnsAsnycReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    esp_err_t espError;
    uint16_t discriminator;
    char hostname[11]; // Hostname will be "CHIP-XXXX"

    espError = mdns_init();
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(discriminator));
    snprintf(hostname, sizeof(hostname), "CHIP-%04d", discriminator);
    espError = mdns_hostname_set(hostname);
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);

exit:
    if (espError != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp mdns internal error: %s", esp_err_to_name(espError));
    }
    initCallback(context, error);

    return error;
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

    VerifyOrExit(service->mTextEntrySize <= UINT8_MAX, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (service->mTextEntryies)
    {
        items = static_cast<mdns_txt_item_t *>(chip::Platform::MemoryCalloc(service->mTextEntrySize, sizeof(mdns_txt_item_t)));
        VerifyOrExit(items != nullptr, error = CHIP_ERROR_NO_MEMORY);
        for (size_t i = 0; i < service->mTextEntrySize; i++)
        {
            items[i].key = service->mTextEntryies[i].mKey;
            // Unfortunately ESP mdns stack dosen't support arbitrary binary data
            items[i].value = reinterpret_cast<const char *>(service->mTextEntryies[i].mData);
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

CHIP_ERROR ChipMdnsBrowse(const char * type, MdnsServiceProtocol protocol, chip::Inet::InterfaceId interface,
                          MdnsBrowseCallback callback, void * context)
{
    mdns_result_t * results = nullptr;
    mdns_result_t * current;
    CHIP_ERROR error = CHIP_NO_ERROR;
    esp_err_t espError;
    MdnsService * services = nullptr;
    TextEntry * textEntries;
    size_t serviceSize = 0;

    espError = mdns_query_ptr(type, GetProtocolString(protocol), kTimeoutMilli, kMaxResults, &results);
    VerifyOrExit(espError == ESP_OK, error = CHIP_ERROR_INTERNAL);
    if (results == nullptr)
    {
        ChipLogProgress(DeviceLayer, "ChipMdnsBrowse: No result");
        callback(context, nullptr, 0, CHIP_NO_ERROR);
    }
    else
    {
        current = results;
        while (current != nullptr)
        {
            serviceSize++;
            current = current->next;
        }
        ChipLogProgress(DeviceLayer, "Service size=%zu", serviceSize);
        services = static_cast<MdnsService *>(chip::Platform::MemoryCalloc(serviceSize, sizeof(MdnsService)));
        VerifyOrExit(services != nullptr, error = CHIP_ERROR_NO_MEMORY);
        current = results;

        for (size_t i = 0; i < serviceSize; i++, current = current->next)
        {
            mdns_ip_addr_t * addr          = current->addr;
            mdns_ip_addr_t * preferredAddr = addr;

            while (addr != nullptr)
            {
                if (addr->addr.type == ESP_IPADDR_TYPE_V6)
                {
                    preferredAddr = addr;

                    break;
                }
                addr = addr->next;
            }
            if (preferredAddr && preferredAddr->addr.type == ESP_IPADDR_TYPE_V6)
            {
                ip6_addr_t addrBinary;

                memcpy(&addrBinary, &preferredAddr->addr.u_addr.ip6, sizeof(addrBinary));
                services[i].mAddress.SetValue(chip::Inet::IPAddress::FromIPv6(addrBinary));
            }
            else if (preferredAddr && preferredAddr->addr.type == ESP_IPADDR_TYPE_V4)
            {
                ip4_addr_t addrBinary;

                memcpy(&addrBinary, &preferredAddr->addr.u_addr.ip4, sizeof(addrBinary));
                services[i].mAddress.SetValue(chip::Inet::IPAddress::FromIPv4(addrBinary));
            }
            if (current->instance_name)
            {
                strncpy(services[i].mName, current->instance_name, sizeof(services[i].mName));
            }
            services[i].mPort     = current->port;
            services[i].mProtocol = protocol;
            strncpy(services[i].mType, type, sizeof(services[i].mType));
        }
        ChipLogProgress(DeviceLayer, "Call callback");
        callback(context, services, serviceSize, CHIP_NO_ERROR);
    }

exit:
    if (services != nullptr)
    {
        chip::Platform::MemoryFree(services);
    }
    if (results != nullptr)
    {
        mdns_query_results_free(results);
    }

    return error;
}

CHIP_ERROR ChipMdnsResolve(MdnsService * service, chip::Inet::InterfaceId interface, MdnsResolveCallback callback, void * context)
{
    CHIP_ERROR error           = CHIP_NO_ERROR;
    mdns_result_t * textResult = nullptr;
    TextEntry * textEntries;
    esp_err_t espError;

    VerifyOrExit(service != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    espError = mdns_query_txt(service->mName, service->mType, GetProtocolString(service->mProtocol), kTimeoutMilli, &textResult);
    VerifyOrExit(espError == ESP_OK && textResult != nullptr, error = CHIP_ERROR_INTERNAL);

    if (textResult->txt_count > 0)
    {
        textEntries = static_cast<TextEntry *>(chip::Platform::MemoryCalloc(textResult->txt_count, sizeof(TextEntry)));
        for (size_t i = 0; i < textResult->txt_count; i++)
        {
            textEntries[i].mKey      = textResult->txt[i].key;
            textEntries[i].mData     = reinterpret_cast<const uint8_t *>(textResult->txt[i].value);
            textEntries[i].mDataSize = strlen(textResult->txt[i].value);
        }
        service->mTextEntryies = textEntries;
    }
    service->mTextEntrySize = textResult->txt_count;

    callback(context, service, error);

exit:
    if (textEntries != nullptr)
    {
        chip::Platform::MemoryFree(textEntries);
    }
    if (textResult != nullptr)
    {
        mdns_query_results_free(textResult);
    }

    return error;
}

} // namespace Mdns
} // namespace Protocols
} // namespace chip
