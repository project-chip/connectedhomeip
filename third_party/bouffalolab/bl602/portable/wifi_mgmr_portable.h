#ifndef __WIFI_MGMR_POARABLE_H__
#define __WIFI_MGMR_POARABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

void wifi_mgmr_sta_ssid_get(char * ssid);
int mgmr_get_security_type(void);
int wifi_mgmr_get_bssid(uint8_t * bssid);
int mgmr_get_current_channel_num(void);
int mgmr_get_rssi(void);
void wifi_mgmr_conn_result_get(uint16_t * status_code, uint16_t * reason_code);
int wifi_mgmr_profile_ssid_get(uint8_t * ssid);
int wifi_mgmr_get_scan_ap_num(void);
void wifi_mgmr_get_scan_result(wifi_mgmr_ap_item_t * result, int * num, uint8_t scan_type, char * ssid);

#ifdef __cplusplus
}
#endif

#endif
