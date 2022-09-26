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
#include <platform/mbed/NetworkCommissioningDriver.h>

#include <net_common.h>
#include <netsocket/WiFiInterface.h>

using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

CHIP_ERROR WiFiDriverImpl::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    mSecurityType = NSAPI_SECURITY_WPA_WPA2;

    auto net_if = get_mbed_net_if();
    if (net_if == nullptr)
    {
        ChipLogError(DeviceLayer, "No network interface available");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (net_if->wifiInterface() != nullptr)
    {
        mWiFiInterface = net_if->wifiInterface();
        // To add more listener to the interface
        mWiFiInterface->add_event_listener(
            [this](nsapi_event_t event, intptr_t data) { ConnectivityMgrImpl().AddQueueEvent(OnWiFiInterfaceEvent, event, data); });

        mWiFiInterface->set_blocking(false);
    }

    mScanCallback         = nullptr;
    mConnectCallback      = nullptr;
    mScanSpecific         = false;
    mStatusChangeCallback = networkStatusChangeCallback;

    mIp4Address = IPAddress::Any;
    mIp6Address = IPAddress::Any;

    auto err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, mSavedNetwork.ssid, sizeof(mSavedNetwork.ssid), &ssidLen);
    if (err == CHIP_NO_ERROR)
    {
        mSavedNetwork.ssidLen = ssidLen;
    }

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, mSavedNetwork.credentials,
                                                   sizeof(mSavedNetwork.credentials), &credentialsLen);
    if (err == CHIP_NO_ERROR)
    {
        mSavedNetwork.credentialsLen = credentialsLen;
    }

    if (mSavedNetwork.ssidLen != 0 && mSavedNetwork.credentialsLen != 0)
    {
        mStagingNetwork = mSavedNetwork;
    }

    return CHIP_NO_ERROR;
}

void WiFiDriverImpl::Shutdown()
{
    Network network;
    auto networks = GetNetworks();
    for (; networks != nullptr && networks->Next(network);)
    {
        if (network.connected)
        {
            DisconnectNetwork(ByteSpan(network.networkID, network.networkIDLen));
        }
    }
    if (networks != nullptr)
    {
        networks->Release();
    }
    mScanCallback         = nullptr;
    mConnectCallback      = nullptr;
    mStatusChangeCallback = nullptr;
    mScanSpecific         = false;
    mWiFiInterface        = nullptr;
    mIp4Address           = IPAddress::Any;
    mIp6Address           = IPAddress::Any;
    mSecurityType         = NSAPI_SECURITY_NONE;
    memset(mScanSSID, 0, sizeof(mScanSSID));
    mStagingNetwork.ssidLen        = 0;
    mStagingNetwork.credentialsLen = 0;
    mSavedNetwork.ssidLen          = 0;
    mSavedNetwork.credentialsLen   = 0;
}

CHIP_ERROR WiFiDriverImpl::CommitConfiguration()
{
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, mStagingNetwork.ssid, mStagingNetwork.ssidLen));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, mStagingNetwork.credentials,
                                                                  mStagingNetwork.credentialsLen));
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiDriverImpl::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

bool WiFiDriverImpl::NetworkMatch(const WiFiNetwork & network, ByteSpan networkId)
{
    return networkId.size() == network.ssidLen && memcmp(networkId.data(), network.ssid, network.ssidLen) == 0;
}

Status WiFiDriverImpl::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                          uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(mStagingNetwork.ssidLen == 0 || NetworkMatch(mStagingNetwork, ssid), Status::kBoundsExceeded);
    VerifyOrReturnError(credentials.size() <= sizeof(mStagingNetwork.credentials), Status::kOutOfRange);
    VerifyOrReturnError(ssid.size() <= sizeof(mStagingNetwork.ssid), Status::kOutOfRange);

    memcpy(mStagingNetwork.credentials, credentials.data(), credentials.size());
    mStagingNetwork.credentialsLen = static_cast<decltype(mStagingNetwork.credentialsLen)>(credentials.size());

    memcpy(mStagingNetwork.ssid, ssid.data(), ssid.size());
    mStagingNetwork.ssidLen = static_cast<decltype(mStagingNetwork.ssidLen)>(ssid.size());

    return Status::kSuccess;
}

