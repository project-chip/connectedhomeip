#include <platform/internal/DeviceNetworkInfo.h>

using namespace ::chip::DeviceLayer::Internal;
class NetworkInfo
{
public:
    char WiFiSSID[kMaxWiFiSSIDLength];
    int8_t BSSID[6];
    WiFiAuthSecurityType security;
    int8_t RSSI;
    int8_t channel;

    const char * sec2str(WiFiAuthSecurityType sec)
    {
        switch (sec)
        {
        case kWiFiSecurityType_None:
            return "None";
        case kWiFiSecurityType_WEP:
            return "WEP";
        case kWiFiSecurityType_WPAPersonal:
            return "WPA";
        case kWiFiSecurityType_WPA2Personal:
            return "WPA2";
        case kWiFiSecurityType_WPA3Personal:
            return "WPA3";
        default:
            return "Unknown";
        }
    }
};
