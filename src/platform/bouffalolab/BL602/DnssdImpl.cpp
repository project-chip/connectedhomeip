/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <BL602Config.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip6_addr.h>
#include <lwip/netifapi.h>
#include <mdns.h>
#include <mdns_server.h>
#include <wifi_mgmr_ext.h>

using namespace chip::Dnssd;

using namespace ::chip::DeviceLayer::Internal;
namespace {

static constexpr uint32_t kTimeoutMilli = 3000;
static constexpr size_t kMaxResults     = 20;
} // namespace

namespace chip {
namespace Dnssd {

#define MDNS_MAX_PACKET_SIZE 64
typedef struct
{
    const char * key;   /*!< item key name */
    const char * value; /*!< item value string */
    size_t value_len;
} mdns_txt_item_t;

typedef struct mdns
{
    struct netif * netif;
    uint8_t slot[10];
    uint8_t slot_idx;
    int txt_cnt;
} mdns_t;

#define MDNS_TXT_MAX_LEN 128
static mdns_t mdns      = { NULL, 0, 0, 0, 0 };
mdns_txt_item_t * items = nullptr;
uint8_t packet[MDNS_TXT_MAX_LEN];

static const DnssdService * glservice;

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    mdns_resp_init();
    mdns.slot_idx = 0;
    initCallback(context, error);

    glservice = static_cast<DnssdService *>(chip::Platform::MemoryCalloc(1, sizeof(DnssdService)));

    return error;
}

void ChipDnssdShutdown() {}

static const char * GetProtocolString(DnssdServiceProtocol protocol)
{
    return protocol == DnssdServiceProtocol::kDnssdProtocolTcp ? "_tcp" : "_udp";
}

static inline uint8_t _mdns_append_u8(uint8_t * packet, uint16_t * index, uint8_t value)
{
    if (*index >= MDNS_MAX_PACKET_SIZE)
    {
        return 0;
    }

    packet[*index] = value;
    *index += 1;
    return 1;
}

static inline int append_one_txt_record_entry(uint8_t * packet, uint16_t * index, mdns_txt_item_t * txt)
{
    if (txt == NULL || txt->key == NULL || packet == NULL)
    {
        return -1;
    }

    size_t key_len = strlen(txt->key);
    size_t len     = key_len + txt->value_len + (txt->value ? 1 : 0);
    if ((*index + len + 1) >= MDNS_MAX_PACKET_SIZE)
    {
        return 0;
    }

    _mdns_append_u8(packet, index, len);
    memcpy(packet + *index, txt->key, key_len);
    if (txt->value)
    {
        packet[*index + key_len] = '=';
        memcpy(packet + *index + key_len + 1, txt->value, txt->value_len);
    }

    *index += len;

    return len + 1;
}

static void dnssd_txt_resolve(uint8_t * packet, mdns_txt_item_t * txt, int count)
{
    uint16_t index = 0;

    for (int i = 0; i < count; i++)
    {
        append_one_txt_record_entry(packet, &index, &(txt[i]));
    }
}

static void srv_txt(struct mdns_service * service, void * txt_userdata)
{
    int i, ret;
    int index = 0;

    for (i = 0; i < mdns.txt_cnt; i++)
    {
        ret = mdns_resp_add_service_txtitem(service, &(packet[index + 1]), packet[index]);
        if (ret)
        {
            log_info("send txt failed.\r\n");

            return;
        }

        index = index + packet[index] + 1;
    }
}

static void ota_txt(struct mdns_service * service, void * txt_userdata)
{
    int ret = mdns_resp_add_service_txtitem(service, "version=12345678", 16);
    if (ret)
    {
        log_info("send ota txt failed.\r\n");
    }
}

static err_t mdns_responder_stop_netifapi_errt_fn(struct netif * netif)
{
    return mdns_responder_stop(netif);
}

int mdns_responder_ops(struct netif * netif)
{
    int ret, slot = -1;
    int protocol        = 0;
    uint16_t packet_len = 0;

    if (netif == NULL)
    {
        log_info("netif is NULL\r\n");
        return -1;
    }

    if (!(mdns.netif))
    {
        mdns.netif = netif;

        ret = mdns_resp_add_netif(netif, glservice->mHostName, 10);
        if (ret != 0)
        {
            mdns_resp_deinit();
            log_info("add netif failed:%d\r\n", ret);
            return -1;
        }
    }

    items        = static_cast<mdns_txt_item_t *>(chip::Platform::MemoryCalloc(glservice->mTextEntrySize, sizeof(mdns_txt_item_t)));
    mdns.txt_cnt = glservice->mTextEntrySize;
    for (size_t i = 0; i < glservice->mTextEntrySize; i++)
    {
        items[i].key       = glservice->mTextEntries[i].mKey;
        items[i].value     = reinterpret_cast<const char *>(glservice->mTextEntries[i].mData);
        items[i].value_len = glservice->mTextEntries[i].mDataSize;
        packet_len         = packet_len + strlen(items[i].key) + items[i].value_len + 1;
    }

    if (MDNS_TXT_MAX_LEN < packet_len)
    {
        return -1;
    }

    dnssd_txt_resolve(packet, items, glservice->mTextEntrySize);
    chip::Platform::MemoryFree(items);

    log_info("name = %s nType = %s protocol = %d port = %d \r\n", glservice->mName, glservice->mType, protocol, glservice->mPort);
    slot = mdns_resp_add_service(netif, glservice->mName, glservice->mType, static_cast<uint8_t>(glservice->mProtocol),
                                 glservice->mPort, 60, srv_txt, NULL);
    if (slot < 0)
    {
        mdns_resp_remove_netif(netif);
        mdns_resp_deinit();
        log_info("add server failed:%d\r\n", slot);
        return -1;
    }

    mdns.slot[mdns.slot_idx] = slot;
    mdns.slot_idx++;
    mdns_resp_announce(netif);

    return slot;
}

static err_t mdns_responder_start_netifapi_errt_fn(struct netif * netif)
{
    return mdns_responder_ops(netif);
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    struct netif * netif;
    int slot;
    bool mdns_flag;

    if (!(chip::DeviceLayer::ConnectivityMgrImpl()._IsWiFiStationConnected()))
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (service)
    {
        memcpy(glservice, service, sizeof(DnssdService));
    }

    netif = wifi_mgmr_sta_netif_get();
    if (netif == NULL)
    {
        log_info("find failed\r\n");
        return CHIP_ERROR_INTERNAL;
    }

    // mdns_responder_ops(netif);

    slot = netifapi_netif_common(netif, NULL, mdns_responder_start_netifapi_errt_fn);
    if (slot < 0)
    {
        log_info("start mdns failed\r\n");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    struct netif * netif;
    int i = 0;

    netif = wifi_mgmr_sta_netif_get();
    if (netif == NULL)
    {
        log_info("find failed\r\n");
        return CHIP_ERROR_INTERNAL;
    }

    for (i = 0; i < mdns.slot_idx; i++)
    {
        mdns_resp_del_service(netif, mdns.slot[i]);
    }

    mdns.slot_idx = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * /*type*/, DnssdServiceProtocol /*protocol*/, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId /*interface*/, DnssdBrowseCallback /*callback*/, void * /*context*/)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ChipDnssdResolve(DnssdService * /*service*/, chip::Inet::InterfaceId /*interface*/, DnssdResolveCallback /*callback*/,
                            void * /*context*/)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