Status WiFiDriverImpl::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);

    // Use empty ssid for representing invalid network
    mStagingNetwork.ssidLen = 0;
    return Status::kSuccess;
}

Status WiFiDriverImpl::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    // Only one network is supported now
    VerifyOrReturnError(index == 0, Status::kOutOfRange);
    VerifyOrReturnError(NetworkMatch(mStagingNetwork, networkId), Status::kNetworkIDNotFound);
    return Status::kSuccess;
}

void WiFiDriverImpl::ExecuteWiFiInterfaceChange(nsapi_connection_status_t status)
{

    switch (status)
    {
    case NSAPI_STATUS_LOCAL_UP:
        ChipLogDetail(DeviceLayer, "Connection status - LOCAL_UP");
        OnNetworkConnected();
        break;
    case NSAPI_STATUS_GLOBAL_UP:
        ChipLogDetail(DeviceLayer, "Connection status - GLOBAL_UP");
        OnNetworkConnected();
        break;
    case NSAPI_STATUS_DISCONNECTED:
        ChipLogDetail(DeviceLayer, "Connection status - DISCONNECTED");
        OnNetworkDisconnected();
        break;
    case NSAPI_STATUS_CONNECTING:
        ChipLogDetail(DeviceLayer, "Connection status - CONNECTING");
        OnNetworkConnecting();
        break;
    default:
        ChipLogDetail(DeviceLayer, "Unknown connection status: 0x%08X", status);
        break;
    }
}

void WiFiDriverImpl::OnWiFiInterfaceEvent(nsapi_event_t event, intptr_t data)
{
    if (event == NSAPI_EVENT_CONNECTION_STATUS_CHANGE)
    {
        GetInstance().ExecuteWiFiInterfaceChange((nsapi_connection_status_t) data);
    }
}

void WiFiDriverImpl::ExecuteConnectNetwork()
{
    const char * ssid = mStagingNetwork.ssid;
    const char * key  = mStagingNetwork.credentials;

    // Set WiFi credentials
    auto error = mWiFiInterface->set_credentials(ssid, key, mSecurityType);
    if (error)
    {
        ChipLogError(DeviceLayer, "Set WiFi credentials failed %d", error);
        if (mConnectCallback)
        {
            mConnectCallback->OnResult(Status::kUnknownError, CharSpan(), 0);
            mConnectCallback = nullptr;
        }
        return;
    }

    // Connect Wifi network
    error = mWiFiInterface->connect();
    if (error)
    {
        ChipLogError(DeviceLayer, "Connect WiFi network failed %d", error);
        if (mConnectCallback)
        {
            mConnectCallback->OnResult(Status::kOtherConnectionFailure, CharSpan(), 0);
            mConnectCallback = nullptr;
        }
        return;
    }
}

void WiFiDriverImpl::OnConnectNetwork(intptr_t arg)
{
    GetInstance().ExecuteConnectNetwork();
}

void WiFiDriverImpl::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    Status status = Status::kSuccess;

    VerifyOrExit(mWiFiInterface != nullptr, status = Status::kUnknownError);
    VerifyOrExit(NetworkMatch(mStagingNetwork, networkId), status = Status::kNetworkIDNotFound);
    VerifyOrExit(mConnectCallback == nullptr, status = Status::kUnknownError);
    ChipLogProgress(NetworkProvisioning, "Mbed WiFi driver connect network: SSID: %.*s", static_cast<int>(networkId.size()),
                    networkId.data());
    mConnectCallback = callback;
    ConnectivityMgrImpl().AddTask(OnConnectNetwork, 0);
exit:
    if (status != Status::kSuccess)
    {
        ChipLogError(DeviceLayer, "Connect WiFi network failed: 0x%x", int(status));
        if (callback)
        {
            callback->OnResult(status, CharSpan(), 0);
        }
    }
}

void WiFiDriverImpl::DisconnectNetwork(ByteSpan networkId)
{
    VerifyOrReturn(mWiFiInterface != nullptr, ChipLogError(DeviceLayer, "Wifi network not available"));
    VerifyOrReturn(NetworkMatch(mStagingNetwork, networkId), ChipLogError(DeviceLayer, "Network not found"));
    ChipLogProgress(NetworkProvisioning, "Mbed WiFi driver disconnect network: SSID: %.*s", static_cast<int>(networkId.size()),
                    networkId.data());

    // Disconnect Wifi network
    auto error = mWiFiInterface->disconnect();
    if (error)
    {
        ChipLogError(DeviceLayer, "Disconnect WiFi network failed %d", error);
    }
}

