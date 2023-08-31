/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include <lwip/dhcp6.h>
#include <lwip/dns.h>
#include <lwip/ethip6.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#include <wifi_mgmr_portable.h>

virt_net_t vnet_spi;
static struct bflbwifi_ap_record vnet_ap_record;
static netif_ext_callback_t netifExtCallback;

/* event callback */
static int virt_net_spi_event_cb(virt_net_t obj, enum virt_net_event_code code, void * opaque)
{
    struct bflbwifi_ap_record * record;

    assert(obj != NULL);

    if (VIRT_NET_EV_ON_LINK_STATUS_DONE == code)
    {
        netbus_fs_link_status_ind_cmd_msg_t * pkg_data;

        pkg_data = (netbus_fs_link_status_ind_cmd_msg_t *) ((struct pkg_protocol *) opaque)->payload;
        record   = &pkg_data->record;

        memcpy(&vnet_ap_record, record, sizeof(struct bflbwifi_ap_record));
        if (record->link_status == BF1B_WIFI_LINK_STATUS_DOWN)
        {
            code = VIRT_NET_EV_ON_DISCONNECT;
        }
        else
        {
            code = -1;
            return 0;
        }
    }
    else if (VIRT_NET_EV_ON_SCAN_DONE == code)
    {
        wifiInterface_eventScanDone((struct netif *) &obj->netif, opaque);
    }

    wifi_event_handler(code);

    return 0;
}

bool wifi_start_firmware_task(void)
{
    vnet_spi = virt_net_create(NULL);
    if (vnet_spi == NULL)
    {
        return false;
    }

    if (vnet_spi->init(vnet_spi))
    {
        return false;
    }

    virt_net_setup_callback(vnet_spi, virt_net_spi_event_cb, NULL);
    netifapi_netif_set_default((struct netif *) &vnet_spi->netif);

    netif_add_ext_callback(&netifExtCallback, network_netif_ext_callback);

    return true;
}

struct netif * deviceInterface_getNetif(void)
{
    if (vnet_spi)
    {
        return (struct netif *) &vnet_spi->netif;
    }

    return NULL;
}

struct netif * otbr_getBackboneNetif(void)
{
    if (vnet_spi)
    {
        return (struct netif *) &vnet_spi->netif;
    }

    return NULL;
}

void wifiInterface_getMacAddress(uint8_t * pmac)
{
    memcpy(pmac, vnet_spi->mac, sizeof(vnet_spi->mac));
}

void wifiInterface_connect(char * ssid, char * passwd)
{
    virt_net_connect_ap(vnet_spi, ssid, passwd);
}

void wifiInterface_disconnect(void)
{
    virt_net_disconnect(vnet_spi);
}

struct bflbwifi_ap_record * wifiInterface_getApInfo(void)
{
    memset(&vnet_ap_record, 0, sizeof(struct bflbwifi_ap_record));
    if (0 == virt_net_get_link_status(vnet_spi))
    {
        return &vnet_ap_record;
    }

    return NULL;
}

void wifiInterface_startScan(void)
{
    virt_net_scan(vnet_spi);
}
