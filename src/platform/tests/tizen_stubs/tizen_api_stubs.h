#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void * gpointer;

static inline size_t g_strlcpy(char * dest, const char * src, size_t size)
{
    const size_t len = src == NULL ? 0 : strlen(src);
    if (size > 0)
    {
        const size_t copyLen = len >= size ? size - 1 : len;
        if (copyLen > 0)
        {
            memcpy(dest, src, copyLen);
        }
        dest[copyLen] = '\0';
    }
    return len;
}

#define BT_CHECK_LE_SUPPORT()
#define BT_CHECK_INIT_STATUS()
#define BT_CHECK_INPUT_PARAMETER(parameter)                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        if (!(parameter))                                                                                                          \
        {                                                                                                                          \
            return BT_ERROR_INVALID_PARAMETER;                                                                                     \
        }                                                                                                                          \
    } while (0)

#if defined(__clang__)
#define TIZENRT_STUB_NO_SANITIZE __attribute__((no_sanitize("address", "undefined", "memory")))
#elif defined(__GNUC__)
#define TIZENRT_STUB_NO_SANITIZE __attribute__((no_sanitize("address", "undefined")))
#else
#define TIZENRT_STUB_NO_SANITIZE
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    BT_ERROR_NONE              = 0,
    BT_ERROR_NOT_SUPPORTED     = -1,
    BT_ERROR_INVALID_PARAMETER = -2,
    BT_ERROR_NO_DATA           = -3,
    BT_ERROR_OUT_OF_MEMORY     = -4,
};

enum
{
    BT_ADAPTER_LE_PACKET_ADVERTISING   = 0,
    BT_ADAPTER_LE_PACKET_SCAN_RESPONSE = 1,
};

enum
{
    BT_ADAPTER_LE_ADVERTISING_DATA_SERVICE_DATA = 0x16,
};

typedef void * bt_scan_filter_h;
typedef int bt_adapter_le_packet_type_e;

typedef struct bt_adapter_le_service_data_s
{
    char * service_uuid;
    char * service_data;
    int service_data_len;
} bt_adapter_le_service_data_s;

typedef struct bt_adapter_le_device_scan_result_info_s
{
    const char * remote_address;
    char * adv_data;
    int adv_data_len;
    char * scan_data;
    int scan_data_len;
} bt_adapter_le_device_scan_result_info_s;

TIZENRT_STUB_NO_SANITIZE int bt_adapter_le_get_scan_result_service_data_list(const bt_adapter_le_device_scan_result_info_s * info,
                                                                             bt_adapter_le_packet_type_e pkt_type,
                                                                             bt_adapter_le_service_data_s ** dataList, int * count);
TIZENRT_STUB_NO_SANITIZE int bt_adapter_le_free_service_data_list(bt_adapter_le_service_data_s * dataList, int count);

int bt_adapter_le_start_scan(void (*callback)(int, bt_adapter_le_device_scan_result_info_s *, void *), void * userData);
int bt_adapter_le_stop_scan(void);
int bt_adapter_le_is_scan_filter_supported(bool * isSupported);
int bt_adapter_le_scan_filter_create(bt_scan_filter_h * filter);
int bt_adapter_le_scan_filter_unregister(bt_scan_filter_h filter);
int bt_adapter_le_scan_filter_destroy(bt_scan_filter_h filter);
int bt_adapter_le_scan_filter_set_device_address(bt_scan_filter_h filter, const char * address);
int bt_adapter_le_scan_filter_set_service_uuid(bt_scan_filter_h filter, const char * uuid);
int bt_adapter_le_scan_filter_set_service_data(bt_scan_filter_h filter, const char * uuid, const char * serviceData,
                                               unsigned int serviceDataLen);
int bt_adapter_le_scan_filter_register(bt_scan_filter_h filter);

typedef void * wifi_manager_h;
typedef void * wifi_manager_ap_h;
typedef void * wifi_manager_config_h;

