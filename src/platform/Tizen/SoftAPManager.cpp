/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <platform/CHIPDeviceLayer.h>
#include <system_info.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "MainLoop.h"
#include "SoftAPManager.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

SoftAPManager SoftAPManager::sInstance;

const char * softap_feature_key = "tizen.org/feature/network.wifi.softap";

static const char * __SoftAPDisabledCauseToStr(softap_disabled_cause_e disabledCode)
{
    switch (disabledCode)
    {
    case SOFTAP_DISABLED_BY_FLIGHT_MODE:
        return "disabled due to flight mode on";
    case SOFTAP_DISABLED_BY_LOW_BATTERY:
        return "disabled due to low battery";
    case SOFTAP_DISABLED_BY_NETWORK_CLOSE:
        return "disabled due to pdp network close";
    case SOFTAP_DISABLED_BY_TIMEOUT:
        return "disabled due to timeout";
    case SOFTAP_DISABLED_BY_OTHERS:
        return "disabled by other apps";
    case SOFTAP_DISABLED_BY_REQUEST:
        return "disabled by my request";
    case SOFTAP_DISABLED_BY_WIFI_ON:
        return "disabled by Wi-Fi station on";
    default:
        return "disabled by unknown reason";
    }
}

static const char * __SoftAPSecurityTypeToStr(softap_security_type_e securityType)
{
    switch (securityType)
    {
    case SOFTAP_SECURITY_TYPE_NONE:
        return "None";
    case SOFTAP_SECURITY_TYPE_WPA2_PSK:
        return "WPA2 PSK";
    case SOFTAP_SECURITY_TYPE_WPS:
        return "WPS";
    default:
        return "(unknown)";
    }
}

void SoftAPManager::_EnabledCb(softap_error_e softAPErr, bool isRequested, void * userData)
{
    if (softAPErr != SOFTAP_ERROR_NONE)
    {
        VerifyOrReturn(isRequested == true);

        ChipLogProgress(DeviceLayer, "FAIL: enable SoftAP [%s]", get_error_message(softAPErr));

        if (sInstance.mEnableLoop)
        {
            g_main_loop_quit(sInstance.mEnableLoop);
            sInstance.mEnableLoop = NULL;
        }
        return;
    }

    if (isRequested)
    {
        ChipLogProgress(DeviceLayer, "SoftAP is enabled by my request");

        if (sInstance.mEnableLoop)
        {
            g_main_loop_quit(sInstance.mEnableLoop);
            sInstance.mEnableLoop = NULL;
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "SoftAP is enabled by other App");
    }
}

void SoftAPManager::_DisabledCb(softap_error_e softAPErr, softap_disabled_cause_e disabledCause, void * userData)
{
    if (softAPErr != SOFTAP_ERROR_NONE)
    {
        VerifyOrReturn(disabledCause == SOFTAP_DISABLED_BY_REQUEST);

        ChipLogProgress(DeviceLayer, "FAIL: enable SoftAP [%s]", get_error_message(softAPErr));

        if (sInstance.mDisableLoop)
        {
            g_main_loop_quit(sInstance.mDisableLoop);
            sInstance.mDisableLoop = NULL;
        }
        return;
    }

    ChipLogProgress(DeviceLayer, "SoftAP is %s", __SoftAPDisabledCauseToStr(disabledCause));

    if (disabledCause == SOFTAP_DISABLED_BY_REQUEST)
    {
        if (sInstance.mDisableLoop)
        {
            g_main_loop_quit(sInstance.mDisableLoop);
            sInstance.mDisableLoop = NULL;
        }
    }
}

void SoftAPManager::_SecurityTypeChangedCb(softap_security_type_e securityType, void * userData)
{
    ChipLogProgress(DeviceLayer, "SoftAP security type changed [%s]", __SoftAPSecurityTypeToStr(securityType));
}

void SoftAPManager::_SsidVisibilityChangedCb(bool visibility, void * userData)
{
    ChipLogProgress(DeviceLayer, "SoftAP ssid visibility changed [%s]", visibility ? "Visible" : "Invisible");
}

