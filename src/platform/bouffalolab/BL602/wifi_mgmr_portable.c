#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <FreeRTOS.h>
#include <aos/yloop.h>
#include <bl60x_wifi_driver/wifi_mgmr.h>
#include <bl60x_wifi_driver/wifi_mgmr_api.h>
#include <bl60x_wifi_driver/wifi_mgmr_profile.h>
#include <task.h>

#include <supplicant_api.h>

#include <wpa_supplicant/src/utils/common.h>

#include <wpa_supplicant/src/common/defs.h>
#include <wpa_supplicant/src/common/wpa_common.h>
#include <wpa_supplicant/src/rsn_supp/wpa_i.h>

#include <wifi_mgmr_portable.h>

extern struct wpa_sm gWpaSm;
static netif_ext_callback_t netifExtCallback;

int wifi_mgmr_get_bssid(uint8_t * bssid)
{
    int i;

    for (i = 0; i < sizeof(wifiMgmr.wifi_mgmr_stat_info.bssid); i++)
    {
        bssid[i] = wifiMgmr.wifi_mgmr_stat_info.bssid[i];
    }

    return 0;
}

int wifi_mgmr_get_scan_ap_num(void)
{
    int num, count;

    num   = sizeof(wifiMgmr.scan_items) / sizeof(wifiMgmr.scan_items[0]);
    count = 0;

    for (int i = 0; i < num; i++)
    {
        if (wifiMgmr.scan_items[i].is_used && (!wifi_mgmr_scan_item_is_timeout(&wifiMgmr, &wifiMgmr.scan_items[i])))
        {
            count++;
        }
    }

    return count;
}

void wifi_mgmr_get_scan_result(wifi_mgmr_ap_item_t * result, int * num, uint8_t scan_type, char * ssid)
{
    int i, count, iter;

    count = sizeof(wifiMgmr.scan_items) / sizeof(wifiMgmr.scan_items[0]);
    iter  = 0;

    for (i = 0; i < count; i++)
    {
        if (wifiMgmr.scan_items[i].is_used && (!wifi_mgmr_scan_item_is_timeout(&wifiMgmr, &wifiMgmr.scan_items[i])))
        {
            if (scan_type)
            {
                if (memcmp(ssid, wifiMgmr.scan_items[i].ssid, wifiMgmr.scan_items[i].ssid_len) != 0)
                {
                    continue;
                }
            }
            memcpy(result[iter].ssid, wifiMgmr.scan_items[i].ssid, wifiMgmr.scan_items[i].ssid_len);
            result[iter].ssid[wifiMgmr.scan_items[i].ssid_len] = 0;
            result[iter].ssid_tail[0]                          = 0;
            result[iter].ssid_len                              = wifiMgmr.scan_items[i].ssid_len;
            memcpy((&(result[iter]))->bssid, wifiMgmr.scan_items[i].bssid, 6);
            result[iter].channel = wifiMgmr.scan_items[i].channel;
            result[iter].auth    = wifiMgmr.scan_items[i].auth;
            result[iter].rssi    = wifiMgmr.scan_items[i].rssi;
            iter++;
        }
    }

    *num = iter;
}

int wifi_mgmr_get_scan_result_filter(wifi_mgmr_ap_item_t * result, char * ssid)
{
    int i, count;

    count = sizeof(wifiMgmr.scan_items) / sizeof(wifiMgmr.scan_items[0]);
    for (i = 0; i < count; i++)
    {
        if (wifiMgmr.scan_items[i].is_used && (!wifi_mgmr_scan_item_is_timeout(&wifiMgmr, &wifiMgmr.scan_items[i])) &&
            !strncmp(ssid, wifiMgmr.scan_items[i].ssid, wifiMgmr.scan_items[i].ssid_len))
        {
            memcpy(result->ssid, wifiMgmr.scan_items[i].ssid, wifiMgmr.scan_items[i].ssid_len);
            result->ssid[wifiMgmr.scan_items[i].ssid_len] = 0;
            result->ssid_tail[0]                          = 0;
            result->ssid_len                              = wifiMgmr.scan_items[i].ssid_len;
            memcpy(result->bssid, wifiMgmr.scan_items[i].bssid, 6);
            result->channel = wifiMgmr.scan_items[i].channel;
            result->auth    = wifiMgmr.scan_items[i].auth;
            result->rssi    = wifiMgmr.scan_items[i].rssi;
            return 0;
        }
    }

    return -1;
}

int wifi_mgmr_profile_ssid_get(uint8_t * ssid)
{
    wifi_mgmr_profile_msg_t profile_msg;

    wifi_mgmr_profile_get_by_idx(&wifiMgmr, &profile_msg, wifiMgmr.profile_active_index);

    memcpy(ssid, profile_msg.ssid, profile_msg.ssid_len);

    return profile_msg.ssid_len;
}

bool wifi_mgmr_security_type_is_open(void)
{
    return strlen(wifiMgmr.wifi_mgmr_stat_info.passphr) == 0;
}

bool wifi_mgmr_security_type_is_wpa(void)
{
    return WPA_PROTO_WPA == gWpaSm.proto;
}

bool wifi_mgmr_security_type_is_wpa2(void)
{
    if (WPA_PROTO_RSN == gWpaSm.proto)
    {
        return (gWpaSm.key_mgmt &
                (WPA_KEY_MGMT_IEEE8021X | WPA_KEY_MGMT_PSK | WPA_KEY_MGMT_PSK_SHA256 | WPA_KEY_MGMT_FT_PSK |
                 WPA_KEY_MGMT_IEEE8021X_SHA256 | WPA_KEY_MGMT_FT_IEEE8021X)) != 0;
    }

    return false;
}

bool wifi_mgmr_security_type_is_wpa3(void)
{
    if (WPA_PROTO_RSN == gWpaSm.proto)
    {
        return (gWpaSm.key_mgmt & (WPA_KEY_MGMT_SAE | WPA_KEY_MGMT_FT_SAE)) != 0;
    }

    return false;
}

struct netif * deviceInterface_getNetif(void)
{
    return wifi_mgmr_sta_netif_get();
}

static void wifi_event_handler_raw(input_event_t * event, void * private_data)
{
    wifi_event_handler(event->code);
}

void wifi_start_firmware_task(void)
{
#define WIFI_STACK_SIZE 1552
    static StackType_t wifi_fw_stack[WIFI_STACK_SIZE];
    static StaticTask_t wifi_fw_task;

    aos_register_event_filter(EV_WIFI, wifi_event_handler_raw, NULL);
    netif_add_ext_callback(&netifExtCallback, network_netif_ext_callback);

    bl_pm_init();
    xTaskCreateStatic(wifi_main, (char *) "fw", WIFI_STACK_SIZE, NULL, 30, wifi_fw_stack, &wifi_fw_task);

    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
}
