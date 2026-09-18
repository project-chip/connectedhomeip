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

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>

#include "esp_log.h"
#include "esp_wifi.h"

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
#include <lib/support/BytesToHex.h>

#include "esp_eap_client.h"
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

#include <limits>
#include <string>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
// A PDC network is stored in two places, with the same three blobs in each. The fail-safe backup
// holds the pre-change network and is removed on commit or revert. The committed set is the store
// of record across a reboot: esp_wifi's own NVS cannot hold a PDC network, because there is no
// passphrase and the EAP-TLS credentials only ever reach esp_eap_client as pointers into RAM.
struct PDCStorageKeys
{
    const char * ssid; // nullptr for the backup, which shares kWiFiSSIDKeyName with passphrase networks
    const char * networkIdentity;
    const char * clientIdentity;
    const char * clientIdentityKeypair;
};
constexpr PDCStorageKeys kPDCBackupKeys{ nullptr, "wifi-ni", "wifi-ci", "wifi-cik" };
constexpr PDCStorageKeys kPDCCommittedKeys{ "pdc-ssid", "pdc-ni", "pdc-ci", "pdc-cik" };

void DeletePDCNetwork(const PDCStorageKeys & keys)
{
    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    TEMPORARY_RETURN_IGNORED kvs.Delete(keys.networkIdentity);
    TEMPORARY_RETURN_IGNORED kvs.Delete(keys.clientIdentity);
    TEMPORARY_RETURN_IGNORED kvs.Delete(keys.clientIdentityKeypair);
    if (keys.ssid != nullptr)
    {
        TEMPORARY_RETURN_IGNORED kvs.Delete(keys.ssid);
    }
}

// Writes the identities and the key they attest to. The SSID, where this key set has one, is
// written last so that its presence marks the stored network as complete.
CHIP_ERROR StorePDCNetwork(const PDCStorageKeys & keys, const ESPWiFiDriver::WiFiNetwork & network,
                           const Crypto::P256Keypair & keypair)
{
    auto & kvs = PersistedStorage::KeyValueStoreMgr();
    ReturnErrorOnFailure(kvs.Put(keys.networkIdentity, network.networkIdentity, network.networkIdentityLen));
    ReturnErrorOnFailure(kvs.Put(keys.clientIdentity, network.clientIdentity, network.clientIdentityLen));

    Crypto::P256SerializedKeypair serializedKeypair;
    ReturnErrorOnFailure(keypair.Serialize(serializedKeypair));
    ReturnErrorOnFailure(kvs.Put(keys.clientIdentityKeypair, serializedKeypair.ConstBytes(), serializedKeypair.Length()));

    if (keys.ssid != nullptr)
    {
        ReturnErrorOnFailure(kvs.Put(keys.ssid, network.ssid, network.ssidLen));
    }
    return CHIP_NO_ERROR;
}

// Reads back what StorePDCNetwork() wrote, leaving network.ssid alone for a key set without one.
// Returns CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if there is no stored PDC network, which
// for the backup keys simply means the backed up network used a passphrase.
//
// Deserializing replaces the live keypair, so it goes last: everything else that can fail has been
// read and validated by then. If it fails anyway the driver is left with no usable Client
// Identity, and the next AddOrUpdateNetworkWithPDC generates a fresh one.
CHIP_ERROR LoadPDCNetwork(const PDCStorageKeys & keys, ESPWiFiDriver::WiFiNetwork & network, Crypto::P256Keypair & keypair)
{
    auto & kvs      = PersistedStorage::KeyValueStoreMgr();
    size_t valueLen = 0;

    const char * marker = (keys.ssid != nullptr) ? keys.ssid : keys.networkIdentity;
    CHIP_ERROR err      = kvs.Get(marker, nullptr, 0);
    VerifyOrReturnError(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, err);

    if (keys.ssid != nullptr)
    {
        ReturnErrorOnFailure(kvs.Get(keys.ssid, network.ssid, sizeof(network.ssid), &valueLen));
        VerifyOrReturnError(valueLen > 0 && CanCastTo<uint8_t>(valueLen), CHIP_ERROR_INTEGRITY_CHECK_FAILED);
        network.ssidLen = static_cast<uint8_t>(valueLen);
    }

    ReturnErrorOnFailure(kvs.Get(keys.networkIdentity, network.networkIdentity, sizeof(network.networkIdentity), &valueLen));
    VerifyOrReturnError(valueLen > 0 && CanCastTo<uint8_t>(valueLen), CHIP_ERROR_INTEGRITY_CHECK_FAILED);
    network.networkIdentityLen = static_cast<uint8_t>(valueLen);

    ReturnErrorOnFailure(kvs.Get(keys.clientIdentity, network.clientIdentity, sizeof(network.clientIdentity), &valueLen));
    VerifyOrReturnError(valueLen > 0 && CanCastTo<uint8_t>(valueLen), CHIP_ERROR_INTEGRITY_CHECK_FAILED);
    network.clientIdentityLen = static_cast<uint8_t>(valueLen);

    Crypto::P256SerializedKeypair serializedKeypair;
    ReturnErrorOnFailure(kvs.Get(keys.clientIdentityKeypair, serializedKeypair.Bytes(), serializedKeypair.Capacity(), &valueLen));
    ReturnErrorOnFailure(serializedKeypair.SetLength(valueLen));
    ReturnErrorOnFailure(keypair.Deserialize(serializedKeypair));
    return CHIP_NO_ERROR;
}

