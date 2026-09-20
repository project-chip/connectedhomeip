/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <platform/senscomm/scm1612s/SCM1612SConfig.h>
#include <platform/senscomm/scm1612s/NetworkCommissioningWiFiDriver.h>


#include "wise_event_loop.h"
#include "wise_wifi_types.h"
#include "wise_err.h"
#include "scm_wifi.h"
#include "wise_wifi.h"

#define SECURITY_OPEN 0
#define SECURITY_TKIP 2
#define SECURITY_CCMP 3
#define SECURITY_CCMP_256 4
#define SECURITY_SAE  6

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];
constexpr uint16_t kMaxCachedWiFiScanAPs = kMaxWiFiScanAPs;
static uint8_t totalScanRounds;

struct CachedWiFiScanResults
{
    std::array<scm_wifi_ap_info, kMaxCachedWiFiScanAPs> mResults = {};
    uint16_t mCount                                        = 0;
    decltype(System::SystemClock().GetMonotonicTimestamp()) mTimestamp = System::Clock::kZero;
    bool mValid                                            = false;
};

CachedWiFiScanResults gCachedWiFiScanResults;

bool HasValidCachedWiFiScanResults()
{
    if (!gCachedWiFiScanResults.mValid || gCachedWiFiScanResults.mTimestamp == System::Clock::kZero)
    {
        return false;
    }

    const auto now = System::SystemClock().GetMonotonicTimestamp();
    /* Set the valid time to 1010s */
    return now < (gCachedWiFiScanResults.mTimestamp + System::Clock::Seconds32(kWiFiScanNetworksTimeOutSeconds + 1000));
}

void InvalidateCachedWiFiScanResults()
{
    gCachedWiFiScanResults.mCount     = 0;
    gCachedWiFiScanResults.mTimestamp = System::Clock::kZero;
    gCachedWiFiScanResults.mValid     = false;
}

size_t GetWiFiSsidLength(const scm_wifi_ap_info & apInfo)
{
    return strnlen(apInfo.ssid, sizeof(apInfo.ssid));
}

// Deduplicate scan entries by BSSID so the cache keeps distinct APs even when SSIDs match.
bool IsSameAccessPoint(const scm_wifi_ap_info & lhs, const scm_wifi_ap_info & rhs)
{
    return memcmp(lhs.bssid, rhs.bssid, sizeof(lhs.bssid)) == 0;
}

void MergeWiFiScanResults(const scm_wifi_ap_info * apListBuffer, uint16_t count)
{
    VerifyOrReturn(apListBuffer != nullptr || count == 0);

    const uint16_t maxCount = static_cast<uint16_t>(gCachedWiFiScanResults.mResults.size());

    for (uint16_t index = 0; index < count; ++index)
    {
        const scm_wifi_ap_info & candidate = apListBuffer[index];
        bool updatedExisting               = false;

        for (uint16_t cachedIndex = 0; cachedIndex < gCachedWiFiScanResults.mCount; ++cachedIndex)
        {
            if (IsSameAccessPoint(gCachedWiFiScanResults.mResults[cachedIndex], candidate))
            {
                gCachedWiFiScanResults.mResults[cachedIndex] = candidate;
                updatedExisting                              = true;
                break;
            }
        }

        if (updatedExisting)
        {
            continue;
        }

        if (gCachedWiFiScanResults.mCount < maxCount)
        {
            gCachedWiFiScanResults.mResults[gCachedWiFiScanResults.mCount++] = candidate;
            continue;
        }

        auto weakestIt = std::min_element(gCachedWiFiScanResults.mResults.begin(),
                                          gCachedWiFiScanResults.mResults.begin() + gCachedWiFiScanResults.mCount,
                                          [](const scm_wifi_ap_info & lhs, const scm_wifi_ap_info & rhs) {
                                              return lhs.rssi < rhs.rssi;
                                          });
        if (weakestIt != gCachedWiFiScanResults.mResults.begin() + gCachedWiFiScanResults.mCount &&
            candidate.rssi > weakestIt->rssi)
        {
            *weakestIt = candidate;
        }
    }

    gCachedWiFiScanResults.mTimestamp = System::SystemClock().GetMonotonicTimestamp();
    gCachedWiFiScanResults.mValid     = gCachedWiFiScanResults.mCount > 0;
}

