/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *    @file
 *          Contains non-inline method definitions for the
 *          GenericNetworkProvisioningServerImpl<> template.
 */

#ifndef GENERIC_NETWORK_PROVISIONING_SERVER_IMPL_IPP
#define GENERIC_NETWORK_PROVISIONING_SERVER_IMPL_IPP

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericNetworkProvisioningServerImpl.h>
#include <platform/internal/NetworkProvisioningServer.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ThreadStackManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace ::chip::TLV;

using Profiles::kChipProfile_Common;
using Profiles::kChipProfile_NetworkProvisioning;

namespace {

const char sLogPrefix[] = "NetworkProvisioningServer: ";

} // unnamed namespace

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericNetworkProvisioningServerImpl<NetworkProvisioningServerImpl>;

template <class ImplClass>
void GenericNetworkProvisioningServerImpl<ImplClass>::_StartPendingScan()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Do nothing if there's no pending ScanNetworks request outstanding, or if a scan is already in progress.
    if (mState != kState_ScanNetworks_Pending)
    {
        ExitNow();
    }

    switch (mScanNetworkType)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    case kNetworkType_WiFi:

        // Defer the scan if the Connection Manager says the system is in a state
        // where a WiFi scan cannot be started (e.g. if the system is connecting to
        // an AP and can't scan and connect at the same time). The Connection Manager
        // is responsible for calling this method again when the system is read to scan.
        if (!ConnectivityMgr().CanStartWiFiScan())
        {
            ExitNow();
        }

        mState = kState_ScanNetworks_InProgress;

        // Delegate to the implementation subclass to initiate the WiFi scan operation.
        err = Impl()->InitiateWiFiScan();
        SuccessOrExit(err);

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    case kNetworkType_Thread:

        // TODO: implement this
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    default:
        ExitNow(err = CHIP_ERROR_INCORRECT_STATE);
    }

exit:
    // If an error occurred, send a Internal Error back to the requestor.
    if (err != CHIP_NO_ERROR)
    {
        SendStatusReport(kChipProfile_Common, kStatus_InternalError, err);
        mState = kState_Idle;
    }
}