// Populates one of the identifier fields of Network with the key identifier of the corresponding
// identity. Leaves the optional absent if the stored identity can't be parsed.
void ExtractKeyIdentifier(Optional<Credentials::CertificateKeyIdStorage> & identifier, ByteSpan identity, const char * kind)
{
    CHIP_ERROR err = Credentials::ExtractIdentifierFromChipNetworkIdentity(identity, identifier.Emplace());
    if (err != CHIP_NO_ERROR)
    {
        identifier.ClearValue();
        ChipLogFailure(err, DeviceLayer, "Failed to extract %s Identifier", kind);
    }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
} // namespace

BitFlags<WiFiSecurityBitmap> ConvertSecurityType(wifi_auth_mode_t authMode)
{
    BitFlags<WiFiSecurityBitmap> securityType;
    switch (authMode)
    {
    case WIFI_AUTH_OPEN:
        securityType.Set(WiFiSecurity::kUnencrypted);
        break;
    case WIFI_AUTH_WEP:
        securityType.Set(WiFiSecurity::kWep);
        break;
    case WIFI_AUTH_WPA_PSK:
        securityType.Set(WiFiSecurity::kWpaPersonal);
        break;
    case WIFI_AUTH_WPA2_PSK:
        securityType.Set(WiFiSecurity::kWpa2Personal);
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        securityType.Set(WiFiSecurity::kWpa2Personal);
        securityType.Set(WiFiSecurity::kWpaPersonal);
        break;
    case WIFI_AUTH_WPA3_PSK:
        securityType.Set(WiFiSecurity::kWpa3Personal);
        break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
        securityType.Set(WiFiSecurity::kWpa3Personal);
        securityType.Set(WiFiSecurity::kWpa2Personal);
        break;
    default:
        break;
    }
    return securityType;
}