void LogCachedWiFiScanResults()
{
    ChipLogError(DeviceLayer, "Cached WiFi scan results: round=%u valid=%d count=%u", totalScanRounds, gCachedWiFiScanResults.mValid,
                    gCachedWiFiScanResults.mCount);

    for (uint16_t index = 0; index < gCachedWiFiScanResults.mCount; ++index)
    {
        const scm_wifi_ap_info & apInfo = gCachedWiFiScanResults.mResults[index];
        ChipLogProgress(DeviceLayer,
                        "Cached AP[%u]: ssid=%s channel=%u auth=%d rssi=%d bssid=%02x:%02x:%02x:%02x:%02x:%02x", index,
                        apInfo.ssid, apInfo.channel, apInfo.auth, apInfo.rssi, apInfo.bssid[0], apInfo.bssid[1],
                        apInfo.bssid[2], apInfo.bssid[3], apInfo.bssid[4], apInfo.bssid[5]);
    }
}

const scm_wifi_ap_info * FindCachedScanResult(const char * ssid, uint8_t ssidLen)
{
    if (!HasValidCachedWiFiScanResults())
    {
        return nullptr;
    }

    const scm_wifi_ap_info * bestMatch = nullptr;

    for (uint16_t index = 0; index < gCachedWiFiScanResults.mCount; ++index)
    {
        const scm_wifi_ap_info & apInfo = gCachedWiFiScanResults.mResults[index];
        const size_t cachedSsidLen      = GetWiFiSsidLength(apInfo);

        if (cachedSsidLen != ssidLen)
        {
            continue;
        }

        if (memcmp(apInfo.ssid, ssid, ssidLen) == 0)
        {
            if (bestMatch == nullptr || apInfo.rssi > bestMatch->rssi)
            {
                bestMatch = &apInfo;
            }
        }
    }

    return bestMatch;
}
} // namespace

#if 0
bool IsStationConnect(void)
{
    int ret;
    scm_wifi_status connect;
    return (scm_wifi_get_options(SCM_WIFI_STA_GET_CONNECT, &connect) == WISE_OK && connect.status == SCM_WIFI_CONNECTED);
}

CHIP_ERROR GetConfiguredNetwork(Network & network)
{
    wifi_config_t config;

    if (!IsStationConnect() || scm_wifi_get_config(WIFI_IF_STA, &config) == WISE_FAIL)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint8_t length = static_cast<uint8_t>(strnlen(reinterpret_cast<const char *>(config.sta.ssid), kMaxWiFiSSIDLength));
    if (length > sizeof(network.networkID))
    {
        return CHIP_ERROR_INTERNAL;
    }
    memcpy(network.networkID, config.sta.ssid, length);
    network.networkIDLen = length;

    return CHIP_NO_ERROR;
}
#endif

CHIP_ERROR WiseWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;
    size_t outLen         = 0;
    mpScanCallback        = nullptr;
    mpConnectCallback     = nullptr;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::Init");


#if 1
    // Note: Used a tricky method to avoid a problem
    // If we directly use ConnectWiFiNetwork to Connect WiFi for HYD none matter BLE WiFi Provision
    // A quirky problem happens: scm_wifi_sta_set_config does not store key values and get trapped in a cycle:
    // SYSTEM_EVENT_STA_NO_NETWORK ==> Connect(failed) ==>SYSTEM_EVENT_STA_NO_NETWORK
    if (networkStatusChangeCallback == NULL) 
    {
        // mSavedNetwork.
        err = SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiSSID, mTmpNetwork.ssid);
        VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);
        err = SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiPSK, mTmpNetwork.credentials);
        VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);
        uint8_t auth = mTmpNetwork.auth_mode;
        err = SCM1612SConfig::WriteConfigValueBin(SCM1612SConfig::kConfigKey_WiFiSEC, &auth, sizeof(auth));
    }