typedef int wifi_manager_error_e;
enum
{
    WIFI_MANAGER_ERROR_NONE           = 0,
    WIFI_MANAGER_ERROR_NOT_SUPPORTED  = -1,
    WIFI_MANAGER_ERROR_UNKNOWN        = -2,
    WIFI_MANAGER_ERROR_ALREADY_EXISTS = -3,
};

typedef enum
{
    WIFI_MANAGER_DEVICE_STATE_DEACTIVATED = 0,
    WIFI_MANAGER_DEVICE_STATE_ACTIVATED   = 1,
} wifi_manager_device_state_e;

typedef enum
{
    WIFI_MANAGER_MODULE_STATE_DETACHED = 0,
    WIFI_MANAGER_MODULE_STATE_ATTACHED = 1,
} wifi_manager_module_state_e;

typedef enum
{
    WIFI_MANAGER_CONNECTION_STATE_FAILURE       = 0,
    WIFI_MANAGER_CONNECTION_STATE_DISCONNECTED  = 1,
    WIFI_MANAGER_CONNECTION_STATE_ASSOCIATION   = 2,
    WIFI_MANAGER_CONNECTION_STATE_CONNECTED     = 3,
    WIFI_MANAGER_CONNECTION_STATE_CONFIGURATION = 4,
} wifi_manager_connection_state_e;

typedef enum
{
    WIFI_MANAGER_SCAN_STATE_NOT_SCANNING = 0,
    WIFI_MANAGER_SCAN_STATE_SCANNING     = 1,
} wifi_manager_scan_state_e;

typedef enum
{
    WIFI_MANAGER_RSSI_LEVEL_0 = 0,
} wifi_manager_rssi_level_e;

typedef enum
{
    WIFI_MANAGER_IP_CONFLICT_STATE_CONFLICT_NOT_DETECTED = 0,
    WIFI_MANAGER_IP_CONFLICT_STATE_CONFLICT_DETECTED     = 1,
} wifi_manager_ip_conflict_state_e;

typedef enum
{
    WIFI_MANAGER_SECURITY_TYPE_NONE       = 0,
    WIFI_MANAGER_SECURITY_TYPE_WEP        = 1,
    WIFI_MANAGER_SECURITY_TYPE_WPA_PSK    = 2,
    WIFI_MANAGER_SECURITY_TYPE_WPA2_PSK   = 3,
    WIFI_MANAGER_SECURITY_TYPE_EAP        = 4,
    WIFI_MANAGER_SECURITY_TYPE_WPA_FT_PSK = 5,
    WIFI_MANAGER_SECURITY_TYPE_SAE        = 6,
    WIFI_MANAGER_SECURITY_TYPE_OWE        = 7,
    WIFI_MANAGER_SECURITY_TYPE_DPP        = 8,
} wifi_manager_security_type_e;

typedef void (*wifi_manager_error_cb)(wifi_manager_error_e, void *);
typedef void (*wifi_manager_device_state_changed_cb)(wifi_manager_device_state_e, void *);
typedef void (*wifi_manager_module_state_changed_cb)(wifi_manager_module_state_e, void *);
typedef void (*wifi_manager_connection_state_changed_cb)(wifi_manager_connection_state_e, wifi_manager_ap_h, void *);
typedef void (*wifi_manager_scan_state_changed_cb)(wifi_manager_scan_state_e, void *);
typedef void (*wifi_manager_rssi_level_changed_cb)(wifi_manager_rssi_level_e, void *);
typedef void (*wifi_manager_ip_conflict_cb)(char *, wifi_manager_ip_conflict_state_e, void *);
typedef bool (*wifi_manager_found_ap_cb)(wifi_manager_ap_h, void *);
typedef bool (*wifi_manager_config_cb)(const wifi_manager_config_h, void *);

int wifi_manager_ap_get_essid(wifi_manager_ap_h ap, char ** essid);
int wifi_manager_ap_get_bssid(wifi_manager_ap_h ap, char ** bssid);
int wifi_manager_ap_get_rssi(wifi_manager_ap_h ap, int * rssi);
int wifi_manager_ap_get_security_type(wifi_manager_ap_h ap, wifi_manager_security_type_e * type);
int wifi_manager_ap_get_frequency(wifi_manager_ap_h ap, int * frequency);
int wifi_manager_ap_is_passphrase_required(wifi_manager_ap_h ap, bool * required);
int wifi_manager_ap_set_passphrase(wifi_manager_ap_h ap, const char * passphrase);
int wifi_manager_ap_clone(wifi_manager_ap_h * cloned, wifi_manager_ap_h ap);
int wifi_manager_ap_destroy(wifi_manager_ap_h ap);