CHIP_ERROR GetConfiguredNetwork(Network & network)
{
    wifi_ap_record_t ap_info;
    esp_err_t err;
    err = esp_wifi_sta_get_ap_info(&ap_info);
    if (err != ESP_OK)
    {
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }
    static_assert(chip::DeviceLayer::Internal::kMaxWiFiSSIDLength <= UINT8_MAX, "SSID length might not fit in length");
    uint8_t length =
        static_cast<uint8_t>(strnlen(reinterpret_cast<const char *>(ap_info.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength));
    if (length > sizeof(network.networkID))
    {
        return CHIP_ERROR_INTERNAL;
    }
    memcpy(network.networkID, ap_info.ssid, length);
    network.networkIDLen = length;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
#ifdef CONFIG_ESP_WIFI_DEBUG_PRINT
    // CONFIG_ESP_WIFI_DEBUG_PRINT only controls whether the supplicant's wpa_printf() calls are
    // compiled in at all; the interesting ones (RSN IE contents, EAP state) are ESP_LOG_DEBUG and
    // stay filtered at runtime unless the tag is raised. All of the supplicant shares one tag.
    esp_log_level_set("wpa", ESP_LOG_DEBUG);
#endif // CONFIG_ESP_WIFI_DEBUG_PRINT

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    bool loadedPDCNetwork = false;
    {
        // A committed PDC network lives in the KVS rather than in esp_wifi, so it has to be loaded
        // and associated by hand; see ConnectWiFiNetworkWithPDC().
        WiFiNetwork network;
        CHIP_ERROR err = LoadPDCNetwork(kPDCCommittedKeys, network, mClientIdentityKeypair);
        if (err == CHIP_NO_ERROR)
        {
            mStagingNetwork  = network;
            loadedPDCNetwork = true;
            ChipLogProgress(DeviceLayer, "Loaded committed PDC network, SSID %.*s", static_cast<int>(network.ssidLen),
                            network.ssid);
        }
        else if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            // Half a PDC network is no use, and keeping it would shadow esp_wifi's config forever.
            ChipLogFailure(err, DeviceLayer, "Discarding unreadable committed PDC network");
            DeletePDCNetwork(kPDCCommittedKeys);
        }
    }
    if (!loadedPDCNetwork)
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    {
        wifi_config_t stationConfig;
        if (esp_wifi_get_config(WIFI_IF_STA, &stationConfig) == ESP_OK && stationConfig.sta.ssid[0] != 0)
        {
            uint8_t ssidLen = static_cast<uint8_t>(
                strnlen(reinterpret_cast<const char *>(stationConfig.sta.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength));
            memcpy(mStagingNetwork.ssid, stationConfig.sta.ssid, ssidLen);
            mStagingNetwork.ssidLen = ssidLen;

            uint8_t credentialsLen = static_cast<uint8_t>(
                strnlen(reinterpret_cast<const char *>(stationConfig.sta.password), DeviceLayer::Internal::kMaxWiFiKeyLength));

            memcpy(mStagingNetwork.credentials, stationConfig.sta.password, credentialsLen);
            mStagingNetwork.credentialsLen = credentialsLen;
        }
    }

    mpScanCallback         = nullptr;
    mpConnectCallback      = nullptr;
    mpStatusChangeCallback = networkStatusChangeCallback;

    // If a fail-safe backup exists the device was rebooted with the fail-safe armed, and the backup
    // takes precedence over anything loaded above: esp_wifi persists credential changes eagerly, so
    // the pre-change network has to be put back. RevertConfiguration() reconnects when it restores.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    bool backupExists =
        (PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, nullptr, 0) != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    ReturnErrorOnFailure(RevertConfiguration());

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    // Nothing else will bring a committed PDC network up: esp_wifi has no config for it, so
    // ConnectivityManagerImpl sees an unprovisioned station and leaves it alone.
    if (loadedPDCNetwork && !backupExists)
    {
        ReturnErrorOnFailure(ConnectWiFiNetworkWithPDC());
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    return CHIP_NO_ERROR;
}

void ESPWiFiDriver::Shutdown()
{
    mpStatusChangeCallback = nullptr;
}

CHIP_ERROR ESPWiFiDriver::CommitConfiguration()
{
    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiSSIDKeyName);
    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiCredentialsKeyName);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    DeletePDCNetwork(kPDCBackupKeys);

    // esp_wifi has already persisted a passphrase network by this point, but knows nothing about a
    // PDC one, so that is stored here instead. Committing anything else clears it, which covers
    // both replacing a PDC network with a passphrase one and removing it outright.
    if (mStagingNetwork.UsingPDC())
    {
        ReturnErrorOnFailure(StorePDCNetwork(kPDCCommittedKeys, mStagingNetwork, mClientIdentityKeypair));
    }
    else
    {
        DeletePDCNetwork(kPDCCommittedKeys);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESPWiFiDriver::RevertConfiguration()
{
    WiFiNetwork network;
    Network configuredNetwork;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    CHIP_ERROR error = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, network.ssid, sizeof(network.ssid), &ssidLen);
    VerifyOrReturnError(error != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_NO_ERROR);
    VerifyOrExit(CanCastTo<uint8_t>(ssidLen), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, network.credentials, sizeof(network.credentials),
                                                          &credentialsLen) == CHIP_NO_ERROR,
                 error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<uint8_t>(credentialsLen), error = CHIP_ERROR_INTERNAL);

    network.ssidLen        = static_cast<uint8_t>(ssidLen);
    network.credentialsLen = static_cast<uint8_t>(credentialsLen);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    {
        // Absence of the PDC keys just means the backed up network used a passphrase, which leaves
        // network.networkIdentityLen at zero.
        CHIP_ERROR pdcError = LoadPDCNetwork(kPDCBackupKeys, network, mClientIdentityKeypair);
        VerifyOrExit(pdcError == CHIP_NO_ERROR || pdcError == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, error = pdcError);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    mStagingNetwork = network;

    // Note this compares SSIDs only, so it cannot tell a PDC association from a passphrase one to
    // the same SSID, and will wrongly skip the reconnect needed to switch between them.
    if (GetConfiguredNetwork(configuredNetwork) == CHIP_NO_ERROR)
    {
        VerifyOrExit(!NetworkMatch(mStagingNetwork, ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)),
                     error = CHIP_NO_ERROR);
    }

    if (error == CHIP_NO_ERROR)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        if (mStagingNetwork.UsingPDC())
        {
            error = ConnectWiFiNetworkWithPDC();
        }
        else
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
        {
            // ConnectWiFiNetwork can work with empty mStagingNetwork (ssidLen = 0).
            error = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
                                       reinterpret_cast<const char *>(mStagingNetwork.credentials), mStagingNetwork.credentialsLen);
        }
    }

exit:

    // Remove the backup.
    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiSSIDKeyName);
    TEMPORARY_RETURN_IGNORED PersistedStorage::KeyValueStoreMgr().Delete(kWiFiCredentialsKeyName);
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    DeletePDCNetwork(kPDCBackupKeys);
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    return error;
}

bool ESPWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status ESPWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                         uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);
    VerifyOrReturnError(BackupConfiguration() == CHIP_NO_ERROR, Status::kUnknownError);

    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    // Adding a network with a passphrase replaces any PDC configuration for it.
    mStagingNetwork.networkIdentityLen = 0;
    mStagingNetwork.clientIdentityLen  = 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    return Status::kSuccess;
}

