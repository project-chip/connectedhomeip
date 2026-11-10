/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    All rights reserved.
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

/**
 * @file CommonDeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "CommonDeviceCallbacks.h"
#include "AppTaskBase.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/Dnssd.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>

#include <lib/support/CodeUtils.h>
#if CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
#include "openthread-system.h"
#endif /* CHIP_ENABLE_OPENTHREAD && CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED */

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OTARequestorInitiator.h"
#endif

#if CHIP_APP_FAILSAFE_EARLY_EXPIRY
#include <app/server/Server.h>
#include <openthread/thread.h>
#include <platform/ThreadStackManager.h>
#include <platform/internal/BLEManager.h>
#include <transport/SecureSession.h>
#endif

using namespace chip::app;
using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;

#if CHIP_APP_FAILSAFE_EARLY_EXPIRY
namespace {
constexpr uint32_t kMaxAttachAttemptsAfterBleLoss = 3;
constexpr System::Clock::Seconds32 kAttachPollInterval(1);

bool sConnectNetworkReceived = false;
bool sAttachWatch            = false;
uint32_t sAttachBaseline     = 0;

uint32_t GetAttachAttempts()
{
    ThreadStackMgr().LockThreadStack();
    uint32_t attempts = otThreadGetMleCounters(ThreadStackMgrImpl().OTInstance())->mAttachAttempts;
    ThreadStackMgr().UnlockThreadStack();
    return attempts;
}

void PollAttach(System::Layer *, void *);

void ResetEarlyExpiry()
{
    SystemLayer().CancelTimer(PollAttach, nullptr);
    sConnectNetworkReceived = false;
    sAttachWatch            = false;
    sAttachBaseline         = 0;
}

void ExpireFailSafe(const char * reason)
{
    ChipLogProgress(AppServer, "Fail-safe early expiry: %s", reason);
    Server::GetInstance().GetFailSafeContext().ForceFailSafeTimerExpiry();
    ResetEarlyExpiry();
}

void PollAttach(System::Layer *, void *)
{
    VerifyOrReturn(sAttachWatch);

    if (!Server::GetInstance().GetFailSafeContext().IsFailSafeArmed() || ConnectivityMgr().IsThreadAttached())
    {
        ResetEarlyExpiry();
        return;
    }

    uint32_t current = GetAttachAttempts();
    if (current < sAttachBaseline)
    {
        sAttachBaseline = current; // counter reset by the stack, re-baseline
    }

    if (current - sAttachBaseline >= kMaxAttachAttemptsAfterBleLoss)
    {
        ExpireFailSafe("Thread attach failed after CHIPoBLE loss");
        return;
    }

    TEMPORARY_RETURN_IGNORED SystemLayer().StartTimer(kAttachPollInterval, PollAttach, nullptr);
}

// Fail-safe armed for a PASE commissioning session carried over BLE
// (excludes CASE/UpdateNOC fail-safes and on-network PASE such as ECM/JCM).
bool IsBleCommissioningFailSafe()
{
    auto & failSafe = Server::GetInstance().GetFailSafeContext();
    VerifyOrReturnValue(failSafe.IsFailSafeArmed() && !failSafe.UpdateNocCommandHasBeenInvoked(), false);

    auto pase = Server::GetInstance().GetCommissioningWindowManager().GetPASESession();
    VerifyOrReturnValue(pase.HasValue(), false);
    const Transport::SecureSession * session = pase.Value()->AsSecureSession();
    VerifyOrReturnValue(session != nullptr, false);
    return session->GetPeerAddress().GetTransportType() == Transport::Type::kBle;
}

void StartAttachWatch()
{
    otInstance * instance = ThreadStackMgrImpl().OTInstance();

    // Restart the attacher so that the OT attach backoff starts again from its minimum.
    // The OT API is used directly: the dataset and the Matter InterfaceEnabled state are untouched.
    ThreadStackMgr().LockThreadStack();
    otThreadSetEnabled(instance, false);
    otError otErr   = otThreadSetEnabled(instance, true);
    sAttachBaseline = otThreadGetMleCounters(instance)->mAttachAttempts;
    ThreadStackMgr().UnlockThreadStack();

    if (otErr != OT_ERROR_NONE)
    {
        // Thread was disabled above but could not be re-enabled: with no BLE bearer and no Thread,
        // waiting for natural expiry is pointless. Expire now so the revert path can recover.
        ChipLogError(AppServer, "Fail-safe early expiry: Thread restart failed (%d)", otErr);
        ExpireFailSafe("Thread restart failed");
        return;
    }

    sAttachWatch = true;
    ChipLogProgress(AppServer, "Fail-safe early expiry: CHIPoBLE closed after ConnectNetwork, Thread restarted");
    TEMPORARY_RETURN_IGNORED SystemLayer().StartTimer(kAttachPollInterval, PollAttach, nullptr);
}

void HandleFailSafeEarlyExpiry(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kThreadStateChange:
        // ConnectNetwork is the first commissioning step that writes the OT active dataset
        // (AddOrUpdateThreadNetwork only stages it). The fail-safe revert also writes it,
        // but only after the fail-safe has been disarmed.
        // Assumption: nothing else writes the active dataset while a BLE commissioning fail-safe
        // is armed. A manual write (e.g. "otcli dataset commit active") would select case (ii)
        // instead of (i); the fail-safe is still expired, just after the attach attempts.
        if ((event->ThreadStateChange.OpenThread.Flags & OT_CHANGED_ACTIVE_DATASET) &&
            Server::GetInstance().GetFailSafeContext().IsFailSafeArmed())
        {
            sConnectNetworkReceived = true;
        }
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed: {
        auto & ble    = chip::DeviceLayer::Internal::BLEMgrImpl();
        uint8_t conId = static_cast<uint8_t>(event->CHIPoBLEConnectionError.ConId);

        VerifyOrReturn(ble.IsChipoBleConId(conId)); // ignore proprietary GATT links
        ble.ClearChipobleConId();
        VerifyOrReturn(IsBleCommissioningFailSafe());

        if (!sConnectNetworkReceived)
        {
            // Case (i): PASE over BLE was the only bearer, so it cannot be recovered.
            ExpireFailSafe("CHIPoBLE closed before ConnectNetwork");
            return;
        }

        // Already on the operational network: the commissioner may continue over CASE.
        VerifyOrReturn(!ConnectivityMgr().IsThreadAttached());

        // Case (ii)
        StartAttachWatch();
        break;
    }
    case DeviceEventType::kThreadConnectivityChange:
        if (sAttachWatch && event->ThreadConnectivityChange.Result == kConnectivity_Established)
        {
            ChipLogProgress(AppServer, "Fail-safe early expiry: Thread attached, cancelled");
            ResetEarlyExpiry();
        }
        break;

    case DeviceEventType::kFailSafeTimerExpired:
    case DeviceEventType::kCommissioningComplete:
        ResetEarlyExpiry();
        break;

    default:
        break;
    }
}
} // namespace
#endif // CHIP_APP_FAILSAFE_EARLY_EXPIRY