void SoftAPManager::_PassphraseChangedCb(void * userData)
{
    ChipLogProgress(DeviceLayer, "SoftAP passphrase is changed");
}

void SoftAPManager::_ClientConnectionStateChangedCb(softap_client_h client, bool open, void * userData)
{
    int softAPErr               = SOFTAP_ERROR_NONE;
    softap_client_h cloneClient = NULL;
    char * ipAddress            = NULL;
    char * macAddress           = NULL;
    char * hostName             = NULL;

    softAPErr = softap_client_clone(&cloneClient, client);
    VerifyOrReturn(softAPErr == SOFTAP_ERROR_NONE);

    softap_client_get_ip_address(cloneClient, SOFTAP_ADDRESS_FAMILY_IPV4, &ipAddress);
    softap_client_get_mac_address(cloneClient, &macAddress);
    softap_client_get_name(cloneClient, &hostName);

    ChipLogProgress(DeviceLayer, "%s station IP [%s] MAC [%s] Hostname [%s]", open ? "Connected" : "Disconnected", ipAddress,
                    macAddress, hostName);

    free(ipAddress);
    free(macAddress);
    free(hostName);

    softap_client_destroy(cloneClient);
}

gboolean SoftAPManager::_SoftAPInitialize(gpointer userData)
{
    int softAPErr = SOFTAP_ERROR_NONE;

    softAPErr = softap_create(&(sInstance.mSoftAPHandle));
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "SoftAP is created");
        sInstance._SoftAPSetCallbacks();
    }
    else
    {
        ChipLogProgress(DeviceLayer, "FAIL: create SoftAP [%s]", get_error_message(softAPErr));
        return false;
    }

    return true;
}

gboolean SoftAPManager::_SoftAPEnable(GMainLoop * mainLoop, gpointer userData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int softAPErr  = SOFTAP_ERROR_NONE;

    sInstance.mEnableLoop = mainLoop;

    err = sInstance._SoftAPConfigure();
    VerifyOrReturnError(err == CHIP_NO_ERROR, false);

    softAPErr = softap_enable(sInstance.mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "SoftAP enable is requested");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "FAIL: request SoftAP enable [%s]", get_error_message(softAPErr));
        return false;
    }

    return true;
}

gboolean SoftAPManager::_SoftAPDisable(GMainLoop * mainLoop, gpointer userData)
{
    int softAPErr = SOFTAP_ERROR_NONE;

    sInstance.mDisableLoop = mainLoop;

    softAPErr = softap_disable(sInstance.mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "SoftAP disable is requested");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "FAIL: request SoftAP disable [%s]", get_error_message(softAPErr));
        return false;
    }

    return true;
}

void SoftAPManager::_SoftAPCheckSupportedFeature(void)
{
    int sysInfoErr = SYSTEM_INFO_ERROR_NONE;

    mSupportedFeature = false;
    sysInfoErr        = system_info_get_platform_bool(softap_feature_key, &mSupportedFeature);
    if (sysInfoErr == SYSTEM_INFO_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "SoftAP feature: %s", mSupportedFeature ? "Supported" : "Unsupported");
    }
    else
    {
        ChipLogError(DeviceLayer, "FAIL: get platform system info [%d]", sysInfoErr);
    }
}