Status ESPWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    VerifyOrReturnError(BackupConfiguration() == CHIP_NO_ERROR, Status::kUnknownError);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    mStagingNetwork.networkIdentityLen = 0;
    mStagingNetwork.clientIdentityLen  = 0;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    return Status::kSuccess;
}

Status ESPWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);

    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR ESPWiFiDriver::DisableStation()
{
    // Clear the WiFi configurations and add the newly provided WiFi configurations.
    if (chip::DeviceLayer::Internal::ESP32Utils::IsStationProvisioned())
    {
        CHIP_ERROR error = chip::DeviceLayer::Internal::ESP32Utils::ClearWiFiStationProvision();
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ClearWiFiStationProvision failed: %" CHIP_ERROR_FORMAT, error.Format());
            return error;
        }
    }

    return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);
}

CHIP_ERROR ESPWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    ReturnErrorOnFailure(DisableStation());

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    // A previous association may have left the EAP client configured; a passphrase network must
    // not inherit it.
    ReleaseEapTlsCredentials();

    // Undo what ConnectWiFiNetworkWithPDC() may have done: passphrase networks keep relying on
    // esp_wifi's own persistence, so that Init() can read them back with esp_wifi_get_config().
    ReturnErrorOnFailure(ESP32Utils::MapError(esp_wifi_set_storage(WIFI_STORAGE_FLASH)));
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    wifi_config_t wifiConfig;

    // Set the wifi configuration
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.sta.ssid, ssid, std::min(ssidLen, static_cast<uint8_t>(sizeof(wifiConfig.sta.ssid))));
    memcpy(wifiConfig.sta.password, key, std::min(keyLen, static_cast<uint8_t>(sizeof(wifiConfig.sta.password))));
#ifdef CONFIG_WIFI_SCAN_ALL_CHANNEL_MODE
    wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
#endif // CONFIG_WIFI_SCAN_ALL_CHANNEL_MODE

    // Configure the ESP WiFi interface.
    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", esp_err_to_name(err));
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }

    return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
}

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
CHIP_ERROR ESPWiFiDriver::DisconnectFromNetwork()
{
    if (chip::DeviceLayer::Internal::ESP32Utils::IsStationProvisioned())
    {
        // Attaching to an empty network will disconnect the network.
        ReturnErrorOnFailure(ConnectWiFiNetwork(nullptr, 0, nullptr, 0));
    }
    return CHIP_NO_ERROR;
}
#endif

void ESPWiFiDriver::OnConnectWiFiNetwork()
{
    DeviceLayer::SystemLayer().CancelTimer(OnConnectWiFiNetworkFailed, NULL);
    if (mpConnectCallback)
    {
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void ESPWiFiDriver::OnConnectWiFiNetworkFailed()
{
    if (mpConnectCallback)
    {
        // The reason code is the only thing distinguishing, say, a rejected client certificate
        // from an AP that never matched the auth mode threshold, so keep it in the log.
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network, reason %u", mLastDisconnectedReason);

        Status status = Status::kOtherConnectionFailure;
        switch (mLastDisconnectedReason)
        {
        case WIFI_REASON_AUTH_FAIL:
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_MIC_FAILURE:
        case WIFI_REASON_CONNECTION_FAIL:
        case WIFI_REASON_802_1X_AUTH_FAILED:
            status = Status::kAuthFailure;
            break;
        case WIFI_REASON_NO_AP_FOUND:
            status = Status::kNetworkNotFound;
            break;
        default:
            break;
        }
        mpConnectCallback->OnResult(status, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void ESPWiFiDriver::OnConnectWiFiNetworkFailed(chip::System::Layer * aLayer, void * aAppState)
{
    CHIP_ERROR error = chip::DeviceLayer::Internal::ESP32Utils::ClearWiFiStationProvision();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ClearWiFiStationProvision failed: %" CHIP_ERROR_FORMAT, error.Format());
    }
    ESPWiFiDriver::GetInstance().OnConnectWiFiNetworkFailed();
}

void ESPWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kSuccess;
    Network configuredNetwork;
    const uint32_t secToMiliSec = 1000;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    const bool usingPDC = mStagingNetwork.UsingPDC();
#else
    constexpr bool usingPDC = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(BackupConfiguration() == CHIP_NO_ERROR, networkingStatus = Status::kUnknownError);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "ESP NetworkCommissioningDelegate: SSID: %.*s%s", static_cast<int>(networkId.size()),
                    networkId.data(), usingPDC ? " (PDC)" : "");

    // The AP record only carries the SSID, which says nothing about whether the current
    // association is the EAP-TLS one a PDC network calls for. Always re-associate for those.
    if (!usingPDC && CHIP_NO_ERROR == GetConfiguredNetwork(configuredNetwork))
    {
        if (NetworkMatch(mStagingNetwork, ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)))
        {
            if (callback)
            {
                callback->OnResult(Status::kSuccess, CharSpan(), 0);
            }
            return;
        }
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    if (usingPDC)
    {
        err = ConnectWiFiNetworkWithPDC();
    }
    else
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    {
        err = ConnectWiFiNetwork(reinterpret_cast<const char *>(mStagingNetwork.ssid), mStagingNetwork.ssidLen,
                                 reinterpret_cast<const char *>(mStagingNetwork.credentials), mStagingNetwork.credentialsLen);
    }
    SuccessOrExit(err);

    err = DeviceLayer::SystemLayer().StartTimer(
        static_cast<System::Clock::Timeout>(kWiFiConnectNetworkTimeoutSeconds * secToMiliSec), OnConnectWiFiNetworkFailed, NULL);
    mpConnectCallback = callback;

exit:
    if (err != CHIP_NO_ERROR)
    {
        networkingStatus = Status::kUnknownError;
    }
    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network: %" CHIP_ERROR_FORMAT, err.Format());
        mpConnectCallback = nullptr;
        if (callback)
        {
            callback->OnResult(networkingStatus, CharSpan(), 0);
        }
    }
}

CHIP_ERROR ESPWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    esp_err_t err = ESP_OK;
    if (!ssid.empty())
    {
        wifi_scan_config_t scan_config = { 0 };
        memset(WiFiSSIDStr, 0, sizeof(WiFiSSIDStr));
        memcpy(WiFiSSIDStr, ssid.data(), ssid.size());
        scan_config.ssid = WiFiSSIDStr;
        err              = esp_wifi_scan_start(&scan_config, false);
    }
    else
    {
        err = esp_wifi_scan_start(nullptr, false);
    }
    if (err != ESP_OK)
    {
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }
    return CHIP_NO_ERROR;
}

