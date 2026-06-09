#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
#include <wifi-manager.h>
}

#include <lib/support/Span.h>

namespace {

struct StubAp
{
    std::string essid;
    std::string bssid;
    int rssi                                  = 0;
    int frequency                             = 2412;
    wifi_manager_security_type_e securityType = WIFI_MANAGER_SECURITY_TYPE_NONE;
    int essidError                            = WIFI_MANAGER_ERROR_NONE;
    int bssidError                            = WIFI_MANAGER_ERROR_NONE;
    int rssiError                             = WIFI_MANAGER_ERROR_NONE;
    int securityError                         = WIFI_MANAGER_ERROR_NONE;
    int frequencyError                        = WIFI_MANAGER_ERROR_NONE;
};

StubAp gAp;
bool gHasConnectedAp     = true;
int gGetConnectedApError = WIFI_MANAGER_ERROR_NONE;

char * DupString(const std::string & value)
{
    char * copy = static_cast<char *>(std::malloc(value.size() + 1));
    if (copy == nullptr)
    {
        return nullptr;
    }
    std::memcpy(copy, value.c_str(), value.size() + 1);
    return copy;
}

std::string FuzzString(const uint8_t * data, size_t len, size_t maxLen)
{
    const size_t outLen = std::min(len, maxLen);
    return std::string(reinterpret_cast<const char *>(data), reinterpret_cast<const char *>(data + outLen));
}

std::string FuzzBssid(const uint8_t * data, size_t len, uint8_t mode)
{
    if ((mode & 0x03) == 0)
    {
        char bssid[18];
        std::snprintf(bssid, sizeof(bssid), "%02x:%02x:%02x:%02x:%02x:%02x", len > 0 ? data[0] : 0, len > 1 ? data[1] : 0,
                      len > 2 ? data[2] : 0, len > 3 ? data[3] : 0, len > 4 ? data[4] : 0, len > 5 ? data[5] : 0);
        return bssid;
    }

    if ((mode & 0x03) == 1)
    {
        return FuzzString(data, len, 48);
    }

    if ((mode & 0x03) == 2)
    {
        return "00:11:22:33:44";
    }

    return "gg:11:22:33:44:55";
}

wifi_manager_security_type_e FuzzSecurityType(uint8_t value)
{
    switch (value % 10)
    {
    case 0:
        return WIFI_MANAGER_SECURITY_TYPE_NONE;
    case 1:
        return WIFI_MANAGER_SECURITY_TYPE_WEP;
    case 2:
        return WIFI_MANAGER_SECURITY_TYPE_WPA_PSK;
    case 3:
        return WIFI_MANAGER_SECURITY_TYPE_WPA2_PSK;
    case 4:
        return WIFI_MANAGER_SECURITY_TYPE_EAP;
    case 5:
        return WIFI_MANAGER_SECURITY_TYPE_WPA_FT_PSK;
    case 6:
        return WIFI_MANAGER_SECURITY_TYPE_SAE;
    case 7:
        return WIFI_MANAGER_SECURITY_TYPE_OWE;
    case 8:
        return WIFI_MANAGER_SECURITY_TYPE_DPP;
    default:
        return static_cast<wifi_manager_security_type_e>(0x7f);
    }
}

int FuzzFrequency(uint8_t value)
{
    static constexpr int kFrequencies[] = { 0,     863,   902,   916,   931,   2412,  2472,  2484,  3600,
                                            3700,  5035,  5180,  5945,  5960,  5980,  5955,  58320, 59400,
                                            60480, 61560, 62640, 63720, 64800, 65880, 66960, 68040, 69120 };
    return kFrequencies[value % (sizeof(kFrequencies) / sizeof(kFrequencies[0]))];
}

} // namespace

