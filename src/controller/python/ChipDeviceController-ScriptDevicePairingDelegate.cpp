/* See Project CHIP LICENSE file for licensing information. */

#include "ChipDeviceController-ScriptDevicePairingDelegate.h"

#include <transport/RendezvousSessionDelegate.h>

namespace chip {
namespace DeviceController {

void ScriptDevicePairingDelegate::SetWifiCredential(const char * ssid, const char * password)
{
    strncpy(mWifiSSID, ssid, sizeof(mWifiSSID));
    strncpy(mWifiPassword, password, sizeof(mWifiPassword));
}

void ScriptDevicePairingDelegate::OnNetworkCredentialsRequested(RendezvousDeviceCredentialsDelegate * callback)
{
    callback->SendNetworkCredentials(mWifiSSID, mWifiPassword);
}

void ScriptDevicePairingDelegate::OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                                    RendezvousDeviceCredentialsDelegate * callback)
{
    // TODO: Implement this
    ChipLogDetail(Controller, "ScriptDevicePairingDelegate::OnOperationalCredentialsRequested\n");
}

} // namespace DeviceController
} // namespace chip