void ESPWiFiDriver::OnScanWiFiNetworkDone()
{
    if (!mpScanCallback)
    {
        ChipLogProgress(DeviceLayer, "No scan callback");
        return;
    }
    uint16_t ap_number;
    esp_wifi_scan_get_ap_num(&ap_number);
    if (!ap_number)
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), nullptr);
        mpScanCallback = nullptr;
        return;
    }

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
    if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_number]() {
            ESPScanResponseIterator iter(ap_number);
            if (GetInstance().mpScanCallback)
            {
                GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
                GetInstance().mpScanCallback = nullptr;
            }
            else
            {
                ChipLogError(DeviceLayer, "can't find the ScanCallback function");
            }
            iter.Release();
        }))
    {
    }
    else
    {
        ChipLogError(DeviceLayer, "can't schedule the scan result processing");
        mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        mpScanCallback = nullptr;
    }
#else
    // Since this is the dynamic memory allocation, restrict it to a configured limit
    ap_number = std::min(static_cast<uint16_t>(CHIP_DEVICE_CONFIG_MAX_SCAN_NETWORKS_RESULTS), ap_number);

    std::unique_ptr<wifi_ap_record_t[]> ap_buffer_ptr(new wifi_ap_record_t[ap_number]);
    if (ap_buffer_ptr == NULL)
    {
        ChipLogError(DeviceLayer, "can't malloc memory for ap_list_buffer");
        mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        mpScanCallback = nullptr;
        return;
    }
    wifi_ap_record_t * ap_list_buffer = ap_buffer_ptr.get();
    if (esp_wifi_scan_get_ap_records(&ap_number, ap_list_buffer) == ESP_OK)
    {
        if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([ap_number, ap_list_buffer]() {
                std::unique_ptr<wifi_ap_record_t[]> auto_free(ap_list_buffer);
                ESPScanResponseIterator iter(ap_number, ap_list_buffer);
                if (GetInstance().mpScanCallback)
                {
                    GetInstance().mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
                    GetInstance().mpScanCallback = nullptr;
                }
                else
                {
                    ChipLogError(DeviceLayer, "can't find the ScanCallback function");
                }
            }))
        {
            ap_buffer_ptr.release();
        }
        else
        {
            ChipLogError(DeviceLayer, "can't schedule the scan result processing");
            mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            mpScanCallback = nullptr;
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "can't get ap_records ");
        mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        mpScanCallback = nullptr;
    }
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 1, 3)
}

void ESPWiFiDriver::OnNetworkStatusChange()
{
    Network configuredNetwork;
    bool staEnabled = false, staConnected = false;
    VerifyOrReturn(ESP32Utils::IsStationEnabled(staEnabled) == CHIP_NO_ERROR);
    VerifyOrReturn(staEnabled && mpStatusChangeCallback != nullptr);
    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }
    VerifyOrReturn(ESP32Utils::IsStationConnected(staConnected) == CHIP_NO_ERROR);
    if (staConnected)
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)), NullOptional);
        return;
    }

    // The disconnect reason for networking status changes is allowed to have
    // manufacturer-specific values, which is why it's an int32_t, even though
    // we just store a uint16_t value in it.
    int32_t lastDisconnectReason = GetLastDisconnectReason();
    mpStatusChangeCallback->OnNetworkingStatusChange(
        Status::kUnknownError, MakeOptional(ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)),
        MakeOptional(lastDisconnectReason));
}

void ESPWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    if (callback != nullptr)
    {
        mpScanCallback = callback;
        if (StartScanWiFiNetworks(ssid) != CHIP_NO_ERROR)
        {
            mpScanCallback = nullptr;
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
}

uint32_t ESPWiFiDriver::GetSupportedWiFiBandsMask() const
{
    uint32_t bands = static_cast<uint32_t>(1UL << chip::to_underlying(WiFiBandEnum::k2g4));
    return bands;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
CHIP_ERROR ESPWiFiDriver::AddOrUpdateNetworkWithPDC(ByteSpan ssid, ByteSpan networkIdentity,
                                                    Optional<uint8_t> clientIdentityNetworkIndex, Status & outStatus,
                                                    MutableCharSpan & outDebugText, MutableByteSpan & outClientIdentity,
                                                    uint8_t & outNetworkIndex)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    outStatus      = Status::kUnknownError;
    outDebugText.reduce_size(0);

    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), CHIP_NO_ERROR,
                        outStatus = Status::kBoundsExceeded);
    VerifyOrReturnError(!ssid.empty() && ssid.size() <= sizeof(mStagingNetwork.ssid), CHIP_NO_ERROR,
                        outStatus = Status::kOutOfRange);
    VerifyOrReturnError(!networkIdentity.empty() && networkIdentity.size() <= sizeof(mStagingNetwork.networkIdentity),
                        CHIP_NO_ERROR, outStatus = Status::kOutOfRange);
    // Only a single network is supported, so the only re-usable Client Identity is the one at index 0.
    VerifyOrReturnError(!clientIdentityNetworkIndex.HasValue() ||
                            (clientIdentityNetworkIndex.Value() == 0 && mStagingNetwork.UsingPDC()),
                        CHIP_NO_ERROR, outStatus = Status::kOutOfRange);
    VerifyOrReturnError(BackupConfiguration() == CHIP_NO_ERROR, CHIP_NO_ERROR, outStatus = Status::kUnknownError);

    // Generate a fresh Client Identity unless an existing one is being re-used. Because only one
    // network is supported, re-use simply means leaving the stored identity and keypair alone.
    if (!clientIdentityNetworkIndex.HasValue())
    {
        // P256Keypair is not copyable, so this generates in place; on failure the (now unusable)
        // Client Identity is discarded rather than left half-updated. Note that re-initializing a
        // keypair without an intervening Clear() is undefined; the PSA backend rejects it outright.
        mStagingNetwork.clientIdentityLen = 0;
        mClientIdentityKeypair.Clear();
        SuccessOrExit(err = mClientIdentityKeypair.Initialize(Crypto::ECPKeyTarget::ECDSA));

        MutableByteSpan clientIdentity(mStagingNetwork.clientIdentity);
        SuccessOrExit(err = Credentials::NewChipNetworkIdentity(mClientIdentityKeypair, clientIdentity));
        mStagingNetwork.clientIdentityLen = static_cast<uint8_t>(clientIdentity.size());
    }

    SuccessOrExit(err = CopySpanToMutableSpan(ByteSpan(mStagingNetwork.clientIdentity, mStagingNetwork.clientIdentityLen),
                                              outClientIdentity));

    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<uint8_t>(ssid.size());
    memcpy(mStagingNetwork.networkIdentity, networkIdentity.data(), networkIdentity.size());
    mStagingNetwork.networkIdentityLen = static_cast<uint8_t>(networkIdentity.size());
    mStagingNetwork.credentialsLen     = 0;

    outNetworkIndex = 0;
    outStatus       = Status::kSuccess;

exit:
    return err;
}

CHIP_ERROR ESPWiFiDriver::GetNetworkIdentity(uint8_t networkIndex, MutableByteSpan & outNetworkIdentity)
{
    VerifyOrReturnError(networkIndex == 0 && mStagingNetwork.UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(mStagingNetwork.networkIdentity, mStagingNetwork.networkIdentityLen), outNetworkIdentity);
}

CHIP_ERROR ESPWiFiDriver::GetClientIdentity(uint8_t networkIndex, MutableByteSpan & outClientIdentity)
{
    VerifyOrReturnError(networkIndex == 0 && mStagingNetwork.UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(mStagingNetwork.clientIdentity, mStagingNetwork.clientIdentityLen), outClientIdentity);
}

