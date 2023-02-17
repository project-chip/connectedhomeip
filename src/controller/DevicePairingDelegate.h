/*
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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <controller/CommissioningDelegate.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/DLLUtil.h>
#include <stdint.h>

namespace chip {
namespace Controller {

/**
 * A delegate that can be notified of progress as a "pairing" (which might mean
 * "PASE session establishment" or "commissioning") proceeds.
 */
class DLL_EXPORT DevicePairingDelegate
{
public:
    virtual ~DevicePairingDelegate() {}

    enum Status : uint8_t
    {
        SecurePairingSuccess = 0,
        SecurePairingFailed,
    };

    /**
     * @brief
     *   Called when the pairing reaches a certain stage.
     *
     * @param status Current status of pairing
     */
    virtual void OnStatusUpdate(DevicePairingDelegate::Status status) {}

    /**
     * @brief
     *   Called when PASE session establishment is complete (with success or error)
     *
     * @param error Error cause, if any
     */
    virtual void OnPairingComplete(CHIP_ERROR error) {}

    /**
     * @brief
     *   Called when the pairing is deleted (with success or error)
     *
     * @param error Error cause, if any
     */
    virtual void OnPairingDeleted(CHIP_ERROR error) {}

    /**
     *   Called when the commissioning process is complete (with success or error)
     */
    virtual void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) {}
    virtual void OnCommissioningSuccess(PeerId peerId) {}
    virtual void OnCommissioningFailure(PeerId peerId, CHIP_ERROR error, CommissioningStage stageFailed,
                                        Optional<Credentials::AttestationVerificationResult> additionalErrorInfo)
    {}

    virtual void OnCommissioningStatusUpdate(PeerId peerId, CommissioningStage stageCompleted, CHIP_ERROR error) {}

    /**
     * @brief
     *  Called with the ReadCommissioningInfo returned from the target
     */
    virtual void OnReadCommissioningInfo(const ReadCommissioningInfo & info) {}

    /**
     * @brief
     *  Called with the NetworkScanResponse returned from the target.
     *
     * The DeviceCommissioner will be waiting in the kNeedsNetworkCreds step and not advancing the commissioning process.
     *
     * The implementation should set the network credentials on the CommissioningParameters of the CommissioningDelegate
     * using CommissioningDelegate.SetCommissioningParameters(), and then call DeviceCommissioner.NetworkCredentialsReady()
     * in order to resume the commissioning process.
     */
    virtual void
    OnScanNetworksSuccess(const app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & dataResponse)
    {}

    /**
     * @brief
     *  Called when the NetworkScan request fails.
     *
     * The DeviceCommissioner will be waiting in the kNeedsNetworkCreds step and not advancing the commissioning process.
     *
     * The implementation should set the network credentials on the CommissioningParameters of the CommissioningDelegate
     * using CommissioningDelegate.SetCommissioningParameters(), and then call DeviceCommissioner.NetworkCredentialsReady()
     * in order to resume the commissioning process.
     */
    virtual void OnScanNetworksFailure(CHIP_ERROR error) {}
};

} // namespace Controller
} // namespace chip