CHIP_ERROR SoftAPManager::_SoftAPConfigure(void)
{
    CHIP_ERROR err                      = CHIP_NO_ERROR;
    int softAPErr                       = SOFTAP_ERROR_NONE;
    uint16_t discriminator              = 0;
    uint32_t setupPinCode               = 0;
    uint16_t vendorID                   = 0;
    uint16_t productID                  = 0;
    char softAPSsid[kMaxWiFiSSIDLength] = {
        0,
    };

    err = ConfigurationMgr().GetSetupDiscriminator(discriminator);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "discriminator[0x%03x]", discriminator & 0xFFF);
    }

    err = ConfigurationMgr().GetSetupPinCode(setupPinCode);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "PinCode[%09xlu]", setupPinCode);
    }

    /* CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID */
    err = ConfigurationMgr().GetVendorId(vendorID);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "VendorId[%hu (0x%hx)]", vendorID, vendorID);
    }

    /* CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID */
    err = ConfigurationMgr().GetProductId(productID);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "ProductId[%hu (0x%hx)]", productID, productID);
    }

    /*
     * 5.4.2.9.2. AP Operating Parameters
     * SSID : CHIP-ddd-vvvv-pppp
     * ddd is the 12-bit Discriminator in hex digits
     * vvvv is the 16-bit Vendor ID (VID) in hex digits
     * pppp is the 16-bit Product ID (PID) in hex digits
     */
    snprintf(softAPSsid, kMaxWiFiSSIDLength, "%s%03X-%04X-%04X", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX, discriminator, vendorID,
             productID);
    ChipLogProgress(DeviceLayer, "Configuring WiFi AP: SSID %s", softAPSsid);

    softAPErr = softap_set_ssid(mSoftAPHandle, softAPSsid);
    VerifyOrReturnError(softAPErr == SOFTAP_ERROR_NONE, CHIP_ERROR_INCORRECT_STATE);

    softAPErr = softap_set_ssid_visibility(mSoftAPHandle, true);
    VerifyOrReturnError(softAPErr == SOFTAP_ERROR_NONE, CHIP_ERROR_INCORRECT_STATE);

    softAPErr = softap_set_channel(mSoftAPHandle, CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL);
    VerifyOrReturnError(softAPErr == SOFTAP_ERROR_NONE, CHIP_ERROR_INCORRECT_STATE);

    softAPErr = softap_set_security_type(mSoftAPHandle, SOFTAP_SECURITY_TYPE_NONE);
    VerifyOrReturnError(softAPErr == SOFTAP_ERROR_NONE, CHIP_ERROR_INCORRECT_STATE);

    return err;
}

void SoftAPManager::_SoftAPDeinitialize(void)
{
    int softAPErr = SOFTAP_ERROR_NONE;

    sInstance._SoftAPUnsetCallbacks();

    softAPErr = softap_destroy(sInstance.mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "SoftAP is destroyed");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "FAIL: destroy SoftAP [%s]", get_error_message(softAPErr));
    }
}

void SoftAPManager::_SoftAPSetCallbacks(void)
{
    int softAPErr = SOFTAP_ERROR_NONE;

    softAPErr = softap_set_enabled_cb(mSoftAPHandle, _EnabledCb, NULL);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set SoftAP enabled callback");
    }

    softAPErr = softap_set_disabled_cb(mSoftAPHandle, _DisabledCb, NULL);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set SoftAP disabled callback");
    }

    softAPErr = softap_set_security_type_changed_cb(mSoftAPHandle, _SecurityTypeChangedCb, NULL);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set SoftAP security type changed callback");
    }

    softAPErr = softap_set_ssid_visibility_changed_cb(mSoftAPHandle, _SsidVisibilityChangedCb, NULL);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set SoftAP ssid visibility changed callback");
    }

    softAPErr = softap_set_passphrase_changed_cb(mSoftAPHandle, _PassphraseChangedCb, NULL);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set SoftAP passphrase changed callback");
    }

    softAPErr = softap_set_client_connection_state_changed_cb(mSoftAPHandle, _ClientConnectionStateChangedCb, NULL);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Set SoftAP client connection state changed callback");
    }
}

void SoftAPManager::_SoftAPUnsetCallbacks(void)
{
    int softAPErr = SOFTAP_ERROR_NONE;

    softAPErr = softap_unset_enabled_cb(mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset SoftAP enabled callback");
    }

    softAPErr = softap_unset_disabled_cb(mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset SoftAP disabled callback");
    }

    softAPErr = softap_unset_security_type_changed_cb(mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset SoftAP security type changed callback");
    }

    softAPErr = softap_unset_ssid_visibility_changed_cb(mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset SoftAP ssid visibility changed callback");
    }

    softAPErr = softap_unset_passphrase_changed_cb(mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset SoftAP passphrase changed callback");
    }

    softAPErr = softap_unset_client_connection_state_changed_cb(mSoftAPHandle);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "Unset SoftAP client connection state changed callback");
    }
}