template <class ImplClass>
void GenericNetworkProvisioningServerImpl<ImplClass>::_OnPlatformEvent(const ChipDeviceEvent * event)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    // Handle a change in Internet connectivity...
    if (event->Type == DeviceEventType::kInternetConnectivityChange)
    {
        // If a TestConnectivity operation is in progress for WiFi, re-evaluate the state
        // connectivity now.
        ContinueWiFiConnectivityTest();
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Define some short-hands for various interesting event conditions.
    const bool threadConnChanged = (event->Type == DeviceEventType::kThreadConnectivityChange &&
                                    event->ThreadConnectivityChange.Result != kConnectivity_NoChange);
    const bool threadRoleChanged = (event->Type == DeviceEventType::kThreadStateChange && event->ThreadStateChange.RoleChanged);
    const bool threadChildrenChanged =
        (event->Type == DeviceEventType::kThreadStateChange && event->ThreadStateChange.ChildNodesChanged);

    // If the state of the Thread interface changed, OR if the Thread role changed, OR
    // if there was a change to the set of child nodes...
    if (threadConnChanged || threadRoleChanged || threadChildrenChanged)
    {
        // If a TestConnectivity operation is in progress for Thread, re-evaluate the state
        // connectivity now.
        ContinueThreadConnectivityTest();
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleScanNetworks(uint8_t networkType)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    bool isAppControlled = false;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    switch (networkType)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    case kNetworkType_WiFi:
        isAppControlled = ConnectivityMgr().IsWiFiStationApplicationControlled();
        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    case kNetworkType_Thread:
        isAppControlled = ConnectivityMgr().IsThreadApplicationControlled();
        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    default:
        err = SendStatusReport(kChipProfile_NetworkProvisioning, kStatusCode_UnsupportedNetworkType);
        ExitNow();
    }

    // Reject the request if the application is currently in control of the requested network.
    if (isAppControlled)
    {
        err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
        ExitNow();
    }

    // Enter the ScanNetworks Pending state and delegate to the implementation class to start the scan.
    mState           = kState_ScanNetworks_Pending;
    mScanNetworkType = networkType;
    Impl()->StartPendingScan();

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::DoInit()
{
    CHIP_ERROR err;

    // Call init on the server base class.
    err = ServerBaseClass::Init(&::chip::DeviceLayer::ExchangeMgr);
    SuccessOrExit(err);

    // Set the pointer to the delegate object.
    SetDelegate(this);

    mState           = kState_Idle;
    mScanNetworkType = kNetworkType_NotSpecified;

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleAddNetwork(PacketBuffer * networkInfoTLV)
{
    return HandleAddUpdateNetwork(networkInfoTLV, false);
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleUpdateNetwork(PacketBuffer * networkInfoTLV)
{
    return HandleAddUpdateNetwork(networkInfoTLV, true);
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleAddUpdateNetwork(PacketBuffer * networkInfoTLV, bool isUpdate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceNetworkInfo netInfo;
    uint32_t netId;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    // Parse the supplied network configuration info.
    {
        TLV::TLVReader reader;
        reader.Init(networkInfoTLV);
        err = netInfo.Decode(reader);
        SuccessOrExit(err);
    }

    // Discard the request buffer.
    PacketBuffer::Free(networkInfoTLV);
    networkInfoTLV = NULL;

    switch (netInfo.NetworkType)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    case kNetworkType_WiFi:

        // If updating the provision, verify that the specified network is provisioned.
        if (isUpdate && !ConnectivityMgr().IsWiFiStationProvisioned())
        {
            err = SendStatusReport(kChipProfile_NetworkProvisioning, kStatusCode_UnknownNetwork);
            ExitNow();
        }

        // Reject the request if the application is currently in control of the WiFi station.
        if (ConnectivityMgr().IsWiFiStationApplicationControlled())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // If updating the provision...
        if (isUpdate)
        {
            DeviceNetworkInfo existingNetInfo;

            // Delegate to the implementation subclass to get the existing station provision.
            err = Impl()->GetWiFiStationProvision(existingNetInfo, true);
            SuccessOrExit(err);

            // Override the existing provision with values specified in the update.
            err = netInfo.MergeTo(existingNetInfo);
            SuccessOrExit(err);
            memcpy(&netInfo, &existingNetInfo, sizeof(existingNetInfo));
        }

        // Check the validity of the new provision. If not acceptable, respond to the requestor
        // with an appropriate StatusReport.
        {
            uint32_t statusProfileId;
            uint16_t statusCode;
            err = Impl()->ValidateWiFiStationProvision(netInfo, statusProfileId, statusCode);
            if (err != CHIP_NO_ERROR)
            {
                err = SendStatusReport(statusProfileId, statusCode, err);
                ExitNow();
            }
        }

        // If the WiFi station is not already configured, disable the station interface.  This
        // ensures that the device will not automatically connect to the new network until an
        // EnableNetwork request is received.
        if (!ConnectivityMgr().IsWiFiStationProvisioned())
        {
            err = ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);
            SuccessOrExit(err);
        }

        // Delegate to the implementation subclass to set the WiFi station provision.
        err = Impl()->SetWiFiStationProvision(netInfo);
        SuccessOrExit(err);

        // Tell the ConnectivityManager there's been a change to the station provision.
        ConnectivityMgr().OnWiFiStationProvisionChange();

        netId = kWiFiStationNetworkId;

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    case kNetworkType_Thread:

        // If updating the provision, verify that the Thread network is provisioned.
        if (isUpdate && !ThreadStackMgr().IsThreadProvisioned())
        {
            err = SendStatusReport(kChipProfile_NetworkProvisioning, kStatusCode_UnknownNetwork);
            ExitNow();
        }

        // Reject the request if the application is currently in control of the Thread network.
        if (ConnectivityMgr().IsThreadApplicationControlled())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // Check the validity of the supplied Thread parameters. If not acceptable, respond to
        // the requestor with an appropriate StatusReport.
        {
            uint32_t statusProfileId;
            uint16_t statusCode;
            err = Impl()->ValidateThreadProvision(isUpdate, netInfo, statusProfileId, statusCode);
            if (err != CHIP_NO_ERROR)
            {
                err = SendStatusReport(statusProfileId, statusCode, err);
                ExitNow();
            }
        }

        // Apply suitable defaults for any parameters not supplied by the client.
        err = Impl()->SetThreadProvisionDefaults(isUpdate, netInfo);
        SuccessOrExit(err);

        // Store the Thread provision.
        err = ThreadStackMgr().SetThreadProvision(netInfo);
        SuccessOrExit(err);

        netId = kThreadNetworkId;

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    default:

        ChipLogProgress(DeviceLayer, "%sUnsupported network type: %d", sLogPrefix, netInfo.NetworkType);
        err = SendStatusReport(kChipProfile_NetworkProvisioning, kStatusCode_UnsupportedNetworkType, CHIP_ERROR_INVALID_ARGUMENT);
        ExitNow();
    }

    // Send an AddNetworkComplete message back to the requestor.
    SendAddNetworkComplete(netId);

exit:
    PacketBuffer::Free(networkInfoTLV);
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleRemoveNetwork(uint32_t networkId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    switch (networkId)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    case kWiFiStationNetworkId:

        // Verify that the specified network exists.
        if (!ConnectivityMgr().IsWiFiStationProvisioned())
        {
            goto sendUnknownNetworkResp;
        }

        // Reject the request if the application is currently in control of the WiFi station.
        if (ConnectivityMgr().IsWiFiStationApplicationControlled())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // Delegate to the implementation subclass to clear the WiFi station provision.
        err = Impl()->ClearWiFiStationProvision();
        SuccessOrExit(err);

        // Tell the ConnectivityManager there's been a change to the station provision.
        ConnectivityMgr().OnWiFiStationProvisionChange();

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    case kThreadNetworkId:

        // Verify that the specified network exists.
        if (!ThreadStackMgr().IsThreadProvisioned())
        {
            goto sendUnknownNetworkResp;
        }

        // Reject the request if the application is currently in control of the Thread network.
        if (ConnectivityMgr().IsThreadApplicationControlled())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // Clear the Thread provision.
        ThreadStackMgr().ErasePersistentInfo();

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    default:
    sendUnknownNetworkResp:

        err = SendStatusReport(kChipProfile_NetworkProvisioning, kStatusCode_UnknownNetwork);
        ExitNow();
    }

    // Respond with a Success response.
    err = SendSuccessResponse();
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleGetNetworks(uint8_t flags)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceNetworkInfo netInfo[2];
    PacketBuffer * respBuf        = NULL;
    uint8_t resultCount           = 0;
    const bool includeCredentials = (flags & kGetNetwork_IncludeCredentials) != 0;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    // Delegate to the implementation subclass to get the WiFi station provision.
    err = Impl()->GetWiFiStationProvision(netInfo[resultCount], includeCredentials);
    if (err == CHIP_NO_ERROR)
    {
        resultCount++;
    }
    else if (err != CHIP_ERROR_INCORRECT_STATE)
    {
        ExitNow();
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Delegate to the implementation subclass to get the Thread provision.
    err = ThreadStackMgr().GetThreadProvision(netInfo[resultCount], includeCredentials);
    if (err == CHIP_NO_ERROR)
    {
        resultCount++;
    }
    else if (err != CHIP_ERROR_INCORRECT_STATE)
    {
        ExitNow();
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Allocate a buffer to hold the response.
    respBuf = PacketBuffer::New();
    VerifyOrExit(respBuf != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Encode the GetNetworks response data.
    {
        TLVWriter writer;

        writer.Init(respBuf);

        err = DeviceNetworkInfo::EncodeArray(writer, netInfo, resultCount);
        SuccessOrExit(err);

        err = writer.Finalize();
        SuccessOrExit(err);
    }

    // Send the response.
    err     = SendGetNetworksComplete(resultCount, respBuf);
    respBuf = NULL;
    SuccessOrExit(err);

exit:
    PacketBuffer::Free(respBuf);
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleEnableNetwork(uint32_t networkId)
{
    return HandleEnableDisableNetwork(networkId, true);
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleDisableNetwork(uint32_t networkId)
{
    return HandleEnableDisableNetwork(networkId, false);
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleEnableDisableNetwork(uint32_t networkId, bool enable)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    switch (networkId)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    case kWiFiStationNetworkId:

        // Verify that the specified network exists.
        if (!ConnectivityMgr().IsWiFiStationProvisioned())
        {
            goto sendUnknownNetworkResp;
        }

        // Reject the request if the application is currently in control of the WiFi station.
        if (ConnectivityMgr().IsWiFiStationApplicationControlled())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // Tell the ConnectivityManager to enable/disable the WiFi station interface.
        // Note that any effects of chaning the WiFi station mode happen asynchronously with this call.
        err = ConnectivityMgr().SetWiFiStationMode((enable) ? ConnectivityManager::kWiFiStationMode_Enabled
                                                            : ConnectivityManager::kWiFiStationMode_Disabled);
        SuccessOrExit(err);

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    case kThreadNetworkId:

        // Verify that the specified network exists.
        if (!ThreadStackMgr().IsThreadProvisioned())
        {
            goto sendUnknownNetworkResp;
        }

        // Reject the request if the application is currently in control of the Thread network.
        if (ConnectivityMgr().IsThreadApplicationControlled())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // Tell the ConnectivityManager to enable/disable the Thread network interface.
        // Note that any effects of changing the Thread mode happen asynchronously with this call.
        err = ThreadStackMgr().SetThreadEnabled(enable);
        SuccessOrExit(err);

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    default:
    sendUnknownNetworkResp:

        err = SendStatusReport(kChipProfile_NetworkProvisioning, kStatusCode_UnknownNetwork);
        ExitNow();
    }

    // Respond with a Success response.
    err = SendSuccessResponse();
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleTestConnectivity(uint32_t networkId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    switch (networkId)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    case kWiFiStationNetworkId:

        // Verify that the specified network exists.
        if (!ConnectivityMgr().IsWiFiStationProvisioned())
        {
            goto sendUnknownNetworkResp;
        }

        // Reject the request if the application is currently in control of the WiFi station.
        if (ConnectivityMgr().IsWiFiStationApplicationControlled())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // Tell the ConnectivityManager to enable the WiFi station interface if it hasn't been done already.
        // Note that any effects of enabling the WiFi station interface (e.g. connecting to an AP) happen
        // asynchronously with this call.
        err = ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);
        SuccessOrExit(err);

        // Record that we're waiting for the WiFi station interface to establish connectivity
        // with the Internet and arm a timer that will generate an error if connectivity isn't established
        // within a certain amount of time.
        mState = kState_TestConnectivity_WaitWiFiConnectivity;
        SystemLayer.StartTimer(CHIP_DEVICE_CONFIG_WIFI_CONNECTIVITY_TIMEOUT, HandleConnectivityTestTimeOut, NULL);

        // Go check for connectivity now.
        Impl()->ContinueWiFiConnectivityTest();

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    case kThreadNetworkId:

        // Verify that the specified network exists.
        if (!ThreadStackMgr().IsThreadProvisioned())
        {
            goto sendUnknownNetworkResp;
        }

        // If the Thread interface is NOT already enabled...
        if (!ThreadStackMgr().IsThreadEnabled())
        {
            // Reject the request if the application is currently in control of the Thread network.
            if (ConnectivityMgr().IsThreadApplicationControlled())
            {
                err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
                ExitNow();
            }

            // Enable the Thread interface.
            err = ThreadStackMgr().SetThreadEnabled(true);
            SuccessOrExit(err);
        }

        // Record that we're waiting to establish Thread connectivity and arm a timer that will
        // generate an error if connectivity isn't established within a certain amount of time.
        mState = kState_TestConnectivity_WaitThreadConnectivity;
        SystemLayer.StartTimer(CHIP_DEVICE_CONFIG_THREAD_CONNECTIVITY_TIMEOUT, HandleConnectivityTestTimeOut, NULL);

        // Go check for connectivity now.
        Impl()->ContinueThreadConnectivityTest();

        break;

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    default:
    sendUnknownNetworkResp:

        err = SendStatusReport(kChipProfile_NetworkProvisioning, kStatusCode_UnknownNetwork);
        ExitNow();
    }

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::HandleSetRendezvousMode(uint16_t rendezvousMode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == kState_Idle, err = CHIP_ERROR_INCORRECT_STATE);

    // Fail with Common:UnsupportedMessage if any unsupported modes were specified.
    {
        const uint16_t kSupportedModes = 0
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
            | kRendezvousMode_EnableWiFiRendezvousNetwork
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
            | kRendezvousMode_EnableThreadRendezvous
#endif
            ;

        if ((rendezvousMode & ~kSupportedModes) != 0)
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_UnsupportedMessage);
            ExitNow();
        }
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

    // If the request is to start the WiFi "rendezvous network" (a.k.a. the WiFi AP interface)...
    if ((rendezvousMode & kRendezvousMode_EnableWiFiRendezvousNetwork) != 0)
    {
        // Reject the request if the application is currently in control of the WiFi AP
        // OR if the AP interface has been expressly disabled by the application.
        const ConnectivityManager::WiFiAPMode apMode = ConnectivityMgr().GetWiFiAPMode();
        if (apMode == ConnectivityManager::kWiFiAPMode_ApplicationControlled || apMode == ConnectivityManager::kWiFiAPMode_Disabled)
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // Otherwise, request the ConnectivityManager to demand start the WiFi AP interface.
        // If the interface is already active this will have no immediate effect, except if the
        // interface is in the "demand" mode, in which case this will serve to extend the
        // active time.
        ConnectivityMgr().DemandStartWiFiAP();
    }

    // Otherwise the request is to stop the WiFi rendezvous network, so request the ConnectivityManager
    // to stop the AP interface if it has been demand started.  This will have no effect if the
    // interface is already stopped, or if the application has expressly enabled the interface.
    else
    {
        ConnectivityMgr().StopOnDemandWiFiAP();
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // If the request is to enable Thread "rendezvous"--i.e. enable Thread joinable mode...
    if ((rendezvousMode & kRendezvousMode_EnableWiFiRendezvousNetwork) != 0)
    {
        // Reject the request if the application is currently in control of the Thread Network
        // OR if the Thread interface has been expressly disabled by the application
        // OR if the Thread network has not been provisioned.
        const ConnectivityManager::ThreadMode threadMode = ConnectivityMgr().GetThreadMode();
        if (threadMode == ConnectivityManager::kThreadMode_ApplicationControlled ||
            threadMode == ConnectivityManager::kThreadMode_Disabled || !ThreadStackMgr().IsThreadProvisioned())
        {
            err = SendStatusReport(kChipProfile_Common, kStatus_NotAvailable);
            ExitNow();
        }

        // TODO: implement this
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    // Otherwise the request is to stop the Thread rendezvous...
    else
    {

        // TODO: implement this
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Respond with a Success response.
    err = SendSuccessResponse();
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
bool GenericNetworkProvisioningServerImpl<ImplClass>::IsPairedToAccount() const
{
    return ConfigurationMgr().IsServiceProvisioned() && ConfigurationMgr().IsPairedToAccount();
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::ValidateWiFiStationProvision(const DeviceNetworkInfo & netInfo,
                                                                                         uint32_t & statusProfileId,
                                                                                         uint16_t & statusCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (netInfo.NetworkType != kNetworkType_WiFi)
    {
        ChipLogProgress(DeviceLayer, "%sUnsupported WiFi station network type: %d", sLogPrefix, netInfo.NetworkType);
        statusProfileId = kChipProfile_NetworkProvisioning;
        statusCode      = kStatusCode_UnsupportedNetworkType;
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (netInfo.WiFiSSID[0] == 0)
    {
        ChipLogProgress(DeviceLayer, "%sMissing WiFi station SSID", sLogPrefix);
        statusProfileId = kChipProfile_NetworkProvisioning;
        statusCode      = kStatusCode_InvalidNetworkConfiguration;
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (netInfo.WiFiMode != kWiFiMode_Managed)
    {
        if (netInfo.WiFiMode == kWiFiMode_NotSpecified)
        {
            ChipLogProgress(DeviceLayer, "%sMissing WiFi station mode", sLogPrefix);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "%sUnsupported WiFi station mode: %d", sLogPrefix, netInfo.WiFiMode);
        }
        statusProfileId = kChipProfile_NetworkProvisioning;
        statusCode      = kStatusCode_InvalidNetworkConfiguration;
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (netInfo.WiFiRole != kWiFiRole_Station)
    {
        if (netInfo.WiFiRole == kWiFiRole_NotSpecified)
        {
            ChipLogProgress(DeviceLayer, "%sMissing WiFi station role", sLogPrefix);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "%sUnsupported WiFi station role: %d", sLogPrefix, netInfo.WiFiRole);
        }
        statusProfileId = kChipProfile_NetworkProvisioning;
        statusCode      = kStatusCode_InvalidNetworkConfiguration;
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Defer to the implementation class to determine if the proposed security type is supported.
    if (!ImplClass::IsSupportedWiFiSecurityType(netInfo.WiFiSecurityType))
    {
        ChipLogProgress(DeviceLayer, "%sUnsupported WiFi station security type: %d", sLogPrefix, netInfo.WiFiSecurityType);
        statusProfileId = kChipProfile_NetworkProvisioning;
        statusCode      = kStatusCode_UnsupportedWiFiSecurityType;
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (netInfo.WiFiSecurityType != kWiFiSecurityType_None && netInfo.WiFiKeyLen == 0)
    {
        ChipLogProgress(DeviceLayer, "%sMissing WiFi Key", sLogPrefix);
        statusProfileId = kChipProfile_NetworkProvisioning;
        statusCode      = kStatusCode_InvalidNetworkConfiguration;
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

exit:
    return err;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

template <class ImplClass>
CHIP_ERROR
GenericNetworkProvisioningServerImpl<ImplClass>::ValidateThreadProvision(bool isUpdate, const DeviceNetworkInfo & netInfo,
                                                                         uint32_t & statusProfileId, uint16_t & statusCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Verify a valid Thread channel was specified.
    if (netInfo.ThreadChannel != kThreadChannel_NotSpecified)
    {
        if (netInfo.ThreadChannel < 11 && netInfo.ThreadChannel > 26)
        {
            statusProfileId = kChipProfile_NetworkProvisioning;
            statusCode      = kStatusCode_InvalidNetworkConfiguration;
            ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
        }
    }

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericNetworkProvisioningServerImpl<ImplClass>::SetThreadProvisionDefaults(bool isUpdate, DeviceNetworkInfo & netInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Generate unique values for any Thread parameters not supplied by the client.

    // If extended PAN id was not specified, generate a random one.
    if (!netInfo.FieldPresent.ThreadExtendedPANId)
    {
        err = Platform::Security::GetSecureRandomData(netInfo.ThreadExtendedPANId, NetworkInfo::kThreadExtendedPANIdLength);
        SuccessOrExit(err);
        netInfo.FieldPresent.ThreadExtendedPANId = true;
    }

    // If network name was not specified, generate a default one.  If the device is a member of a
    // Chip fabric, base part of the name on the fabric id.
    if (netInfo.ThreadNetworkName[0] == 0)
    {
        uint16_t nameSuffix = (::chip::DeviceLayer::FabricState.FabricId != kFabricIdNotSpecified)
            ? (uint16_t)::chip::DeviceLayer::FabricState.FabricId
            : Encoding::BigEndian::Get16(&netInfo.ThreadExtendedPANId[6]);
        snprintf(netInfo.ThreadNetworkName, sizeof(netInfo.ThreadNetworkName), "%s%04X",
                 CHIP_DEVICE_CONFIG_DEFAULT_THREAD_NETWORK_NAME_PREFIX, nameSuffix);
    }

    // If a mesh prefix was not specified, generate one based on the extended PAN id.
    if (!netInfo.FieldPresent.ThreadMeshPrefix)
    {
        memset(netInfo.ThreadMeshPrefix, 0, sizeof(netInfo.ThreadMeshPrefix));
        netInfo.ThreadMeshPrefix[0] = 0xFD; // IPv6 ULA prefix
        memcpy(&netInfo.ThreadMeshPrefix[1], netInfo.ThreadExtendedPANId, 5);
        netInfo.FieldPresent.ThreadMeshPrefix = true;
    }

    // If network key was not specified, generate a random key.
    if (!netInfo.FieldPresent.ThreadNetworkKey)
    {
        err = Platform::Security::GetSecureRandomData(netInfo.ThreadNetworkKey, NetworkInfo::kThreadNetworkKeyLength);
        SuccessOrExit(err);
        netInfo.FieldPresent.ThreadNetworkKey = true;
    }

    // If a PSKc was not specified, generate a random PSKc.
    if (!netInfo.FieldPresent.ThreadPSKc)
    {
        err = Platform::Security::GetSecureRandomData(netInfo.ThreadPSKc, NetworkInfo::kThreadPSKcLength);
        SuccessOrExit(err);
        netInfo.FieldPresent.ThreadPSKc = true;
    }

    // If a PAN Id was not specified, generate a random Id.
    if (netInfo.ThreadPANId == kThreadPANId_NotSpecified)
    {
        uint16_t randPANId;
        err = Platform::Security::GetSecureRandomData((uint8_t *) &randPANId, sizeof(randPANId));
        SuccessOrExit(err);
        netInfo.ThreadPANId = randPANId;
    }

    // If Thread channel not present, choose a random one.
    if (netInfo.ThreadChannel == kThreadChannel_NotSpecified)
    {
        err = Platform::Security::GetSecureRandomData((uint8_t *) &netInfo.ThreadChannel, sizeof(netInfo.ThreadChannel));
        SuccessOrExit(err);
        netInfo.ThreadChannel = (netInfo.ThreadChannel % 0xF) + 11; // Convert value to 11 thru 26
    }

exit:
    return err;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

template <class ImplClass>
void GenericNetworkProvisioningServerImpl<ImplClass>::ContinueWiFiConnectivityTest(void)
{
    // If waiting for Internet connectivity to be established ...
    if (mState == kState_TestConnectivity_WaitWiFiConnectivity)
    {
        // Check for IPv4 Internet connectivity.  If available...
        if (ConnectivityMgr().HaveIPv4InternetConnectivity())
        {
            // TODO: perform positive test of connectivity to the Internet by pinging/connecting to
            // a well-known external server.

            // Send a Success result to the client.
            HandleConnectivityTestSuccess();
        }

        // Otherwise, arrange to return an appropriate error when the connectivity test times out.
        else
        {
            // TODO: Elaborate on the nature of the connectivity failure.  Ideally the status
            // code would distinguish the following types of failures:
            //     - Inability to connect to the local WiFi AP
            //     - Lack of a suitable local address (RFC1918 for IPv4; Global address for IPv6)
            //     - Lack of a default router
            //     - Lack of a DNS server
            //     - Inability to contact an external server.
            mTestConnectivityResult.mStatusProfileId = kChipProfile_NetworkProvisioning;
            mTestConnectivityResult.mStatusCode      = kStatusCode_TestNetworkFailed;
        }
    }
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

template <class ImplClass>
void GenericNetworkProvisioningServerImpl<ImplClass>::ContinueThreadConnectivityTest(void)
{
    // If waiting for Thread connectivity to be established...
    if (mState == kState_TestConnectivity_WaitThreadConnectivity)
    {
        // Check for connectivity to the Thread mesh.  In this context, connectivity means
        // that this node knows of (i.e. has in its Thread neighbor table) another node
        // which is acting as a Thread router. If connectivity exists, send a Success
        // result to the client.
        if (ThreadStackMgr().HaveMeshConnectivity())
        {
            // TODO: perform positive test of connectivity to peer router node.

            // Send a Success result to the client.
            HandleConnectivityTestSuccess();
        }

        // If connectivity doesn't exist arrange to return an appropriate error when the
        // connectivity test times out.
        else
        {
            // TODO: Elaborate on the nature of the connectivity failure.  Ideally the status
            // code would distinguish the following types of failures:
            //     - Lack of a peer router detected in the Mesh (for end nodes, this means
            //       lack of a parent router).
            //     - Inability to contact/ping the peer router.
            mTestConnectivityResult.mStatusProfileId = kChipProfile_NetworkProvisioning;
            mTestConnectivityResult.mStatusCode      = kStatusCode_NoRouterAvailable;
        }
    }
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_THREAD

template <class ImplClass>
void GenericNetworkProvisioningServerImpl<ImplClass>::HandleConnectivityTestSuccess(void)
{
    // Reset the state.
    mState = kState_Idle;
    SystemLayer.CancelTimer(HandleConnectivityTestTimeOut, NULL);

    // Verify that the TestConnectivity request is still outstanding and if so,
    // send a Success response to the client
    if (GetCurrentOp() == kMsgType_TestConnectivity)
    {
        SendSuccessResponse();
    }
}

template <class ImplClass>
void GenericNetworkProvisioningServerImpl<ImplClass>::HandleConnectivityTestTimeOut(::chip::System::Layer * aLayer,
                                                                                    void * aAppState, ::chip::System::Error aError)
{
    GenericNetworkProvisioningServerImpl<ImplClass> * self = &NetworkProvisioningSvrImpl();

    if (self->mState == kState_TestConnectivity_WaitWiFiConnectivity ||
        self->mState == kState_TestConnectivity_WaitThreadConnectivity)
    {
        const bool testingWiFi = (self->mState == kState_TestConnectivity_WaitWiFiConnectivity);

        ChipLogProgress(DeviceLayer, "%sTime out waiting for %s connectivity", sLogPrefix, (testingWiFi) ? "Internet" : "Thread");

        // Reset the state.
        self->mState = kState_Idle;
        SystemLayer.CancelTimer(HandleConnectivityTestTimeOut, NULL);

        // Verify that the TestConnectivity request is still outstanding; if so, send a StatusReport
        // to the client contain an appropriate error.
        if (self->GetCurrentOp() == kMsgType_TestConnectivity)
        {
            self->SendStatusReport(self->mTestConnectivityResult.mStatusProfileId, self->mTestConnectivityResult.mStatusCode);
        }
    }
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_THREAD

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_NETWORK_PROVISIONING_SERVER_IMPL_IPP
