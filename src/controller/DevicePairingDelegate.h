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
#include <protocols/secure_channel/RendezvousParameters.h>
#include <setup_payload/SetupPayload.h>
#include <stdint.h>

#include <optional>

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
     *   Called when PASE session establishment is complete (with success or error)
     *
     * @param error Error cause, if any
     *
     * @param rendezvousParameters The RendezvousParameters that were used for PASE establishment.
     *                             If available, this helps identify which exact commissionee PASE
     *                             was established for. This will generally be present only when
     *                             PASE establishment succeeds.
     *
     * @param setupPayload The SetupPayload that was used for PASE establishment, if one is
     *                     available.  This will generally be present only when PASE establishment
     *                     succeeds and the original input to commissioning was a payload string.
     *                     If the original input represented a concatenated QR code, this will
     *                     represent the actual payload that was used to successfully establish PASE
     *                     with the commissionee.
     */
    virtual void OnPairingComplete(CHIP_ERROR error, const std::optional<RendezvousParameters> & rendezvousParameters,
                                   const std::optional<SetupPayload> & setupPayload)
    {
        OnPairingComplete(error);
    }

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
     * Called when MatchingFabricInfo returned from target
     */
    virtual void OnFabricCheck(NodeId matchingNodeId) {}

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

    /**
     * @brief
     *  Called when the ICD registration information (ICD symmetric key, check-in node ID and monitored subject) is required.
     *
     * The DeviceCommissioner will be waiting in the kICDGetRegistrationInfo step and not advancing the commissioning process.
     *
     * The implementation should set the ICD registration info on the CommissioningParameters of the CommissioningDelegate
     * using CommissioningDelegate.SetCommissioningParameters(), and then call DeviceCommissioner.ICDRegistrationInfoReady()
     * in order to resume the commissioning process.
     *
     * Not called if the ICD registration info is provided up front.
     */
    virtual void OnICDRegistrationInfoRequired() {}

    /**
     * @brief
     *   Called when the registration flow for the ICD completes.
     *
     * @param[in] icdNodeId    The node id of the ICD.
     * @param[in] icdCounter   The ICD Counter received from the device.
     */
    virtual void OnICDRegistrationComplete(ScopedNodeId icdNodeId, uint32_t icdCounter) {}

    /**
     * @brief
     *   Called upon completion of the LIT ICD commissioning flow, when ICDStayActiveDuration is set
     *   and the corresponding stayActive command response is received
     *
     * @param[in] icdNodeId    The node id of the ICD.
     * @param[in] promisedActiveDurationMsec   The actual duration that the ICD server can stay active
     *            from the time it receives the StayActiveRequest command.
     */
    virtual void OnICDStayActiveComplete(ScopedNodeId icdNodeId, uint32_t promisedActiveDurationMsec) {}

    /**
     * @brief
     *   Called when a commissioning stage starts.
     *
     * @param[in] peerId an identifier for the commissioning process.  This is generally the
     *                   client-provided commissioning identifier before AddNOC and the actual
     *                   NodeID of the node after AddNoc, combined with the compressed fabric ID for
     *                   the fabric doing the commissioning.
     * @param[in] stageStarting the stage being started.
     */
    virtual void OnCommissioningStageStart(PeerId peerId, CommissioningStage stageStarting) {}

    /**
     * @brief
     *   Called when Wi-Fi credentials are needed.  If the call returns
     *   CHIP_NO_ERROR, commissioning will pause until NetworkCredentialsReady()
     *   is called on the CHIPDeviceController.  This call must happen
     *   asynchronously, after WiFiCredentialsNeeded has returned.
     *
     * @param[in] endpoint the endpoint that hosts the Network Commissioning cluster the credentials are needed for.
     */
    virtual CHIP_ERROR WiFiCredentialsNeeded(EndpointId endpoint) { return CHIP_ERROR_NOT_IMPLEMENTED; }

    /**
     * @brief
     *   Called when Thread credentials are needed.  If the call returns
     *   CHIP_NO_ERROR, commissioning will pause until NetworkCredentialsReady()
     *   is called on the CHIPDeviceController.  This call must happen
     *   asynchronously, after ThreadCredentialsNeeded has returned.
     *
     * @param[in] endpoint the endpoint that hosts the Network Commissioning cluster the credentials are needed for.
     */
    virtual CHIP_ERROR ThreadCredentialsNeeded(EndpointId endpoint) { return CHIP_ERROR_NOT_IMPLEMENTED; }
};

} // namespace Controller
} // namespace chip