#endif

    // If reading fails, wifi is not provisioned, no need to go further.
    err = SCM1612SConfig::ReadConfigValueStr(SCM1612SConfig::kConfigKey_WiFiSSID, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid),
                                           ssidLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    err = SCM1612SConfig::ReadConfigValueBin(SCM1612SConfig::kConfigKey_WiFiSEC, &mSavedNetwork.auth_mode,
                                           sizeof(mSavedNetwork.auth_mode), outLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);
    err = SCM1612SConfig::ReadConfigValueStr(SCM1612SConfig::kConfigKey_WiFiPSK, mSavedNetwork.credentials,
                                           sizeof(mSavedNetwork.credentials), credentialsLen);
    // VerifyOrReturnError(err == CHIP_NO_ERROR, CHIP_NO_ERROR);

    mSavedNetwork.credentialsLen = credentialsLen;
    mSavedNetwork.ssidLen        = ssidLen;
    mStagingNetwork              = mSavedNetwork;

    if (IsInitialScanBlockingConnections())
    {
        EnableInitConnect = true;
        ChipLogProgress(NetworkProvisioning, "Deferring WiFi connect until initial scan completes");
        return CHIP_NO_ERROR;
    }

    ConnectWiFiNetwork(mSavedNetwork.ssid, ssidLen, mSavedNetwork.credentials, credentialsLen);
    return err;
}

void WiseWiFiDriver::ConfigureInitialScan(uint8_t scanRounds)
{
    EnableInitialScan     = (scanRounds > 0);
    InitialScanTriggered  = false;
    InitialScanInProgress = false;
    InitialScanCnt        = scanRounds;
    totalScanRounds = scanRounds;
}

bool WiseWiFiDriver::IsInitialScanBlockingConnections() const
{
    return EnableInitialScan;
}

void WiseWiFiDriver::ClearInitialScanState()
{
    EnableInitialScan     = false;
    InitialScanTriggered  = true;
    InitialScanInProgress = false;
    InitialScanCnt        = 0;
}

bool WiseWiFiDriver::StartNextInitialScanRound()
{
    VerifyOrReturnValue(EnableInitialScan && InitialScanCnt > 0, false);

    wifi_scan_config_t scanConfig = {};

    // ChipLogError(NetworkProvisioning, "Start initial WiFi scan, rounds left after this: %u", InitialScanCnt);

    scanConfig.scan_type            = WIFI_SCAN_TYPE_ACTIVE;
    scanConfig.scan_time.active.min = 100;
    scanConfig.scan_time.active.max = 130;

    if (wise_wifi_scan_start(&scanConfig, true, WIFI_IF_STA) != WISE_OK)
    {
        ChipLogError(NetworkProvisioning, "Initial WiFi scan start failed");
        ClearInitialScanState();
        return false;
    }

    InitialScanInProgress = true;
    return true;
}

bool WiseWiFiDriver::StartInitialScanOnStationStart()
{
    if (!EnableInitialScan || InitialScanTriggered)
    {
        return false;
    }

    InitialScanTriggered = true;
    return StartNextInitialScanRound();
}

CHIP_ERROR WiseWiFiDriver::CommitConfiguration()
{
    CHIP_ERROR err;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::CommitConfiguration");

    ReturnErrorOnFailure(SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiSSID, mStagingNetwork.ssid));
    ReturnErrorOnFailure(SCM1612SConfig::WriteConfigValueBin(SCM1612SConfig::kConfigKey_WiFiSEC, &mStagingNetwork.auth_mode,
                                                        sizeof(mStagingNetwork.auth_mode)));
    ReturnErrorOnFailure(SCM1612SConfig::WriteConfigValueStr(SCM1612SConfig::kConfigKey_WiFiPSK, mStagingNetwork.credentials));

    mSavedNetwork = mStagingNetwork;

    return CHIP_NO_ERROR;
}