void WiFiDriverImpl::ExecuteScanNetwork()
{
    uint16_t ap_number, ap_index = 0;
    WiFiAccessPoint * ap_buffer;
    MbedScanResponseIterator * scan_resp_iter;

    // Get APs number
    ap_number = mWiFiInterface->scan(nullptr, 0);
    if (!ap_number)
    {
        ChipLogProgress(DeviceLayer, "No AP found");
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
        goto exit;
    }

    ap_buffer = new WiFiAccessPoint[ap_number];
    if (ap_buffer == nullptr)
    {
        ChipLogError(DeviceLayer, "Can't malloc memory for AP list");
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
        goto exit;
    }

    // Get APs details
    if (mWiFiInterface->scan(ap_buffer, ap_number) < 0)
    {
        if (mScanSpecific)
        {
            while (ap_index < ap_number)
            {
                if (strcmp(ap_buffer->get_ssid(), mScanSSID) == 0)
                {
                    break;
                }
                ap_buffer++;
                ap_index++;
            }
            if (ap_index == ap_number)
            {
                ChipLogError(DeviceLayer, "Specific AP not found");
                if (mScanCallback)
                {
                    mScanCallback->OnFinished(Status::kNetworkNotFound, CharSpan(), nullptr);
                }
                goto exit;
            }
            ap_number = 1;
        }
        scan_resp_iter = new MbedScanResponseIterator(ap_number, ap_buffer);
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kSuccess, CharSpan(), scan_resp_iter);
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Scan Wifi AP failed");
        if (mScanCallback)
        {
            mScanCallback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
        }
        goto exit;
    }

exit:
    if (ap_buffer)
    {
        delete[] ap_buffer;
    }
    mScanCallback = nullptr;
    mScanSpecific = false;
}

void WiFiDriverImpl::OnScanNetwork(intptr_t arg)
{
    GetInstance().ExecuteScanNetwork();
}

void WiFiDriverImpl::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback)
{
    Status status = Status::kSuccess;

    VerifyOrExit(mWiFiInterface != nullptr, status = Status::kUnknownError);

    if (ssid.data())
    {
        memset(mScanSSID, 0, sizeof(mScanSSID));
        memcpy(mScanSSID, ssid.data(), ssid.size());
        mScanSpecific = true;
    }
    mScanCallback = callback;
    ConnectivityMgrImpl().AddTask(OnScanNetwork, 0);

exit:
    if (status != Status::kSuccess)
    {
        ChipLogError(DeviceLayer, "Scan WiFi networks failed: 0x%x", int(status));
        if (callback)
        {
            callback->OnFinished(status, CharSpan(), nullptr);
        }
    }
}

CHIP_ERROR WiFiDriverImpl::SetLastDisconnectReason(const ChipDeviceEvent * event)
{
    (void) event;
    mLastDisconnectedReason = 0;
    return CHIP_NO_ERROR;
}

int32_t WiFiDriverImpl::GetLastDisconnectReason()
{
    return mLastDisconnectedReason;
}

size_t WiFiDriverImpl::WiFiNetworkIterator::Count()
{
    return mDriver->mStagingNetwork.ssidLen == 0 ? 0 : 1;
}

bool WiFiDriverImpl::WiFiNetworkIterator::Next(Network & item)
{
    if (mExhausted || mDriver->mStagingNetwork.ssidLen == 0)
    {
        return false;
    }
    memcpy(item.networkID, mDriver->mStagingNetwork.ssid, mDriver->mStagingNetwork.ssidLen);
    item.networkIDLen = mDriver->mStagingNetwork.ssidLen;
    item.connected    = get_mbed_net_if()->wifiInterface()->get_connection_status() == NSAPI_STATUS_LOCAL_UP ||
        get_mbed_net_if()->wifiInterface()->get_connection_status() == NSAPI_STATUS_GLOBAL_UP;
    mExhausted = true;

    return true;
}

