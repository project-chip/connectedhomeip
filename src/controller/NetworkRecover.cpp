/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Implementation of SetUp Code Pairer, a class that parses a given
 *      setup code and uses the extracted informations to discover and
 *      filter commissionables nodes, before initiating the pairing process.
 *
 */

#include <controller/NetworkRecover.h>

#include <controller/CHIPDeviceController.h>
#include <controller/AutoNetworkRecover.h>
#include <lib/support/CodeUtils.h>
#include <memory>
#include <system/SystemClock.h>
#include <tracing/metric_event.h>

constexpr uint32_t kDeviceDiscoveredTimeout = CHIP_CONFIG_SETUP_CODE_PAIRER_DISCOVERY_TIMEOUT_SECS * chip::kMillisecondsPerSecond;

using namespace chip::Tracing;

namespace chip {
namespace Controller {

namespace {
    static void OnNetworkRecoverFinish(void * context, NodeId remoteNodeId, CHIP_ERROR status)
    {
        auto * recover = static_cast<NetworkRecover *>(context);
        if (recover->GetNetworkRecoverDelegate() != nullptr)
        {
            recover->GetNetworkRecoverDelegate()->OnNetworkRecoverComplete(remoteNodeId, status);
        }
        else
        {

        }
    }
}

#if CONFIG_NETWORK_LAYER_BLE
NetworkRecoverParameters::NetworkRecoverParameters(BLE_CONNECTION_OBJECT connObj)
{
    Transport::PeerAddress peerAddress = Transport::PeerAddress::BLE();
    SetPeerAddress(peerAddress);
}
#endif // CONFIG_NETWORK_LAYER_BLE

NetworkRecoverParameters::NetworkRecoverParameters(const uint64_t recoveryId) : mRecoveryId(recoveryId)
{
    Transport::PeerAddress peerAddress = Transport::PeerAddress::BLE();
    SetPeerAddress(peerAddress);
}

CHIP_ERROR NetworkRecover::Discover(uint64_t recoveryId)
{
    mNetworkRecoverBehaviour = NetworkRecoverBehaviour::kDiscover;
    ReturnErrorOnFailureWithMetric(kMetricNetworkRecover, StartDiscoverOverBle(recoveryId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkRecover::Recover(NodeId remoteId, uint64_t recoveryId, WiFiCredentials wiFiCreds, uint64_t breadcrumb)
{
    mRemoteId = remoteId;
    mWiFiCreds.SetValue(wiFiCreds);
    mBreadcrumb = breadcrumb;
    mNetworkRecoverBehaviour = NetworkRecoverBehaviour::kRecover;
    ReturnErrorOnFailureWithMetric(kMetricNetworkRecover, StartDiscoverOverBle(recoveryId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkRecover::StartDiscoverOverBle(uint64_t recoveryId)
{
    #if CONFIG_NETWORK_LAYER_BLE
    #if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
        VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
        mCommissioner->ConnectBleTransportToSelf();
    #endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
        VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    
        ChipLogProgress(Controller, "Starting recoverable discovery over BLE");
    
        // Handle possibly-sync callbacks.
        CHIP_ERROR err =
         mBleLayer->NewBleConnectionByRecoveryIdentifier(recoveryId, this, 
                                                         OnDiscoveredDeviceOverBleConnected,
                                                         OnDiscoveredDeviceOverBleError, 
                                                         OnDiscoveredDeviceOverBle);
        return err;
    #else
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    #endif // CONFIG_NETWORK_LAYER_BLE
}

bool NetworkRecover::NotifyOrConnectToDiscoveredDevice()
{
    while (!mNetworkRecoverParameters.empty())
    {
        // Grab the first element from the queue and try connecting to it.
        // Remove it from the queue before we try to connect, in case the
        // connection attempt fails and calls right back into us to try the next
        // thing.
        NetworkRecoverParameters params(mNetworkRecoverParameters.front());
        mNetworkRecoverParameters.pop_front();

#if CHIP_PROGRESS_LOGGING
        char buf[Transport::PeerAddress::kMaxToStringSize];
        params.GetPeerAddress().ToString(buf);
        ChipLogProgress(Controller, "Found recoverable device %s", buf);
#endif // CHIP_PROGRESS_LOGGING

        CHIP_ERROR err = CHIP_NO_ERROR;
        if (mNetworkRecoverBehaviour == NetworkRecoverBehaviour::kDiscover)
        {
            // notify to application
            ChipLogProgress(Controller, "Notify recoverable devices:%lu", params.GetRecoveryId());
            if (mNetworkRecoverDelegate != nullptr)
            {
                mNetworkRecoverDelegate->OnNetworkRecoverDiscover(params.GetRecoveryId());
            }
            else
            {
                ChipLogError(Controller, "Network recover delegate is null");
            }
        }
        else
        {
            ChipLogProgress(Controller, "Start to network recovery");
            err = AutoNetworkRecover::RecoverNetwork(mCommissioner, mRemoteId, params, mWiFiCreds.Value(),
                                               mBreadcrumb, OnNetworkRecoverFinish);
        }

        LogErrorOnFailure(err);
        if (err == CHIP_NO_ERROR)
        {
            return true;
        }
    }

    return true;
}

#if CONFIG_NETWORK_LAYER_BLE
void NetworkRecover::OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj)
{
    ChipLogProgress(Controller, "Connectted recoverable device over BLE");
    // In order to not wait for all the possible addresses discovered over mdns to
    // be tried before trying to connect over BLE, the discovered connection object is
    // inserted at the beginning of the list.
    //
    // It makes it the 'next' thing to try to connect to if there are already some
    // discovered parameters in the list.
    mNetworkRecoverParameters.emplace_front(connObj);
    NotifyOrConnectToDiscoveredDevice();
}
void NetworkRecover::OnDiscoveredDeviceOverBle(uint64_t recoveryId)
{
    ChipLogProgress(Controller, "Discovered recoverable device over BLE");
    // In order to not wait for all the possible addresses discovered over mdns to
    // be tried before trying to connect over BLE, the discovered connection object is
    // inserted at the beginning of the list.
    //
    // It makes it the 'next' thing to try to connect to if there are already some
    // discovered parameters in the list.
    mNetworkRecoverParameters.emplace_front(recoveryId);
    NotifyOrConnectToDiscoveredDevice();
}

void NetworkRecover::OnBLEDiscoveryError(CHIP_ERROR err)
{
    ChipLogError(Controller, "discovery over BLE failed: %" CHIP_ERROR_FORMAT, err.Format());
    LogErrorOnFailure(err);
}

void NetworkRecover::OnDiscoveredDeviceOverBle(void * appState, uint64_t recoveryId)
{
    (static_cast<NetworkRecover *>(appState))->OnDiscoveredDeviceOverBle(recoveryId);
}

void NetworkRecover::OnDiscoveredDeviceOverBleConnected(void * appState, BLE_CONNECTION_OBJECT connObj)
{
    (static_cast<NetworkRecover *>(appState))->OnDiscoveredDeviceOverBle(connObj);
}

void NetworkRecover::OnDiscoveredDeviceOverBleError(void * appState, CHIP_ERROR err)
{
    static_cast<NetworkRecover *>(appState)->OnBLEDiscoveryError(err);
}
#endif // CONFIG_NETWORK_LAYER_BLE

} // namespace Controller
} // namespace chip