void WiseWiFiDriver::UpdateWiFiAuthmode()
{
    wifi_config_t config;
    wifi_auth_mode_t authmode = WIFI_AUTH_OPEN;
    scm_wifi_get_config(WIFI_IF_STA, &config);

    switch (config.sta.proto)
    {
        case WIFI_PROTO_WPA:
            authmode = WIFI_AUTH_WPA_PSK;
            break;
        case WIFI_PROTO_WPA2:
            switch (config.sta.alg)
            {
                case SECURITY_CCMP:
                    authmode = WIFI_AUTH_WPA2_PSK;
                    break;
                case SECURITY_SAE:
                    authmode = WIFI_AUTH_WPA3_SAE;
                    break;
                default:
                    authmode = WIFI_AUTH_OPEN;
                    break;
            }
            break;
        default:
            authmode = WIFI_AUTH_OPEN;
            break;
    }

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::UpdateWiFiAuthmode, authmode %d %d", 
                    mStagingNetwork.auth_mode, authmode);

    if (mStagingNetwork.auth_mode != authmode)
    {
        mStagingNetwork.auth_mode = authmode;
        CommitConfiguration();
    }
}

CHIP_ERROR WiseWiFiDriver::RevertConfiguration()
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::RevertConfiguration");

    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool WiseWiFiDriver::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status WiseWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                           uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::AddOrUpdateNetwork");

    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memset(mStagingNetwork.credentials, 0, sizeof(mStagingNetwork.credentials));
    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());
    /* The input parameter "credentials" may experience a mismatch between data and length due to packet loss */
    VerifyOrReturnError(credentials.size() == strlen(mStagingNetwork.credentials), Status::kOutOfRange);

    memset(mStagingNetwork.ssid, 0, sizeof(mStagingNetwork.ssid));
    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

    mStagingNetwork.auth_mode = credentials.size() != 0 ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;

    ChipLogError(NetworkProvisioning, "AddOrUpdateNetwork ssid:%s, cred:%s, len:%d\n", mStagingNetwork.ssid, mStagingNetwork.credentials, mStagingNetwork.credentialsLen);
    return Status::kSuccess;
}

Status WiseWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::RemoveNetwork");

    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status WiseWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::ReorderNetwork");

    outDebugText.reduce_size(0);
    // Only one network is supported for now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

CHIP_ERROR WiseWiFiDriver::ConnectSavedNetwork()
{
    if (EnableInitConnect)
    {
        EnableInitConnect = false;
        return ConnectWiFiNetwork(mSavedNetwork.ssid, mSavedNetwork.ssidLen, mSavedNetwork.credentials, mSavedNetwork.credentialsLen);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiseWiFiDriver::ConnectWiFiNetwork(const char * ssid, uint8_t ssidLen, const char * key, uint8_t keyLen)
{
    scm_wifi_assoc_request req = {0};
#if 0
    // uint8_t pmk_stored[WISE_PMK_LEN] = {0};
    // scm_wifi_fast_assoc_request fast_request = {0};
    // const scm_wifi_ap_info * cachedApInfo = nullptr;
    // int ret;
#endif
    ChipLogError(NetworkProvisioning, "WiseWiFiDriver::ConnectWiFiNetwork");
    VerifyOrReturnError(!IsInitialScanBlockingConnections(), CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));

    memcpy(req.ssid, ssid, ssidLen);
    memcpy(req.key, key, keyLen);
    switch (mStagingNetwork.auth_mode)
    {
        case WIFI_AUTH_WPA3_SAE:
            req.auth = SCM_WIFI_SECURITY_SAE;
            break;
        case WIFI_AUTH_WPA2_PSK:
            req.auth = SCM_WIFI_SECURITY_WPA2PSK;
            break;
        case WIFI_AUTH_OPEN:
            req.auth = SCM_WIFI_SECURITY_OPEN;
            break;
        case SCM_WIFI_SECURITY_UNKNOWN:
        default:
            req.auth = SCM_WIFI_SECURITY_WPA2PSK;
            break;
    }
    req.pairwise = SCM_WIFI_PAIRWISE_AES;

    if (scm_wifi_sta_set_config(&req, NULL) != WISE_OK)
    {
        ChipLogError(DeviceLayer, "scm_wifi_sta_set_config() failed");
        return CHIP_ERROR_INTERNAL;
    }
#if 0
    ret = scm_wifi_get_options(SCM_WIFI_STA_GET_PSK, pmk_stored);

    ChipLogProgress(NetworkProvisioning, "Setting up connection for WiFi SSID: %s", ssid);

    ChipLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
    cachedApInfo = FindCachedScanResult(ssid, ssidLen);
    if (cachedApInfo != nullptr && ret == WISE_OK)
    {
        memcpy(fast_request.req.ssid, ssid, ssidLen);
        fast_request.req.auth     = req.auth;
        fast_request.req.pairwise = req.pairwise;
        memcpy(fast_request.req.bssid, cachedApInfo->bssid, sizeof(fast_request.req.bssid));
        fast_request.channel = static_cast<unsigned char>(cachedApInfo->channel);
        memcpy(fast_request.psk, pmk_stored, WISE_PMK_LEN);

        ChipLogError(DeviceLayer, "Using fast connect for cached WiFi SSID: %s on channel %u", ssid, fast_request.channel);
        ret = scm_wifi_sta_fast_connect(&fast_request);
        if (ret != WISE_OK)
        {
            ChipLogError(DeviceLayer, "scm_wifi_sta_fast_connect() failed, fallback to generic connect");
            ret = scm_wifi_sta_connect();
        }
    }
    else
    {
        if (cachedApInfo == nullptr)
        {
            ChipLogError(DeviceLayer, "No cached scan hit for WiFi SSID: %s, use generic connect", ssid);
        }
        else
        {
            ChipLogError(DeviceLayer, "No cached PSK for WiFi SSID: %s, use generic connect", ssid);
        }

        ret = scm_wifi_sta_connect();
    }

    if (ret != WISE_OK)
    {
        ChipLogError(DeviceLayer, "scm_wifi_connect() failed");
        return CHIP_ERROR_INTERNAL;
    }
#endif

    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled));
    ReturnErrorOnFailure(ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled));

    return CHIP_NO_ERROR;
}

