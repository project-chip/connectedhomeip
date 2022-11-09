#include "wifi_mgmr.h"
#include "wifi_mgmr_api.h"

#include <blog.h>
#include <stdint.h>
#include <string.h>

BLOG_DECLARE(tcal_power)
BLOG_DECLARE(scan)
#define USER_UNUSED(a) ((void) (a))

#define DEBUG_HEADER "[WF][SM] "

int wifi_mgmr_get_bssid(uint8_t * bssid)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        bssid[i] = wifiMgmr.wifi_mgmr_stat_info.bssid[i];
    }

    return;
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