void SoftAPManager::Init(void)
{
    sInstance.mEnableLoop  = NULL;
    sInstance.mDisableLoop = NULL;

    sInstance._SoftAPCheckSupportedFeature();
    VerifyOrReturn(sInstance.mSupportedFeature);

    MainLoop::Instance().Init(_SoftAPInitialize);
}

void SoftAPManager::Deinit(void)
{
    VerifyOrReturn(sInstance.mSupportedFeature);

    sInstance._SoftAPDeinitialize();
    MainLoop::Instance().Deinit();
}

CHIP_ERROR SoftAPManager::IsEnabled(bool * isSoftAPEnabled)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int softAPErr  = SOFTAP_ERROR_NONE;

    VerifyOrReturnError(sInstance.mSupportedFeature, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    softAPErr = softap_is_enabled(sInstance.mSoftAPHandle, isSoftAPEnabled);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        ChipLogProgress(DeviceLayer, "SoftAP is %s", *isSoftAPEnabled ? "enabled" : "disabled");
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogProgress(DeviceLayer, "FAIL: check whether SoftAP is activated [%s]", get_error_message(softAPErr));
    }

    return err;
}

CHIP_ERROR SoftAPManager::Enable(void)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    int softAPErr        = SOFTAP_ERROR_NONE;
    bool isSoftAPEnabled = false;
    bool dbusAsyncErr    = false;

    VerifyOrReturnError(sInstance.mSupportedFeature, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    softAPErr = softap_is_enabled(sInstance.mSoftAPHandle, &isSoftAPEnabled);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        if (isSoftAPEnabled == false)
        {
            dbusAsyncErr = MainLoop::Instance().AsyncRequest(_SoftAPEnable);
            if (dbusAsyncErr == false)
            {
                err = CHIP_ERROR_INCORRECT_STATE;
            }
        }
        else
        {
            ChipLogProgress(DeviceLayer, "SoftAP is already enabled");
        }
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogProgress(DeviceLayer, "FAIL: check whether SoftAP is enabled [%s]", get_error_message(softAPErr));
    }

    return err;
}

CHIP_ERROR SoftAPManager::Disable(void)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    int softAPErr        = SOFTAP_ERROR_NONE;
    bool isSoftAPEnabled = false;
    bool dbusAsyncErr    = false;

    VerifyOrReturnError(sInstance.mSupportedFeature, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    softAPErr = softap_is_enabled(sInstance.mSoftAPHandle, &isSoftAPEnabled);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        if (isSoftAPEnabled == true)
        {
            dbusAsyncErr = MainLoop::Instance().AsyncRequest(_SoftAPDisable);
            if (dbusAsyncErr == false)
            {
                err = CHIP_ERROR_INCORRECT_STATE;
            }
        }
        else
        {
            ChipLogProgress(DeviceLayer, "SoftAP is already disabled");
        }
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogProgress(DeviceLayer, "FAIL: check whether SoftAP is enabled [%s]", get_error_message(softAPErr));
    }

    return err;
}

CHIP_ERROR SoftAPManager::GetAPState(bool * apState)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    int softAPErr        = SOFTAP_ERROR_NONE;
    bool isSoftAPEnabled = false;

    VerifyOrReturnError(sInstance.mSupportedFeature, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    softAPErr = softap_is_enabled(sInstance.mSoftAPHandle, &isSoftAPEnabled);
    if (softAPErr == SOFTAP_ERROR_NONE)
    {
        *apState = isSoftAPEnabled;
        ChipLogProgress(DeviceLayer, "Get AP state [%s]", isSoftAPEnabled ? "Enabled" : "Disabled");
    }
    else
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogProgress(DeviceLayer, "FAIL: check whether SoftAP is enabled [%s]", get_error_message(softAPErr));
    }

    return err;
}

CHIP_ERROR SoftAPManager::SetAPState(bool apState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(sInstance.mSupportedFeature, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (apState == false)
    {
        err = Disable();
    }
    else
    {
        err = Enable();
    }

    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
