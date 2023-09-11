/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/ConfigurationManager.h>

#if !CHIP_DISABLE_PLATFORM_KVS
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
#endif

namespace chip {
namespace DeviceLayer {

bool ConfigurationManagerImpl::HasWiFiNetworkInformations()
{
#if !CHIP_DISABLE_PLATFORM_KVS
    uint8_t ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    size_t ssidLen = 0;
    uint8_t key[DeviceLayer::Internal::kMaxWiFiKeyLength];
    size_t keyLen = 0;

    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    VerifyOrReturnError(CHIP_NO_ERROR == kvs.Get(kWiFiSSIDKeyName, ssid, sizeof(ssid), &ssidLen), false);
    VerifyOrReturnError(CHIP_NO_ERROR == kvs.Get(kWiFiCredentialsKeyName, key, sizeof(key), &keyLen), false);

    return ssidLen != 0;
#endif
    return false;
}

CHIP_ERROR ConfigurationManagerImpl::ClearWiFiNetworkInformations()
{
#if !CHIP_DISABLE_PLATFORM_KVS
    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    kvs.Delete(kWiFiSSIDKeyName);
    kvs.Delete(kWiFiCredentialsKeyName);
    return CHIP_NO_ERROR;
#endif
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::GetWiFiNetworkInformations(WiFiNetworkInfos & infos)
{
#if !CHIP_DISABLE_PLATFORM_KVS
    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    CHIP_ERROR err;

    err = kvs.Get(kWiFiCredentialsKeyName, infos.credentials, sizeof(infos.credentials), &infos.credentialsLen);
    if (CHIP_ERROR_KEY_NOT_FOUND == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    err = kvs.Get(kWiFiSSIDKeyName, infos.ssid, sizeof(infos.ssid), &infos.ssidLen);
    if (CHIP_ERROR_KEY_NOT_FOUND == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    return CHIP_NO_ERROR;
#endif
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ConfigurationManagerImpl::StoreWiFiNetworkInformations(WiFiNetworkInfos & infos)
{
#if !CHIP_DISABLE_PLATFORM_KVS
    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    ReturnErrorOnFailure(kvs.Put(kWiFiSSIDKeyName, infos.ssid, infos.ssidLen));
    ReturnErrorOnFailure(kvs.Put(kWiFiCredentialsKeyName, infos.credentials, infos.credentialsLen));
    return CHIP_NO_ERROR;
#endif
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace chip
