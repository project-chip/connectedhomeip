/* See Project CHIP LICENSE file for licensing information. */

#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

#include "DeviceNetworkProvisioningDelegateImpl.h"
#include "ServiceProvisioning.h"

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DeviceNetworkProvisioningDelegateImpl::_ProvisionWiFiNetwork(const char * ssid, const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NetworkProvisioning, "ESP32NetworkProvisioningDelegate: SSID: %s", ssid);
    err = SetWiFiStationProvisioning(ssid, key);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %s", chip::ErrorStr(err));
    }

    return err;
}

} // namespace DeviceLayer
} // namespace chip