void WiseWiFiDriver::OnConnectWiFiNetwork()
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::OnConnectWiFiNetwork");

    if (mpConnectCallback)
    {
        CommitConfiguration();
        DeviceLayer::SystemLayer().CancelTimer(OnConnectWiFiNetworkFailed, NULL);
        mpConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void WiseWiFiDriver::OnConnectWiFiNetworkFailed()
{
    if (mpConnectCallback)
    {
        mpConnectCallback->OnResult(Status::kNetworkNotFound, CharSpan(), 0);
        mpConnectCallback = nullptr;
    }
}

void WiseWiFiDriver::OnConnectWiFiNetworkFailed(chip::System::Layer * aLayer, void * aAppState)
{
#if 0
    /* Clear all req infos */
    scm_wifi_assoc_request req;
    memset(&req, 0, sizeof(scm_wifi_assoc_request));
    scm_wifi_sta_set_config(&req, NULL);
#endif
    WiseWiFiDriver::GetInstance().OnConnectWiFiNetworkFailed();
}

CHIP_ERROR WiseWiFiDriver::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    // todo: confirm the reason details ?
    lastDisconnectedReason = event->Platform.SCMSystemEvent.event.event_info.disconnected.reason;
    return CHIP_NO_ERROR;
}

int16_t WiseWiFiDriver::GetLastDisconnectReason()
{
    return lastDisconnectedReason;
}

void WiseWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    Status networkingStatus = Status::kUnknownError;
    Network configuredNetwork;

    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::ConnectNetwork");

    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), networkingStatus = Status::kNetworkIDNotFound);
    VerifyOrExit(mpConnectCallback == nullptr, networkingStatus = Status::kUnknownError);

#if 0
    if (CHIP_NO_ERROR == GetConfiguredNetwork(configuredNetwork))
    {
        printf("Skip ConnectWiFiNetwork00!!!\n");
        if (NetworkMatch(mStagingNetwork, ByteSpan(configuredNetwork.networkID, configuredNetwork.networkIDLen)))
        {
            if (callback)
            {
                printf("Skip ConnectWiFiNetwork11!!!\n");
                callback->OnResult(Status::kSuccess, CharSpan(), 0);
            }
            return;
        }
    }
#endif
    err = ConnectWiFiNetwork(mStagingNetwork.ssid, mStagingNetwork.ssidLen, mStagingNetwork.credentials,
                             mStagingNetwork.credentialsLen);
    if (err == CHIP_NO_ERROR)
    {
        mpConnectCallback = callback;
        networkingStatus  = Status::kSuccess;
    }

    err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kWiFiConnectNetworkTimeoutSeconds), OnConnectWiFiNetworkFailed, NULL);
