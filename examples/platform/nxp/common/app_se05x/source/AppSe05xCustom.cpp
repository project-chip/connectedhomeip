/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#if CONFIG_CHIP_SE05X
#include "AppSe05x.h"
#include <app/FailSafeContext.h>
#include <app/server/Server.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/nxp/crypto/se05x/kvs_utilities/CHIPCryptoPALHsm_se05x_readClusters.h>
#include <platform/nxp/crypto/se05x/mcu_kvs/NFCDataRetrievalInfo.hpp>

namespace chip {
namespace NXP {
namespace App {
namespace Se05x {

#ifdef __linux__
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
static DeviceLayer::NetworkCommissioning::LinuxThreadDriver sLinuxThreadDriver;

class ThreadConnectCallback : public DeviceLayer::NetworkCommissioning::Internal::WirelessDriver::ConnectCallback
{
public:
    void OnResult(DeviceLayer::NetworkCommissioning::Status status, CharSpan debugText, int32_t connectStatus) override
    {
        if (status == DeviceLayer::NetworkCommissioning::Status::kSuccess)
        {
            ChipLogProgress(NotSpecified, "Thread ConnectNetwork succeeded, committing configuration");
            CHIP_ERROR err = sLinuxThreadDriver.CommitConfiguration();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(NotSpecified, "Failed to commit Thread config: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
        else
        {
            ChipLogError(NotSpecified, "Thread ConnectNetwork failed with status: %d", static_cast<int>(status));
            TEMPORARY_RETURN_IGNORED sLinuxThreadDriver.RevertConfiguration();
        }
    }
};

static ThreadConnectCallback sThreadConnectCallback;

CHIP_ERROR linux_connect_to_thread_network()
{
    char debugTextBuffer[128];
    MutableCharSpan debugText(debugTextBuffer);
    uint8_t networkIndex   = 0;
    char op_data_set[256]  = { 0 };
    size_t op_data_set_len = sizeof(op_data_set);

    CHIP_ERROR err =
        DeviceLayer::NFCCommissioning::NFCDataRetrievalInfoMgr().GetOperationalDataSetFormSE05x(op_data_set, &op_data_set_len);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    if (op_data_set_len == 0)
    {
        ChipLogDetail(NotSpecified, "SE05x: No operational data set found from SE05x");
        return CHIP_NO_ERROR;
    }

    err = sLinuxThreadDriver.Init(nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to init LinuxThreadDriver: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    DeviceLayer::NetworkCommissioning::Status status =
        sLinuxThreadDriver.AddOrUpdateNetwork(ByteSpan((uint8_t *) op_data_set, op_data_set_len), debugText, networkIndex);
    if (status != DeviceLayer::NetworkCommissioning::Status::kSuccess)
    {
        ChipLogError(NotSpecified, "AddOrUpdateNetwork failed with status: %d", static_cast<int>(status));
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(NotSpecified, "AddOrUpdateNetwork succeeded, networkIndex=%u", networkIndex);

    Thread::OperationalDataset dataset;
    err = dataset.Init(ByteSpan((uint8_t *) op_data_set, op_data_set_len));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to init OperationalDataset: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    uint8_t extPanId[Thread::kSizeExtendedPanId];
    err = dataset.GetExtendedPanId(extPanId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to get ExtendedPanId: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    ChipLogProgress(NotSpecified, "Calling ConnectNetwork...");
    sLinuxThreadDriver.ConnectNetwork(ByteSpan(extPanId, sizeof(extPanId)), &sThreadConnectCallback);

    return CHIP_NO_ERROR;
}

#endif // #if CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

/**
 * @brief Connect to WiFi network using credentials stored in KVS
 *
 * This function retrieves WiFi network credentials (SSID and password) from the
 * Key-Value Storage and attempts to connect to the WiFi network asynchronously.
 *
 * @return CHIP_NO_ERROR on success, or appropriate error code on failure
 *
 */
CHIP_ERROR linux_connect_to_wifi_network()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    size_t ssidLen = 0;
    uint8_t credentials[DeviceLayer::Internal::kMaxWiFiKeyLength];
    size_t credentialsLen = 0;

    auto & kvs = DeviceLayer::PersistedStorage::KeyValueStoreMgr();

    // Retrieve WiFi SSID from KVS
    err = kvs.Get(kWiFiSSIDKeyName, ssid, sizeof(ssid), &ssidLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogDetail(NotSpecified, "No WiFi SSID found in KVS");
        return CHIP_NO_ERROR;
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to retrieve WiFi SSID: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    if (ssidLen > sizeof(ssid))
    {
        ChipLogError(NotSpecified, "Invalid SSID length: %zu", ssidLen);
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    if (ssidLen != 0)
    {
        // Retrieve WiFi credentials from KVS
        err = kvs.Get(kWiFiCredentialsKeyName, credentials, sizeof(credentials), &credentialsLen);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            ChipLogDetail(NotSpecified, "No WiFi credentials found in KVS");
            return CHIP_NO_ERROR;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to retrieve WiFi credentials: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
        if (credentialsLen > sizeof(credentials))
        {
            ChipLogError(NotSpecified, "Invalid credentials length: %zu", credentialsLen);
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        // Connect to WiFi network asynchronously
        err = DeviceLayer::ConnectivityMgrImpl().ConnectWiFiNetworkAsync(ByteSpan(ssid, ssidLen),
                                                                          ByteSpan(credentials, credentialsLen), NULL);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed to connect to WiFi network: %" CHIP_ERROR_FORMAT, err.Format());
            return CHIP_ERROR_INTERNAL;
        }

        ChipLogProgress(NotSpecified, "WiFi connection initiated successfully");
    }
    return CHIP_NO_ERROR;
}
#endif // #if CHIP_DEVICE_CONFIG_ENABLE_WPA
#endif // #ifdef __linux__

CHIP_ERROR Init()
{
    return chip::DeviceLayer::NFCCommissioning::NFCDataRetrievalInfoMgr().Init();
}

CHIP_ERROR PostInit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DeviceLayer, "SE05x - Check for SE05x fail safe timer");

    uint16_t fail_safe_time = DeviceLayer::NFCCommissioning::NFCDataRetrievalInfoMgr().GetRemainingFailSafeTimerForSE05x();

    if (fail_safe_time != 0)
    {
        chip::app::FailSafeContext & failSafeContext = chip::Server::GetInstance().GetFailSafeContext();

        err = failSafeContext.ArmFailSafe(se05x_get_fabric_id(), chip::System::Clock::Seconds16(fail_safe_time));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "SE05x - Error in starting Fail Safe timer for SE05x NFC commissioned fabric");
        }
        else
        {
            ChipLogDetail(NotSpecified, "SE05x - Started Fail Safe timer for SE05x NFC commissioned fabric");
        }
    }

#ifdef __linux__
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    TEMPORARY_RETURN_IGNORED linux_connect_to_thread_network();
#endif // #if CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    TEMPORARY_RETURN_IGNORED linux_connect_to_wifi_network();
#endif // #if CHIP_DEVICE_CONFIG_ENABLE_WPA
#endif // #ifdef __linux__

    return err;
}

} // namespace Se05x
} // namespace App
} // namespace NXP
} // namespace chip

#endif // CONFIG_CHIP_SE05X
