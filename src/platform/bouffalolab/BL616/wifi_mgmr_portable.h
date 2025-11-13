#ifndef __WIFI_MGMR_POARABLE_H__
#define __WIFI_MGMR_POARABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

int wifi_start_scan(const uint8_t * ssid, uint32_t length);
void network_netif_ext_callback(struct netif * netif, netif_nsc_reason_t reason, const netif_ext_callback_args_t * args);
void wifi_start_firmware_task(void);

struct netif * deviceInterface_getNetif(void);

#ifdef __cplusplus
}
#endif

#endif