void WiFiDriverImpl::OnNetworkConnected()
{
    ChipLogDetail(DeviceLayer, "OnNetworkConnected");

    if (mConnectCallback)
    {
        mConnectCallback->OnResult(Status::kSuccess, CharSpan(), 0);
        mConnectCallback = nullptr;
    }

    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Established;
    ConnectivityMgrImpl().PostEvent(&event, true);

    // Update IP address
    SocketAddress address;
    auto error = mWiFiInterface->get_ip_address(&address);
    if (error != NSAPI_ERROR_OK)
    {
        if (mIp4Address != IPAddress::Any)
        {
            // Unexpected change, forward to the application
            mIp4Address = IPAddress::Any;
            ChipDeviceEvent event;
            event.Type                                 = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4      = kConnectivity_Lost;
            event.InternetConnectivityChange.IPv6      = kConnectivity_NoChange;
            event.InternetConnectivityChange.ipAddress = mIp4Address;
            ConnectivityMgrImpl().PostEvent(&event, true);
            ChipLogError(DeviceLayer, "Unexpected loss of Ip4 address");
        }

        if (mIp6Address != IPAddress::Any)
        {
            // Unexpected change, forward to the application
            mIp6Address = IPAddress::Any;
            ChipDeviceEvent event;
            event.Type                                 = DeviceEventType::kInternetConnectivityChange;
            event.InternetConnectivityChange.IPv4      = kConnectivity_NoChange;
            event.InternetConnectivityChange.IPv6      = kConnectivity_Lost;
            event.InternetConnectivityChange.ipAddress = mIp6Address;
            ConnectivityMgrImpl().PostEvent(&event, true);
            ChipLogError(DeviceLayer, "Unexpected loss of Ip6 address");
        }
    }
    else
    {
        IPAddress addr;
        if (address.get_ip_version() == NSAPI_IPv4)
        {
            if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp4Address)
            {
                mIp4Address = addr;
                ChipDeviceEvent event;
                event.Type                                 = DeviceEventType::kInternetConnectivityChange;
                event.InternetConnectivityChange.IPv4      = kConnectivity_Established;
                event.InternetConnectivityChange.IPv6      = kConnectivity_NoChange;
                event.InternetConnectivityChange.ipAddress = mIp4Address;
                ConnectivityMgrImpl().PostEvent(&event, true);
                ChipLogProgress(DeviceLayer, "New Ip4 address set: %s", address.get_ip_address());
            }

            error = mWiFiInterface->get_ipv6_link_local_address(&address);
            if (error)
            {
                if (mIp6Address != IPAddress::Any)
                {
                    // Unexpected change, forward to the application
                    mIp6Address = IPAddress::Any;
                    ChipDeviceEvent event;
                    event.Type                                 = DeviceEventType::kInternetConnectivityChange;
                    event.InternetConnectivityChange.IPv4      = kConnectivity_NoChange;
                    event.InternetConnectivityChange.IPv6      = kConnectivity_Lost;
                    event.InternetConnectivityChange.ipAddress = mIp6Address;
                    ConnectivityMgrImpl().PostEvent(&event, true);
                    ChipLogError(DeviceLayer, "Unexpected loss of Ip6 address");
                }
            }
            else
            {
                if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp6Address)
                {
                    mIp6Address = addr;
                    ChipDeviceEvent event;
                    event.Type                                 = DeviceEventType::kInternetConnectivityChange;
                    event.InternetConnectivityChange.IPv4      = kConnectivity_NoChange;
                    event.InternetConnectivityChange.IPv6      = kConnectivity_Established;
                    event.InternetConnectivityChange.ipAddress = mIp6Address;
                    ConnectivityMgrImpl().PostEvent(&event, true);
                    ChipLogProgress(DeviceLayer, "New Ip6 address set %s", address.get_ip_address());
                }
            }
        }
        else
        {
            if (IPAddress::FromString(address.get_ip_address(), addr) && addr != mIp6Address)
            {
                mIp6Address = addr;
                ChipDeviceEvent event;
                event.Type                                 = DeviceEventType::kInternetConnectivityChange;
                event.InternetConnectivityChange.IPv4      = kConnectivity_NoChange;
                event.InternetConnectivityChange.IPv6      = kConnectivity_Established;
                event.InternetConnectivityChange.ipAddress = mIp6Address;
                ConnectivityMgrImpl().PostEvent(&event, true);
                ChipLogProgress(DeviceLayer, "New Ip6 address set %s", address.get_ip_address());
            }
        }
    }
}