void chip::NXP::App::CommonDeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    ChipLogDetail(DeviceLayer, "DeviceEventCallback: 0x%04x", event->Type);
#if CHIP_APP_FAILSAFE_EARLY_EXPIRY
    HandleFailSafeEarlyExpiry(event);
#endif
    switch (event->Type)
    {
    case DeviceEventType::kWiFiConnectivityChange:
        OnWiFiConnectivityChange(event);
        break;

    case DeviceEventType::kInternetConnectivityChange:
        OnInternetConnectivityChange(event);
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
#if !CHIP_ENABLE_OPENTHREAD // No need to do this for OT mDNS server
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
#endif
        OnInterfaceIpAddressChanged(event);
        break;

#if CHIP_ENABLE_OPENTHREAD
    case DeviceEventType::kCommissioningComplete:
        CommonDeviceCallbacks::OnComissioningComplete(event);
        break;
#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    case DeviceEventType::kThreadConnectivityChange:
        if (!ConnectivityMgr().IsWiFiStationConnected() && (event->ThreadConnectivityChange.Result == kConnectivity_Established))
        {
            // Restart DnsSd service when operating as Matter over Thread
            chip::app::DnssdServer::Instance().StartServer();
        }
        break;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#endif // CHIP_ENABLE_OPENTHREAD
    case DeviceLayer::DeviceEventType::kDnssdInitialized:
        ChipLogProgress(DeviceLayer, "kDnssdInitialized");
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        /* Initialize OTA Requestor */
        OTARequestorInitiator::Instance().InitOTA(reinterpret_cast<intptr_t>(&OTARequestorInitiator::Instance()));
#endif
        break;
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnWiFiConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->WiFiConnectivityChange.Result == kConnectivity_Established)
    {
        ChipLogProgress(DeviceLayer, "WiFi connection established");
    }
    else if (event->WiFiConnectivityChange.Result == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "WiFi connection lost");
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnInternetConnectivityChange(const ChipDeviceEvent * event)
{
    if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
    {
        char ip_addr[Inet::IPAddress::kMaxStringLength];
        event->InternetConnectivityChange.ipAddress.ToString(ip_addr);
        ChipLogProgress(DeviceLayer, "Server ready at: %s:%d", ip_addr, CHIP_PORT);
#if !CHIP_ENABLE_OPENTHREAD // No need to do this for OT mDNS server
        chip::app::DnssdServer::Instance().StartServer();
#endif
    }
    else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv4 connectivity...");
    }
    if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
    {
        char ip_addr[Inet::IPAddress::kMaxStringLength];
        event->InternetConnectivityChange.ipAddress.ToString(ip_addr);
        ChipLogProgress(DeviceLayer, "IPv6 Server ready at: [%s]:%d", ip_addr, CHIP_PORT);

        chip::app::DnssdServer::Instance().StartServer();

#if CHIP_DEVICE_CONFIG_ENABLE_TBR
        // Start the TBRM cluster after we are connected to local network. At this point the Open Thread
        // Border Router management is up and running and the Border Router Service Name is created.
        GetAppTask().EnableTbrManagementCluster();
#endif
    }
    else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
    {
        ChipLogProgress(DeviceLayer, "Lost IPv6 connectivity...");
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnInterfaceIpAddressChanged(const ChipDeviceEvent * event)
{
    switch (event->InterfaceIpAddressChanged.Type)
    {
    case InterfaceIpChangeType::kIpV4_Assigned:
        ChipLogProgress(DeviceLayer, "Interface IPv4 address assigned");
        break;
    case InterfaceIpChangeType::kIpV4_Lost:
        ChipLogProgress(DeviceLayer, "Interface IPv4 address lost");
        break;
    case InterfaceIpChangeType::kIpV6_Assigned:
        ChipLogProgress(DeviceLayer, "Interface IPv6 address assigned");
        break;
    case InterfaceIpChangeType::kIpV6_Lost:
        ChipLogProgress(DeviceLayer, "Interface IPv6 address lost");
        break;
    }
}

void chip::NXP::App::CommonDeviceCallbacks::OnSessionEstablished(chip::DeviceLayer::ChipDeviceEvent const *)
{
    /* Empty */
}

#if CHIP_ENABLE_OPENTHREAD
void chip::NXP::App::CommonDeviceCallbacks::OnComissioningComplete(const chip::DeviceLayer::ChipDeviceEvent * event)
{
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    /*
     * If a transceiver supporting a multiprotocol scenario is used, a check of the provisioning state is required,
     * so that we can inform the transceiver to stop BLE to give the priority to another protocol.
     * For example it is the case when a K32W0 transceiver supporting OT+BLE+Zigbee is used. When the device is already provisioned,
     * BLE is no more required and the transceiver needs to be informed so that Zigbee can be switched on and BLE switched off.
     *
     * If a transceiver does not support such vendor property the cmd would be ignored.
     */
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogDetail(DeviceLayer, "Provisioning complete, stopping BLE\n");
        ThreadStackMgrImpl().LockThreadStack();
        PlatformMgrImpl().StopBLEConnectivity();
        ThreadStackMgrImpl().UnlockThreadStack();
    }
#endif
}
#endif