exit:
    if (networkingStatus != Status::kSuccess)
    {
        ChipLogError(NetworkProvisioning, "Failed to connect to WiFi network:%s", chip::ErrorStr(err));
        mpConnectCallback = nullptr;
        callback->OnResult(networkingStatus, CharSpan(), 0);
    }
}

chip::BitFlags<WiFiSecurityBitmap> ConvertSecurityType(scm_wifi_auth_mode auth_mode)
{
    chip::BitFlags<WiFiSecurityBitmap> securityType;
    if (auth_mode == SCM_WIFI_SECURITY_OPEN)
    {
        securityType = WiFiSecurity::kUnencrypted;
    }
    else if (auth_mode == SCM_WIFI_SECURITY_WPAPSK)
    {
        securityType = WiFiSecurity::kWpaPersonal;
    }
    else if (auth_mode == SCM_WIFI_SECURITY_WPA2PSK)
    {
        securityType = WiFiSecurity::kWpa2Personal;
    }
    else if (auth_mode == SCM_WIFI_SECURITY_SAE)
    {
        securityType = WiFiSecurity::kWpa3Personal;
    }
    else
    {
        securityType = WiFiSecurity::kUnencrypted;
    }

    return securityType;
}

bool WiseWiFiDriver::StartScanWiFiNetworks(ByteSpan ssid)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::StartScanWiFiNetworks");

    ChipLogProgress(DeviceLayer, "Start Scan WiFi Networks");

    if (!ssid.empty()) // ssid is given, only scan this network
    {
        scm_wifi_scan_params sp = {0};
        sp.scan_type = SCM_WIFI_SSID_SCAN;
        sp.ssid_len = ssid.size();
        memcpy(sp.ssid, ssid.data(), ssid.size());
        scm_wifi_sta_advance_scan(&sp);
    }
    else // scan all networks
    {
        bool CacheValid = HasValidCachedWiFiScanResults();
        ChipLogError(DeviceLayer, "Cache scan results %s", CacheValid ? "valid" : "invalid");
        if (CacheValid)
        {
            ChipDeviceEvent e = { 0 };
            e.Type = DeviceEventType::kSCMSystemEvent;
            e.Platform.SCMSystemEvent.event.event_id = SYSTEM_EVENT_SCAN_DONE;
            (void) PlatformMgr().PostEvent(&e);
        }
        else
        {
            scm_wifi_sta_scan();
        }
    }

    return true;
}