void WiFiDriverImpl::OnNetworkDisconnected()
{
    ChipLogDetail(DeviceLayer, "OnStationDisconnected");

    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_Lost;
    ConnectivityMgrImpl().PostEvent(&event, true);

    // Update IPv4 address
    if (mIp4Address != IPAddress::Any)
    {
        // Unexpected change, forward to the application
        mIp4Address = IPAddress::Any;
        ChipDeviceEvent event;
        event.Type                                 = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4      = kConnectivity_Lost;
        event.InternetConnectivityChange.IPv6      = kConnectivity_NoChange;
        event.InternetConnectivityChange.ipAddress = mIp4Address;
        ConnectivityMgrImpl().PostEvent(&event, true);
        ChipLogError(DeviceLayer, "Loss of Ip4 address");
    }

    if (mIp6Address != IPAddress::Any)
    {
        // Unexpected change, forward to the application
        mIp6Address = IPAddress::Any;
        ChipDeviceEvent event;
        event.Type                                 = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4      = kConnectivity_NoChange;
        event.InternetConnectivityChange.IPv6      = kConnectivity_Lost;
        event.InternetConnectivityChange.ipAddress = mIp6Address;
        ConnectivityMgrImpl().PostEvent(&event, true);
        ChipLogError(DeviceLayer, "Loss of Ip6 address");
    }
}

void WiFiDriverImpl::OnNetworkConnecting()
{
    ChipLogDetail(DeviceLayer, "OnNetworkConnecting");

    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kWiFiConnectivityChange;
    event.WiFiConnectivityChange.Result = kConnectivity_NoChange;
    ConnectivityMgrImpl().PostEvent(&event, true);
}

WiFiAuthSecurityType WiFiDriverImpl::NsapiToNetworkSecurity(nsapi_security_t nsapi_security)
{
    switch (nsapi_security)
    {
    case NSAPI_SECURITY_NONE:
        return kWiFiSecurityType_None;
    case NSAPI_SECURITY_WEP:
        return kWiFiSecurityType_WEP;
    case NSAPI_SECURITY_WPA:
        return kWiFiSecurityType_WPAPersonal;
    case NSAPI_SECURITY_WPA2:
        return kWiFiSecurityType_WPA2Personal;
    case NSAPI_SECURITY_WPA_WPA2:
        return kWiFiSecurityType_WPA2MixedPersonal;
    case NSAPI_SECURITY_WPA2_ENT:
        return kWiFiSecurityType_WPA2Enterprise;
    case NSAPI_SECURITY_WPA3:
        return kWiFiSecurityType_WPA3Personal;
    case NSAPI_SECURITY_WPA3_WPA2:
        return kWiFiSecurityType_WPA3MixedPersonal;
    case NSAPI_SECURITY_PAP:
    case NSAPI_SECURITY_CHAP:
    case NSAPI_SECURITY_EAP_TLS:
    case NSAPI_SECURITY_PEAP:
    case NSAPI_SECURITY_UNKNOWN:
    default:
        return kWiFiSecurityType_NotSpecified;
    }
}

void WiFiDriverImpl::OnNetworkStatusChange()
{
    // Network configuredNetwork;
    bool staEnabled   = ConnectivityMgrImpl().IsWiFiStationEnabled();
    bool staConnected = ConnectivityMgrImpl().IsWiFiStationConnected();
    VerifyOrReturn(staEnabled && mStatusChangeCallback != nullptr);

    if (staConnected)
    {
        mStatusChangeCallback->OnNetworkingStatusChange(
            Status::kSuccess, MakeOptional(ByteSpan((const uint8_t *) mStagingNetwork.ssid, mStagingNetwork.ssidLen)),
            NullOptional);
        return;
    }
    mStatusChangeCallback->OnNetworkingStatusChange(
        Status::kUnknownError, MakeOptional(ByteSpan((const uint8_t *) mStagingNetwork.ssid, mStagingNetwork.ssidLen)),
        MakeOptional(GetLastDisconnectReason()));
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