CHIP_ERROR ESPWiFiDriver::SignWithClientIdentity(uint8_t networkIndex, const ByteSpan & message,
                                                 Crypto::P256ECDSASignature & outSignature)
{
    VerifyOrReturnError(networkIndex == 0 && mStagingNetwork.UsingPDC(), CHIP_ERROR_INVALID_ARGUMENT);
    return mClientIdentityKeypair.ECDSA_sign_msg(message.data(), message.size(), outSignature);
}

CHIP_ERROR ESPWiFiDriver::ConnectWiFiNetworkWithPDC()
{
    ReturnErrorOnFailure(DisableStation());
    ReleaseEapTlsCredentials();

    // esp_wifi's NVS copy of the station config can't be used for a PDC network: there is no
    // passphrase to hold, and a persisted SSID makes ESP32Utils::IsStationProvisioned() true at the
    // next boot, so ConnectivityManagerImpl drives a doomed association before this driver has
    // loaded the identities. Keep the config in RAM; kPDCCommittedKeys hold it across a reboot.
    ReturnErrorOnFailure(ESP32Utils::MapError(esp_wifi_set_storage(WIFI_STORAGE_RAM)));

    wifi_config_t wifiConfig;
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.sta.ssid, mStagingNetwork.ssid,
           std::min(mStagingNetwork.ssidLen, static_cast<uint8_t>(sizeof(wifiConfig.sta.ssid))));

    // PDC uses similar settings to WPA3-Enterprise: WPA-EAP-SHA256 (00-0f-ac-5) with MFP, however
    // PDC allows MFPC=1 MFPR=0 so long as PDC STAs actually negotiate MFP; WPA3-Enterprise requires
    // MFPR=1. Tis maps to WIFI_AUTH_WPA2_WPA3_ENTERPRISE for the purposes of the auth mode
    // threshold, which correct rejects APs advertising only WPA-EAP (00-0f-ac-1). Note that the
    // "strength" comparison applied by the threshold logic is not a straight numeric comparison,
    // (see roaming_app_authmode_strength_rank() in ESP-IDF's roaming_app.c).
    wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_WPA3_ENTERPRISE;
    wifiConfig.sta.pmf_cfg.required   = true;
#ifdef CONFIG_WIFI_SCAN_ALL_CHANNEL_MODE
    wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
#endif // CONFIG_WIFI_SCAN_ALL_CHANNEL_MODE

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", esp_err_to_name(err));
        return chip::DeviceLayer::Internal::ESP32Utils::MapError(err);
    }

    CHIP_ERROR error = InstallEapTlsCredentials();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to install EAP-TLS credentials: %" CHIP_ERROR_FORMAT, error.Format());
        ReleaseEapTlsCredentials();
        return error;
    }

    return ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
}

CHIP_ERROR ESPWiFiDriver::InstallEapTlsCredentials()
{
    using namespace chip::Credentials;

    // The three DER encodings share one allocation, laid out in this order. esp_eap_client only
    // stores the pointers, so the buffer has to stay alive for as long as the connection does.
    constexpr size_t kNetworkIdentityOffset = 0;
    constexpr size_t kClientIdentityOffset  = kNetworkIdentityOffset + kMaxDERCertLength;
    constexpr size_t kClientKeyOffset       = kClientIdentityOffset + kMaxDERCertLength;
    constexpr size_t kCredentialsSize       = kClientKeyOffset + kP256ECPrivateKeyDERLength;

    VerifyOrReturnError(mEapTlsCredentials.Alloc(kCredentialsSize), CHIP_ERROR_NO_MEMORY);
    uint8_t * const credentials = mEapTlsCredentials.Get();

    ByteSpan networkIdentity(mStagingNetwork.networkIdentity, mStagingNetwork.networkIdentityLen);

    MutableByteSpan networkIdentityDER(credentials + kNetworkIdentityOffset, kMaxDERCertLength);
    ReturnErrorOnFailure(ConvertChipCertToX509Cert(networkIdentity, networkIdentityDER));

    MutableByteSpan clientIdentityDER(credentials + kClientIdentityOffset, kMaxDERCertLength);
    ReturnErrorOnFailure(
        ConvertChipCertToX509Cert(ByteSpan(mStagingNetwork.clientIdentity, mStagingNetwork.clientIdentityLen), clientIdentityDER));

    MutableByteSpan clientKeyDER(credentials + kClientKeyOffset, kP256ECPrivateKeyDERLength);
    {
        Crypto::P256SerializedKeypair serializedKeypair;
        ReturnErrorOnFailure(mClientIdentityKeypair.Serialize(serializedKeypair));
        ReturnErrorOnFailure(ConvertECDSAKeypairRawToDER(serializedKeypair, clientKeyDER));
    }

    // The EAP identity is the NAI derived from the Network Identity's key identifier, as
    // described in section 11.24 of the Matter specification.
    {
        CertificateKeyIdStorage keyId;
        ReturnErrorOnFailure(ExtractIdentifierFromChipNetworkIdentity(networkIdentity, keyId));

        static constexpr char kNAIDomain[]  = ".pdc.csa-iot.org";
        static constexpr size_t keyIdHexLen = 2 * kKeyIdentifierLength;
        char identity[1 + keyIdHexLen + sizeof(kNAIDomain)]; // sizeof(kNAIDomain) includes the null terminator
        identity[0] = '@';
        ReturnErrorOnFailure(Encoding::BytesToUppercaseHexBuffer(keyId.data(), keyId.size(), &identity[1], keyIdHexLen));
        memcpy(&identity[1 + keyIdHexLen], kNAIDomain, sizeof(kNAIDomain));

        ReturnErrorOnFailure(ESP32Utils::MapError(
            esp_eap_client_set_identity(reinterpret_cast<const uint8_t *>(identity), static_cast<int>(sizeof(identity) - 1))));
    }

    // Install the Network Identity as the CA certificate. The access point directly presents this
    // certificate, i.e. the certificate chain has length 1.
    ReturnErrorOnFailure(
        ESP32Utils::MapError(esp_eap_client_set_ca_cert(networkIdentityDER.data(), static_cast<int>(networkIdentityDER.size()))));
    ReturnErrorOnFailure(ESP32Utils::MapError(
        esp_eap_client_set_certificate_and_key(clientIdentityDER.data(), static_cast<int>(clientIdentityDER.size()),
                                               clientKeyDER.data(), static_cast<int>(clientKeyDER.size()), nullptr, 0)));
    ReturnErrorOnFailure(ESP32Utils::MapError(esp_eap_client_set_eap_methods(ESP_EAP_TYPE_TLS)));

    return ESP32Utils::MapError(esp_wifi_sta_enterprise_enable());
}