int wifi_manager_initialize(wifi_manager_h * handle);
int wifi_manager_deinitialize(wifi_manager_h handle);
int wifi_manager_activate(wifi_manager_h handle, wifi_manager_error_cb callback, void * userData);
int wifi_manager_deactivate(wifi_manager_h handle, wifi_manager_error_cb callback, void * userData);
int wifi_manager_scan(wifi_manager_h handle, wifi_manager_error_cb callback, void * userData);
int wifi_manager_scan_specific_ap(wifi_manager_h handle, const char * essid, wifi_manager_error_cb callback, void * userData);
int wifi_manager_connect(wifi_manager_h handle, wifi_manager_ap_h ap, wifi_manager_error_cb callback, void * userData);
int wifi_manager_forget_ap(wifi_manager_h handle, wifi_manager_ap_h ap);
int wifi_manager_is_activated(wifi_manager_h handle, bool * activated);
int wifi_manager_get_module_state(wifi_manager_h handle, wifi_manager_module_state_e * state);
int wifi_manager_get_connection_state(wifi_manager_h handle, wifi_manager_connection_state_e * state);
int wifi_manager_get_connected_ap(wifi_manager_h handle, wifi_manager_ap_h * ap);
int wifi_manager_get_mac_address(wifi_manager_h handle, char ** mac);
int wifi_manager_foreach_found_ap(wifi_manager_h handle, wifi_manager_found_ap_cb callback, void * userData);
int wifi_manager_foreach_found_specific_ap(wifi_manager_h handle, wifi_manager_found_ap_cb callback, void * userData);
int wifi_manager_config_foreach_configuration(wifi_manager_h handle, wifi_manager_config_cb callback, void * userData);
int wifi_manager_config_get_name(wifi_manager_config_h config, char ** name);
int wifi_manager_config_get_security_type(wifi_manager_config_h config, wifi_manager_security_type_e * type);
int wifi_manager_config_remove(wifi_manager_h handle, wifi_manager_config_h config);

int wifi_manager_set_device_state_changed_cb(wifi_manager_h handle, wifi_manager_device_state_changed_cb callback, void * userData);
int wifi_manager_set_module_state_changed_cb(wifi_manager_h handle, wifi_manager_module_state_changed_cb callback, void * userData);
int wifi_manager_set_connection_state_changed_cb(wifi_manager_h handle, wifi_manager_connection_state_changed_cb callback,
                                                 void * userData);
int wifi_manager_set_scan_state_changed_cb(wifi_manager_h handle, wifi_manager_scan_state_changed_cb callback, void * userData);
int wifi_manager_set_rssi_level_changed_cb(wifi_manager_h handle, wifi_manager_rssi_level_changed_cb callback, void * userData);
int wifi_manager_set_background_scan_cb(wifi_manager_h handle, wifi_manager_error_cb callback, void * userData);
int wifi_manager_set_ip_conflict_cb(wifi_manager_h handle, wifi_manager_ip_conflict_cb callback, void * userData);
int wifi_manager_unset_device_state_changed_cb(wifi_manager_h handle);
int wifi_manager_unset_module_state_changed_cb(wifi_manager_h handle);
int wifi_manager_unset_connection_state_changed_cb(wifi_manager_h handle);
int wifi_manager_unset_scan_state_changed_cb(wifi_manager_h handle);
int wifi_manager_unset_rssi_level_changed_cb(wifi_manager_h handle);
int wifi_manager_unset_background_scan_cb(wifi_manager_h handle);
int wifi_manager_unset_ip_conflict_cb(wifi_manager_h handle);

const char * get_error_message(int error);

#ifdef __cplusplus
}
#endif