void WiseWiFiDriver::OnScanWiFiNetworkDone()
{
    uint16_t apNumber = kMaxWiFiScanAPs;
    uint16_t num = 0;
    if (!GetInstance().mpScanCallback)
    {
        ChipLogProgress(DeviceLayer, "No scan callback");
        if (GetInstance().EnableInitialScan)
        {
            // Use a heap buffer for the per-round scan results instead of a
            // stack array: the latter cost ~1.7 KB of stack per call.
            std::unique_ptr<scm_wifi_ap_info[]> apBuffer(new scm_wifi_ap_info[kMaxWiFiScanAPs]());

            if (apBuffer != nullptr && scm_wifi_sta_scan_results(apBuffer.get(), &apNumber, kMaxWiFiScanAPs) == WISE_OK)
            {
                MergeWiFiScanResults(apBuffer.get(), apNumber);
                ChipLogProgress(DeviceLayer, "Merged %u WiFi scan results without callback, cache count=%u", apNumber,
                                gCachedWiFiScanResults.mCount);
            }
            else
            {
                ChipLogError(DeviceLayer, "Failed to cache WiFi scan results without callback");
                InvalidateCachedWiFiScanResults();
            }

            if (GetInstance().InitialScanCnt > 0)
            {
                GetInstance().InitialScanCnt--;
            }

            if (GetInstance().InitialScanCnt > 0)
            {
                if (!GetInstance().StartNextInitialScanRound())
                {
                    ChipLogError(DeviceLayer, "Failed to continue initial WiFi scan");
                }
            }
            else
            {
                LogCachedWiFiScanResults();
                ChipLogProgress(DeviceLayer, "Initial WiFi scan completed");
                GetInstance().ClearInitialScanState();
            }
        }
        return;
    }

    // Match the behavior of StartScanWiFiNetworks():
    //  - if the cache is still valid (a synthetic SCAN_DONE event was posted),
    //    report the cached results directly without fetching again;
    //  - otherwise fetch fresh scan results into the static cache (BSS)
    //    first, then report. No heap buffer is transferred into the async
    //    lambda, so there is nothing to free asynchronously and no leak risk.
    if (HasValidCachedWiFiScanResults())
    {
        ChipLogError(DeviceLayer, "WiFi scan cache valid, report directly, cache count=%u", gCachedWiFiScanResults.mCount);
    }
    else
    {
        if (scm_wifi_sta_scan_results(gCachedWiFiScanResults.mResults.data(), &num,
                                      static_cast<uint16_t>(gCachedWiFiScanResults.mResults.size())) != WISE_OK)
        {
            ChipLogError(DeviceLayer, "can't get ap_records");
            GetInstance().mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
            GetInstance().mpScanCallback = nullptr;
            return;
        }

        if (num > 0)
        {
            gCachedWiFiScanResults.mCount     = num;
            gCachedWiFiScanResults.mTimestamp = System::SystemClock().GetMonotonicTimestamp();
            gCachedWiFiScanResults.mValid     = true;
        }
    }

    const uint16_t resultCount = gCachedWiFiScanResults.mValid ? gCachedWiFiScanResults.mCount : 0;

    if (CHIP_NO_ERROR == DeviceLayer::SystemLayer().ScheduleLambda([resultCount]() {
            WiseScanResponseIterator iter(resultCount, gCachedWiFiScanResults.mResults.data());
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
        ChipLogError(DeviceLayer, "Reported WiFi scan results, fresh scan count: %u cache count: %u", num,
                     gCachedWiFiScanResults.mCount);
    }
    else
    {
        ChipLogError(DeviceLayer, "can't schedule the scan result processing");
        GetInstance().mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        GetInstance().mpScanCallback = nullptr;
    }
}

void WiseWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::ScanNetworks");

    if (callback != nullptr)
    {
        mpScanCallback = callback;
        if (!StartScanWiFiNetworks(ssid))
        {
            ChipLogError(DeviceLayer, "ScanWiFiNetworks failed to start");
            mpScanCallback = nullptr;
            callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
    }
}

CHIP_ERROR GetConnectedNetwork(Network & network)
{
    ChipLogProgress(NetworkProvisioning, "GetConnectedNetwork");

    wifi_config_t stationConfig;
    scm_wifi_get_config(WIFI_IF_STA, &stationConfig);
    uint8_t length = strnlen(reinterpret_cast<const char*>(stationConfig.sta.ssid), DeviceLayer::Internal::kMaxWiFiSSIDLength);
    if (length > sizeof(network.networkID))
    {
        ChipLogError(DeviceLayer, "SSID too long");
        return CHIP_ERROR_INTERNAL;
    }

    memcpy(network.networkID, stationConfig.sta.ssid, length);
    network.networkIDLen = length;

    return CHIP_NO_ERROR;
}

size_t WiseWiFiDriver::WiFiNetworkIterator::Count()
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::WiFiNetworkIterator::Count");

    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool WiseWiFiDriver::WiFiNetworkIterator::Next(Network & item)
{
    ChipLogProgress(NetworkProvisioning, "WiseWiFiDriver::WiFiNetworkIterator::Next");

    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = mDriver->mStagingNetwork.ssidLen;
    item.connected    = false;
    mExhausted        = true;

    Network connectedNetwork;
    CHIP_ERROR err = GetConnectedNetwork(connectedNetwork);
    if (err == CHIP_NO_ERROR)
    {
        if (connectedNetwork.networkIDLen == item.networkIDLen &&
            memcmp(connectedNetwork.networkID, item.networkID, item.networkIDLen) == 0)
        {
            item.connected = true;
        }
    }
    return true;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
