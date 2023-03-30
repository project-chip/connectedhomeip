#ifndef __WIFI_MGMR_POARABLE_H__
#define __WIFI_MGMR_POARABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

void wifi_mgmr_sta_ssid_get(char * ssid);
int wifi_mgmr_get_bssid(uint8_t * bssid);
void wifi_mgmr_conn_result_get(uint16_t * status_code, uint16_t * reason_code);
int wifi_mgmr_profile_ssid_get(uint8_t * ssid);
int wifi_mgmr_get_scan_ap_num(void);
void wifi_mgmr_get_scan_result(wifi_mgmr_ap_item_t * result, int * num, uint8_t scan_type, char * ssid);

bool wifi_mgmr_security_type_is_open(void);
bool wifi_mgmr_security_type_is_wpa(void);
bool wifi_mgmr_security_type_is_wpa2(void);
bool wifi_mgmr_security_type_is_wpa3(void);

#ifdef __cplusplus
}
#endif

#endif
