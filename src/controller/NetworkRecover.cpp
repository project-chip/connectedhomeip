/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <controller/AutoNetworkRecover.h>
#include <controller/CHIPDeviceController.h>
#include <lib/support/CodeUtils.h>
#include <memory>
#include <system/SystemClock.h>
#include <tracing/metric_event.h>

using namespace chip::Tracing;

namespace chip {
namespace Controller {

#if CONFIG_NETWORK_LAYER_BLE
NetworkRecoverParameters::NetworkRecoverParameters(BLE_CONNECTION_OBJECT connObj)
{
    Transport::PeerAddress peerAddress = Transport::PeerAddress::BLE();
    SetPeerAddress(peerAddress);
    SetConnectionObject(connObj);
}
#endif // CONFIG_NETWORK_LAYER_BLE

NetworkRecoverParameters::NetworkRecoverParameters(const uint64_t recoveryId) : mRecoveryId(recoveryId)
{
    Transport::PeerAddress peerAddress = Transport::PeerAddress::BLE();
    SetPeerAddress(peerAddress);
}

CHIP_ERROR NetworkRecover::Discover(uint16_t timeout)
{
    VerifyOrReturnErrorWithMetric(kMetricNetworkRecover, mSystemLayer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mDiscoverTimeout         = false;
    mNetworkRecoverBehaviour = NetworkRecoverBehaviour::kDiscover;
    ReturnErrorOnFailureWithMetric(kMetricNetworkRecover, StartDiscoverOverBle(0));
    auto errorCode = mSystemLayer->StartTimer(System::Clock::Milliseconds32(timeout * chip::kMillisecondsPerSecond),
                                              OnRecoverableDiscoveredTimeoutCallback, this);
    if (CHIP_NO_ERROR == errorCode)
    {
        MATTER_LOG_METRIC_BEGIN(kMetricNetworkRecover);
    }
    return errorCode;
}

CHIP_ERROR NetworkRecover::Recover(NodeId remoteId, uint64_t recoveryId, WiFiCredentials wiFiCreds, uint64_t breadcrumb)
{
    mRemoteId = remoteId;
    mWiFiCreds.SetValue(wiFiCreds);
    mBreadcrumb              = breadcrumb;
    mNetworkRecoverBehaviour = NetworkRecoverBehaviour::kRecover;
    ReturnErrorOnFailureWithMetric(kMetricNetworkRecover, StartDiscoverOverBle(recoveryId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkRecover::Recover(NodeId remoteId, uint64_t recoveryId, ByteSpan operationalDataset, uint64_t breadcrumb)
{
    mRemoteId = remoteId;
    mOperationalDataset.SetValue(operationalDataset);
    mBreadcrumb              = breadcrumb;
    mNetworkRecoverBehaviour = NetworkRecoverBehaviour::kRecover;
    ReturnErrorOnFailureWithMetric(kMetricNetworkRecover, StartDiscoverOverBle(recoveryId));
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkRecover::StartDiscoverOverBle(uint64_t recoveryId)
{
    if (mNetworkRecoverBehaviour == NetworkRecoverBehaviour::kDiscover && mDiscoverTimeout)
    {
        // stop recoverable scanning
        return CHIP_NO_ERROR;
    }
#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mCommissioner != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mCommissioner->ConnectBleTransportToSelf();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ChipLogProgress(Controller, "Starting recoverable discovery over BLE");

    // Handle possibly-sync callbacks.
    CHIP_ERROR err = mBleLayer->NewBleConnectionByRecoveryIdentifier(recoveryId, this, OnDiscoveredDeviceOverBleConnected,
                                                                     OnDiscoveredDeviceOverBleError, OnDiscoveredDeviceOverBle);
    return err;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // CONFIG_NETWORK_LAYER_BLE
}

CHIP_ERROR NetworkRecover::StopConnectOverBle()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if CONFIG_NETWORK_LAYER_BLE
    VerifyOrReturnError(mBleLayer != nullptr, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    err = mBleLayer->CancelBleIncompleteConnection();
#endif // CONFIG_NETWORK_LAYER_BLE
    return err;
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
            ChipLogProgress(Controller, "Notify recoverable devices " ChipLogFormatX64, ChipLogValueX64(params.GetRecoveryId()));
            auto iter = std::find(mDiscoveredRecoveryIds.begin(), mDiscoveredRecoveryIds.end(), params.GetRecoveryId());
            if (iter == mDiscoveredRecoveryIds.end())
            {
                mDiscoveredRecoveryIds.push_back(params.GetRecoveryId());
            }
            StartDiscoverOverBle(0);
        }
        else
        {
            ChipLogProgress(Controller, "Starting network recovery");
#if CONFIG_NETWORK_LAYER_BLE
            err = mBleLayer->NewBleConnectionByObject(params.GetConnectionObject());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "Failed to connect to device: %" CHIP_ERROR_FORMAT, err.Format());
                NetworkRecoverComplete(mRemoteId, err);
                break;
            }
#endif // CONFIG_NETWORK_LAYER_BLE
            auto addr = params.GetPeerAddress();
            if (mWiFiCreds.HasValue())
            {
                err = AutoNetworkRecover::RecoverNetwork(this, mRemoteId, addr, mWiFiCreds, chip::NullOptional, mBreadcrumb);
            }
            else
            {
                err =
                    AutoNetworkRecover::RecoverNetwork(this, mRemoteId, addr, chip::NullOptional, mOperationalDataset, mBreadcrumb);
            }
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Controller, "Failed to recover device: %" CHIP_ERROR_FORMAT, err.Format());
                NetworkRecoverComplete(mRemoteId, err);
                break;
            }
        }
    }

    return true;
}

void NetworkRecover::NetworkRecoverDiscoverFinish()
{
    if (mNetworkRecoverDelegate != nullptr)
    {
        mNetworkRecoverDelegate->OnNetworkRecoverDiscover(mDiscoveredRecoveryIds);
    }
    mSystemLayer->CancelTimer(OnRecoverableDiscoveredTimeoutCallback, this);
    mDiscoveredRecoveryIds.clear();
}

void NetworkRecover::NetworkRecoverComplete(NodeId deviceId, CHIP_ERROR error)
{
    StopConnectOverBle();
    if (mNetworkRecoverDelegate != nullptr)
    {
        mNetworkRecoverDelegate->OnNetworkRecoverComplete(deviceId, error);
    }
}

void NetworkRecover::OnRecoverableDiscoveredTimeoutCallback(System::Layer * layer, void * context)
{
    ChipLogProgress(Controller, "Discovery timed out");
    auto * recover = static_cast<NetworkRecover *>(context);
    recover->NetworkRecoverDiscoverFinish();
    recover->mDiscoverTimeout = true;
    MATTER_LOG_METRIC_END(kMetricNetworkRecover, CHIP_NO_ERROR);
}

#if CONFIG_NETWORK_LAYER_BLE
void NetworkRecover::OnDiscoveredDeviceOverBle(BLE_CONNECTION_OBJECT connObj)
{
    ChipLogProgress(Controller, "Connected recoverable device over BLE");
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
    if (mNetworkRecoverBehaviour == NetworkRecoverBehaviour::kRecover)
    {
        // finish recover connection
        NetworkRecoverComplete(mRemoteId, err);
    }
    else
    {
        // BLE scan timeout, no recoverable found, scan again
        StartDiscoverOverBle(0);
    }
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