void ESPWiFiDriver::ReleaseEapTlsCredentials()
{
    VerifyOrReturn(mEapTlsCredentials); // nothing was ever handed to the EAP client

    // Disabling is a no-op when the EAP client is already disabled, and in either case it drops
    // the certificate and identity pointers we handed over, making it safe to free the buffer.
    esp_err_t err = esp_wifi_sta_enterprise_disable();
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_sta_enterprise_disable() failed: %s", esp_err_to_name(err));
    }
    mEapTlsCredentials.Free();
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

CHIP_ERROR ESPWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    VerifyOrReturnError(event->Type == DeviceEventType::kESPSystemEvent && event->Platform.ESPSystemEvent.Base == WIFI_EVENT &&
                            event->Platform.ESPSystemEvent.Id == WIFI_EVENT_STA_DISCONNECTED,
                        CHIP_ERROR_INVALID_ARGUMENT);
    mLastDisconnectedReason = event->Platform.ESPSystemEvent.Data.WiFiStaDisconnected.reason;
    return CHIP_NO_ERROR;
}

uint16_t ESPWiFiDriver::GetLastDisconnectReason()
{
    return mLastDisconnectedReason;
}

size_t ESPWiFiDriver::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool ESPWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = mDriver->mStagingNetwork.ssidLen;
    item.connected    = false;
    mExhausted        = true;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    // NetworkInfoStruct reports the key identifiers of the two identities rather than the
    // identities themselves, which are retrieved with QueryIdentity. Both are null for a network
    // configured with a passphrase.
    const auto & network = mDriver->mStagingNetwork;
    if (network.UsingPDC())
    {
        ExtractKeyIdentifier(item.networkIdentifier, ByteSpan(network.networkIdentity, network.networkIdentityLen), "Network");
        ExtractKeyIdentifier(item.clientIdentifier, ByteSpan(network.clientIdentity, network.clientIdentityLen), "Client");
    }
    else
    {
        item.networkIdentifier.ClearValue();
        item.clientIdentifier.ClearValue();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC

    Network configuredNetwork;
    CHIP_ERROR err = GetConfiguredNetwork(configuredNetwork);
    if (err == CHIP_NO_ERROR)
    {
        bool isConnected = false;
        err              = ESP32Utils::IsStationConnected(isConnected);
        if (err == CHIP_NO_ERROR && isConnected && configuredNetwork.networkIDLen == item.networkIDLen &&
            memcmp(configuredNetwork.networkID, item.networkID, item.networkIDLen) == 0)
        {
            item.connected = true;
        }
    }
    return true;
}

CHIP_ERROR ESPWiFiDriver::BackupConfiguration()
{
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, nullptr, 0);
    if (err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    if (mStagingNetwork.UsingPDC())
    {
        ReturnErrorOnFailure(StorePDCNetwork(kPDCBackupKeys, mStagingNetwork, mClientIdentityKeypair));
    }
    else
    {
        // Nothing should have left these behind, but a stale set would be restored over a
        // passphrase network and turn it back into a PDC one.
        DeletePDCNetwork(kPDCBackupKeys);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_PDC
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    return CHIP_NO_ERROR;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
