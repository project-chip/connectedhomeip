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

    MTRCommissioningStatus MapStatus(chip::Controller::DevicePairingDelegate::Status status);
};

NS_ASSUME_NONNULL_END
