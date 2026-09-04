/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "MTRCommissioningParameters.h"
#import "MTRDeviceControllerDelegate.h"
#import "MTRDeviceControllerDelegate_Internal.h"

#include <clusters/NetworkCommissioning/Enums.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningDelegate.h>
#include <lib/support/BitMask.h>
#include <platform/CHIPDeviceConfig.h>

#include <string>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

class MTRDeviceControllerDelegateBridge : public chip::Controller::DevicePairingDelegate {
public:
    MTRDeviceControllerDelegateBridge();
    ~MTRDeviceControllerDelegateBridge();

    void setDelegate(MTRDeviceController * controller, id<MTRDeviceControllerDelegate> delegate, dispatch_queue_t queue);

    // DevicePairingDelegate implementation
    void OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status) override;

    void OnPairingComplete(CHIP_ERROR error, const std::optional<chip::RendezvousParameters> & rendezvousParameters, const std::optional<chip::SetupPayload> & setupPayload) override;

    void OnPairingDeleted(CHIP_ERROR error) override;

    void OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info) override;

    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR error) override;
    void OnCommissioningSuccess(chip::PeerId peerId) override;
    void OnCommissioningFailure(chip::PeerId peerId, const chip::Controller::CompletionStatus & completionStatus) override;
    void OnCommissioningStatusUpdate(chip::PeerId peerId, chip::Controller::CommissioningStage stageCompleted, CHIP_ERROR error) override;
    void OnCommissioningStageStart(chip::PeerId peerId, chip::Controller::CommissioningStage stageStarting) override;

    void
    OnScanNetworksSuccess(const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & dataResponse) override;
    void OnScanNetworksFailure(CHIP_ERROR error) override;

    CHIP_ERROR WiFiCredentialsNeeded(chip::EndpointId endpoint) override;
    CHIP_ERROR ThreadCredentialsNeeded(chip::EndpointId endpoint) override;

    // Other helper methods
    void SetDeviceNodeID(chip::NodeId deviceNodeId);

    void SetCommissioningParameters(MTRCommissioningParameters * commissioningParameters);

private:
    MTRDeviceController * __weak mController;
    _Nullable id<MTRDeviceControllerDelegate> __weak mDelegate;
    _Nullable dispatch_queue_t mQueue;
    chip::NodeId mDeviceNodeId;

    // TODO: Once the C++ SDK is fixed to correctly handle multiple network
    // commissioning endpoints, we will know which scan is which, but for now
    // just assume (since that's what the SDK does) that the scan, if any, is
    // always on endpoint 0.
    //
    // See https://github.com/project-chip/connectedhomeip/issues/40755
    chip::BitMask<chip::app::Clusters::NetworkCommissioning::Feature> mRootEndpointNetworkCommissioningFeatureMap;

    MTRCommissioningParameters * mCommissioningParameters;

    // OnCommissioningComplete records the (nodeId, error) tuple here and returns
    // without notifying the delegate. The subsequent OnCommissioningSuccess /
    // OnCommissioningFailure call (which always runs synchronously after
    // OnCommissioningComplete on the same Matter work queue thread, see
    // src/controller/CHIPDeviceController.cpp around DeviceCommissioner::OnDone)
    // then performs the dispatch_async with every input captured by value,
    // including the device-reported NetworkCommissioning status from
    // CompletionStatus when available. This data-flow design intentionally
    // avoids a cross-queue race on a stash that would otherwise have to be
    // read on the delegate queue while being written on the work queue.
    bool mHasPendingCommissioningComplete = false;
    chip::NodeId mPendingCommissioningCompleteNodeId = chip::kUndefinedNodeId;
    CHIP_ERROR mPendingCommissioningCompleteError = CHIP_NO_ERROR;
    // NetworkCommissioning status carried with the pending record. Set by
    // OnCommissioningFailure when the upstream CompletionStatus carries one;
    // cleared to NullOptional by OnCommissioningComplete (initial stash) and
    // again by DispatchPendingCommissioningComplete after dispatch. Storing
    // it here (rather than passing it as a parameter to the dispatcher)
    // ensures every flush path -- setDelegate-rebind, defensive
    // double-OnCommissioningComplete drain, and the normal Success/Failure
    // path -- forwards the same status to the delegate.
    //
    // Note on rebind/double-Complete flush paths: the value is NullOptional in
    // those cases (OnCommissioningComplete always clears this field before
    // OnCommissioningFailure sets it, and the C++ SDK fires the two calls
    // synchronously, so no real NC status can survive into a rebind-flush).
    // A non-null value is only stashed by OnCommissioningFailure after
    // OnCommissioningComplete sets the pending record.
    chip::Optional<chip::app::Clusters::NetworkCommissioning::NetworkCommissioningStatusEnum> mPendingNetworkCommissioningStatus;
    // Optional driver-level errorValue from a NetworkCommissioning
    // ConnectNetworkResponse, stashed alongside mPendingNetworkCommissioningStatus
    // by OnCommissioningFailure and read by DispatchPendingCommissioningComplete.
    chip::Optional<int32_t> mPendingNetworkCommissioningConnectErrorValue;
    // Owned copy of the device-supplied debugText from a NetworkCommissioning
    // NetworkConfigResponse / ConnectNetworkResponse. Empty when no debugText
    // was reported.
    std::string mPendingNetworkCommissioningDebugText;
    // Idempotence flag set by DispatchPendingCommissioningComplete after a
    // successful dispatch and cleared by OnCommissioningComplete on the next
    // attempt. Guards against double-fires when OnStatusUpdate's PASE-fail
    // synthesizer and a subsequent upstream Success/Failure both attempt to
    // drain the pending record for a single commissioning attempt.
    bool mDispatchedCommissioningCompleteForCurrentAttempt = false;

    MTRCommissioningStatus MapStatus(chip::Controller::DevicePairingDelegate::Status status);

    // Helper invoked from OnCommissioningSuccess / OnCommissioningFailure once
    // the full picture (error + optional NetworkCommissioning status) is known.
    // Consumes any pending commissioning-complete record set by
    // OnCommissioningComplete and dispatches the delegate callbacks on mQueue.
    // Reads mPendingNetworkCommissioningStatus rather than taking a parameter,
    // so flush paths that did not observe the upstream Failure callback (e.g.
    // setDelegate rebind, defensive double-Complete) still forward whatever
    // status was previously stashed.
    void DispatchPendingCommissioningComplete();
};

NS_ASSUME_NONNULL_END