extern "C" int wifi_manager_ap_get_essid(wifi_manager_ap_h, char ** essid)
{
    if (gAp.essidError != WIFI_MANAGER_ERROR_NONE)
    {
        return gAp.essidError;
    }
    *essid = DupString(gAp.essid);
    return *essid == nullptr ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_ap_get_bssid(wifi_manager_ap_h, char ** bssid)
{
    if (gAp.bssidError != WIFI_MANAGER_ERROR_NONE)
    {
        return gAp.bssidError;
    }
    *bssid = DupString(gAp.bssid);
    return *bssid == nullptr ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_ap_get_rssi(wifi_manager_ap_h, int * rssi)
{
    *rssi = gAp.rssi;
    return gAp.rssiError;
}

extern "C" int wifi_manager_ap_get_security_type(wifi_manager_ap_h, wifi_manager_security_type_e * type)
{
    *type = gAp.securityType;
    return gAp.securityError;
}

extern "C" int wifi_manager_ap_get_frequency(wifi_manager_ap_h, int * frequency)
{
    *frequency = gAp.frequency;
    return gAp.frequencyError;
}

extern "C" int wifi_manager_get_connected_ap(wifi_manager_h, wifi_manager_ap_h * ap)
{
    if (gGetConnectedApError != WIFI_MANAGER_ERROR_NONE)
    {
        *ap = nullptr;
        return gGetConnectedApError;
    }

    *ap = gHasConnectedAp ? reinterpret_cast<wifi_manager_ap_h>(&gAp) : nullptr;
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_ap_is_passphrase_required(wifi_manager_ap_h, bool * required)
{
    *required = false;
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_ap_set_passphrase(wifi_manager_ap_h, const char *)
{
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_ap_clone(wifi_manager_ap_h * cloned, wifi_manager_ap_h ap)
{
    *cloned = ap;
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_ap_destroy(wifi_manager_ap_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_initialize(wifi_manager_h * handle)
{
    *handle = reinterpret_cast<wifi_manager_h>(&gAp);
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_deinitialize(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_activate(wifi_manager_h, wifi_manager_error_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_deactivate(wifi_manager_h, wifi_manager_error_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_scan(wifi_manager_h, wifi_manager_error_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_scan_specific_ap(wifi_manager_h, const char *, wifi_manager_error_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_connect(wifi_manager_h, wifi_manager_ap_h, wifi_manager_error_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_forget_ap(wifi_manager_h, wifi_manager_ap_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_is_activated(wifi_manager_h, bool * activated)
{
    *activated = true;
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_get_module_state(wifi_manager_h, wifi_manager_module_state_e * state)
{
    *state = WIFI_MANAGER_MODULE_STATE_ATTACHED;
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_get_connection_state(wifi_manager_h, wifi_manager_connection_state_e * state)
{
    *state = WIFI_MANAGER_CONNECTION_STATE_CONNECTED;
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_get_mac_address(wifi_manager_h, char ** mac)
{
    *mac = DupString(gAp.bssid);
    return *mac == nullptr ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_foreach_found_ap(wifi_manager_h, wifi_manager_found_ap_cb callback, void * userData)
{
    callback(reinterpret_cast<wifi_manager_ap_h>(&gAp), userData);
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_foreach_found_specific_ap(wifi_manager_h handle, wifi_manager_found_ap_cb callback, void * userData)
{
    return wifi_manager_foreach_found_ap(handle, callback, userData);
}
extern "C" int wifi_manager_config_foreach_configuration(wifi_manager_h, wifi_manager_config_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_config_get_name(wifi_manager_config_h, char ** name)
{
    *name = DupString(gAp.essid);
    return *name == nullptr ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_config_get_security_type(wifi_manager_config_h, wifi_manager_security_type_e * type)
{
    *type = gAp.securityType;
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_config_remove(wifi_manager_h, wifi_manager_config_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" int wifi_manager_set_device_state_changed_cb(wifi_manager_h, wifi_manager_device_state_changed_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_set_module_state_changed_cb(wifi_manager_h, wifi_manager_module_state_changed_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_set_connection_state_changed_cb(wifi_manager_h, wifi_manager_connection_state_changed_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_set_scan_state_changed_cb(wifi_manager_h, wifi_manager_scan_state_changed_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_set_rssi_level_changed_cb(wifi_manager_h, wifi_manager_rssi_level_changed_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_set_background_scan_cb(wifi_manager_h, wifi_manager_error_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_set_ip_conflict_cb(wifi_manager_h, wifi_manager_ip_conflict_cb, void *)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_unset_device_state_changed_cb(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_unset_module_state_changed_cb(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_unset_connection_state_changed_cb(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_unset_scan_state_changed_cb(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_unset_rssi_level_changed_cb(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_unset_background_scan_cb(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}
extern "C" int wifi_manager_unset_ip_conflict_cb(wifi_manager_h)
{
    return WIFI_MANAGER_ERROR_NONE;
}

extern "C" const char * get_error_message(int)
{
    return "stub";
}

#define private public
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#include <platform/Tizen/WiFiManager.cpp>
#pragma clang diagnostic pop
#undef private

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t len)
{
    if (len < 8)
    {
        return 0;
    }

    const uint8_t flags   = data[0];
    const size_t ssidLen  = data[1] % std::min<size_t>(len - 7, 48);
    const uint8_t * ssid  = data + 7;
    const uint8_t * bssid = ssid + ssidLen;
    const size_t bssidLen = len - 7 - ssidLen;

    gAp.essid            = FuzzString(ssid, ssidLen, 96);
    gAp.bssid            = FuzzBssid(bssid, bssidLen, flags >> 2);
    gAp.rssi             = static_cast<int8_t>(data[2]);
    gAp.frequency        = FuzzFrequency(data[3]);
    gAp.securityType     = FuzzSecurityType(data[4]);
    gAp.essidError       = (flags & 0x01) ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
    gAp.bssidError       = (flags & 0x02) ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
    gAp.rssiError        = (data[5] & 0x01) ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
    gAp.securityError    = (data[5] & 0x02) ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
    gAp.frequencyError   = (data[5] & 0x04) ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;
    gHasConnectedAp      = (data[5] & 0x08) == 0;
    gGetConnectedApError = (data[6] & 0x02) ? WIFI_MANAGER_ERROR_UNKNOWN : WIFI_MANAGER_ERROR_NONE;

    std::vector<chip::DeviceLayer::NetworkCommissioning::WiFiScanResponse> scanResponses;
    chip::DeviceLayer::Internal::WiFiManager::_FoundAPOnScanCb(reinterpret_cast<wifi_manager_ap_h>(&gAp), &scanResponses);

    uint8_t mac[6] = {};
    chip::MutableByteSpan macSpan(mac, (data[6] & 0x01) ? sizeof(mac) - 1 : sizeof(mac));
    (void) chip::DeviceLayer::Internal::WiFiMgr().GetBssId(macSpan);

    return 0;
}
