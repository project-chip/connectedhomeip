/**
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#import "MTRDeviceControllerDelegateBridge.h"

#import <Matter/MTRClusterConstants.h>

#import "MTRCommissioneeInfo_Internal.h"
#import "MTRDeviceController.h"
#import "MTRDeviceController_Internal.h"
#import "MTREndpointInfo_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRProductIdentity.h"
#import "MTRSetupPayload_Internal.h"
#import "MTRUtilities.h"
#import "zap-generated/MTRCommandPayloads_Internal.h"

#include <lib/core/DataModelTypes.h>

using namespace chip::Tracing::DarwinFramework;

MTRDeviceControllerDelegateBridge::MTRDeviceControllerDelegateBridge(void)
    : mDelegate(nil)
{
}

MTRDeviceControllerDelegateBridge::~MTRDeviceControllerDelegateBridge(void) {}

void MTRDeviceControllerDelegateBridge::setDelegate(
    MTRDeviceController * controller, id<MTRDeviceControllerDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mController = controller;
        mDelegate = delegate;
        mQueue = queue;
    } else {
        mController = nil;
        mDelegate = nil;
        mQueue = nil;
    }
}

MTRCommissioningStatus MTRDeviceControllerDelegateBridge::MapStatus(chip::Controller::DevicePairingDelegate::Status status)
{
    MTRCommissioningStatus rv = MTRCommissioningStatusUnknown;
    switch (status) {
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingSuccess:
        rv = MTRCommissioningStatusSuccess;
        break;
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingFailed:
        rv = MTRCommissioningStatusFailed;
        break;
    }
    return rv;
}

void MTRDeviceControllerDelegateBridge::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status)
{
    MTRDeviceController * strongController = mController;

    MTR_LOG("%@ DeviceControllerDelegate status updated: %d", strongController, status);

    // If pairing failed, PASE failed. However, since OnPairingComplete(failure_code) might not be invoked in all cases, mark
    // end of PASE with timeout as assumed failure. If OnPairingComplete is invoked, the right error code will be updated in
    // the end event
    if (status == chip::Controller::DevicePairingDelegate::Status::SecurePairingFailed) {
        MATTER_LOG_METRIC_END(kMetricSetupPASESession, CHIP_ERROR_TIMEOUT);
    }

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue && strongController) {
        if ([strongDelegate respondsToSelector:@selector(controller:statusUpdate:)]) {
            MTRCommissioningStatus commissioningStatus = MapStatus(status);
            dispatch_async(mQueue, ^{
                [strongDelegate controller:strongController statusUpdate:commissioningStatus];
            });
        }

        // If PASE session setup fails and the client implements the delegate that accepts metrics, invoke the delegate
        // to mark end of commissioning request.
        // Since OnPairingComplete(failure_code) might not be invoked in all cases, use this opportunity to inform of failed commissioning
        // and default the error to timeout since that is best guess in this layer.
        if (status == chip::Controller::DevicePairingDelegate::Status::SecurePairingFailed && [strongDelegate respondsToSelector:@selector(controller:commissioningComplete:nodeID:metrics:)]) {
            OnCommissioningComplete(mDeviceNodeId, CHIP_ERROR_TIMEOUT);
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnPairingComplete(CHIP_ERROR error, const std::optional<chip::RendezvousParameters> & rendezvousParameters, const std::optional<chip::SetupPayload> & setupPayload)
{
    MTRDeviceController * strongController = mController;

    if (error == CHIP_NO_ERROR) {
        MTR_LOG("%@ MTRDeviceControllerDelegate PASE session establishment succeeded.", strongController);
    } else {
        MTR_LOG_ERROR("%@ MTRDeviceControllerDelegate PASE session establishment failed: %" CHIP_ERROR_FORMAT, strongController, error.Format());
    }
    MATTER_LOG_METRIC_END(kMetricSetupPASESession, error);

    auto * strongDelegate = static_cast<id<MTRDeviceControllerDelegate_Internal>>(mDelegate);
    if (strongDelegate && mQueue && strongController) {
        // The methods on MTRDeviceControllerDelegate_Internal are required, but
        // we don't know whether our delegate actually implements the protocol,
        // so still need to do the respondsToSelector checks.
        if ([strongDelegate respondsToSelector:@selector(controller:commissioningSessionEstablishmentDone:forPayload:)]) {
            MTRSetupPayload * payload;
            if (setupPayload) {
                payload = [[MTRSetupPayload alloc] initWithSetupPayload:*setupPayload];
            }
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate controller:strongController commissioningSessionEstablishmentDone:nsError forPayload:payload];
            });
        } else if ([strongDelegate respondsToSelector:@selector(controller:commissioningSessionEstablishmentDone:)]) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate controller:strongController commissioningSessionEstablishmentDone:nsError];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnPairingDeleted(CHIP_ERROR error)
{
    MTR_LOG("DeviceControllerDelegate Pairing deleted. Status %s", chip::ErrorStr(error));

    // This is never actually called; just do nothing.
}

void MTRDeviceControllerDelegateBridge::OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info)
{
    MTRDeviceController * strongController = mController;
    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    VerifyOrReturn(strongDelegate && mQueue && strongController);

    // TODO: These checks are pointless since currently mController == mDelegate
    BOOL wantCommissioneeInfo = [strongDelegate respondsToSelector:@selector(controller:readCommissioneeInfo:)];
    BOOL wantProductIdentity = [strongDelegate respondsToSelector:@selector(controller:readCommissioningInfo:)];
    if (wantCommissioneeInfo || wantProductIdentity) {
        auto * commissioneeInfo = [[MTRCommissioneeInfo alloc] initWithCommissioningInfo:info commissioningParameters:mCommissioningParameters];
        dispatch_async(mQueue, ^{
            if (wantCommissioneeInfo) { // prefer the newer delegate method over the deprecated one
                [strongDelegate controller:strongController readCommissioneeInfo:commissioneeInfo];
            } else if (wantProductIdentity) {
                [strongDelegate controller:strongController readCommissioningInfo:commissioneeInfo.productIdentity];
            }
        });
    }

    mRootEndpointNetworkCommissioningFeatureMap.ClearAll();
    if (info.attributes) {
        using TypeInfo = chip::app::Clusters::NetworkCommissioning::Attributes::FeatureMap::TypeInfo;
        chip::app::ConcreteAttributePath path(chip::kRootEndpointId, MTRClusterIDTypeNetworkCommissioningID,
            MTRAttributeIDTypeGlobalAttributeFeatureMapID);
        TypeInfo::DecodableType value;
        if (info.attributes->Get<TypeInfo>(path, value) == CHIP_NO_ERROR) {
            mRootEndpointNetworkCommissioningFeatureMap.SetRaw(value);
        }
    }

    // Don't hold on to the commissioning parameters now that we don't need them anymore.
    mCommissioningParameters = nil;
}

void MTRDeviceControllerDelegateBridge::OnCommissioningComplete(chip::NodeId nodeId, CHIP_ERROR error)
{
    MTRDeviceController * strongController = mController;

    MTR_LOG("%@ DeviceControllerDelegate Commissioning complete. NodeId 0x%016llx Status %s", strongController, nodeId, chip::ErrorStr(error));
    MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, error);

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue && strongController) {

        // Always collect the metrics to avoid unbounded growth of the stats in the collector
        MTRMetrics * metrics = [[MTRMetricsCollector sharedInstance] metricSnapshotForCommissioning:YES];
        MTR_LOG("%@ Device commissioning complete with metrics %@", strongController, metrics);

        if ([strongDelegate respondsToSelector:@selector(controller:commissioningComplete:nodeID:)] ||
            [strongDelegate respondsToSelector:@selector(controller:commissioningComplete:nodeID:metrics:)]) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                NSNumber * nodeID = nil;
                if (error == CHIP_NO_ERROR) {
                    nodeID = @(nodeId);
                }

                // If the client implements the metrics delegate, prefer that over others
                if ([strongDelegate respondsToSelector:@selector(controller:commissioningComplete:nodeID:metrics:)]) {
                    [strongDelegate controller:strongController commissioningComplete:nsError nodeID:nodeID metrics:metrics];
                } else {
                    [strongDelegate controller:strongController commissioningComplete:nsError nodeID:nodeID];
                }
            });
            return;
        }
        // If only the DEPRECATED function is defined
        if ([strongDelegate respondsToSelector:@selector(controller:commissioningComplete:)]) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate controller:strongController commissioningComplete:nsError];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnCommissioningStatusUpdate(chip::PeerId peerId, chip::Controller::CommissioningStage stageCompleted, CHIP_ERROR error)
{
    using namespace chip::Controller;

    MTRDeviceController * strongController = mController;

    MTR_LOG("%@ DeviceControllerDelegate stage %s completed for nodeID 0x%016llx with status %s", strongController, StageToString(stageCompleted), peerId.GetNodeId(), error.AsString());

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;

    if (strongDelegate && mQueue && strongController && error == CHIP_NO_ERROR && (stageCompleted == CommissioningStage::kFailsafeBeforeWiFiEnable || stageCompleted == CommissioningStage::kFailsafeBeforeThreadEnable)) {
        // We're about to send the ConnectNetwork command.  Let our delegate
        // know that the other side now has network credentials.
        NSNumber * nodeID = @(peerId.GetNodeId());
        if ([strongDelegate respondsToSelector:@selector(controller:commissioneeHasReceivedNetworkCredentials:)]) {
            dispatch_async(mQueue, ^{
                [strongDelegate controller:strongController commissioneeHasReceivedNetworkCredentials:nodeID];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnCommissioningStageStart(chip::PeerId peerId, chip::Controller::CommissioningStage stageStarting)
{
    MTRDeviceController * strongController = mController;
    auto * strongDelegate = static_cast<id<MTRDeviceControllerDelegate_Internal>>(mDelegate);
    // The methods on MTRDeviceControllerDelegate_Internal are required, but
    // we don't know whether our delegate actually implements the protocol,
    // so still need to do the respondsToSelector checks.

    if (stageStarting != chip::Controller::CommissioningStage::kScanNetworks) {
        // This is the only stage we handle right now.
        return;
    }

    if (!strongController || !mQueue || !strongDelegate) {
        MTR_LOG_ERROR("Unable to handle commissioning stage start: missing required data: %@ %@ %@", strongController, mQueue, strongDelegate);
        return;
    }

    if ([strongDelegate respondsToSelector:@selector(controllerStartingNetworkScan:)]) {
        dispatch_async(mQueue, ^{
            [strongDelegate controllerStartingNetworkScan:strongController];
        });
    }
}

void MTRDeviceControllerDelegateBridge::OnScanNetworksSuccess(const chip::app::Clusters::NetworkCommissioning::Commands::ScanNetworksResponse::DecodableType & dataResponse)
{
    MTRDeviceController * strongController = mController;
    auto * strongDelegate = static_cast<id<MTRDeviceControllerDelegate_Internal>>(mDelegate);
    // The methods on MTRDeviceControllerDelegate_Internal are required, but
    // we don't know whether our delegate actually implements the protocol,
    // so still need to do the respondsToSelector checks.

    MTR_LOG("%@ DeviceControllerDelegate network scan for nodeID 0x%016llx complete", strongController, mDeviceNodeId);

    if (!strongController || !mQueue || !strongDelegate) {
        MTR_LOG_ERROR("Unable to handle ScanNetworks success: missing required data: %@ %@ %@", strongController, mQueue, strongDelegate);
        return;
    }

    auto * response = [[MTRNetworkCommissioningClusterScanNetworksResponseParams alloc] initWithDecodableStruct:dataResponse];
    if (response == nil) {
        // The TLV in the arrays was invalid.  Just treat this as a failure.
        return OnScanNetworksFailure(CHIP_ERROR_SCHEMA_MISMATCH);
    }

    if (response.wiFiScanResults) {
        if ([strongDelegate respondsToSelector:@selector(controller:needsWiFiCredentialsWithScanResults:error:)]) {
            dispatch_async(mQueue, ^{
                [strongDelegate controller:strongController needsWiFiCredentialsWithScanResults:response.wiFiScanResults error:nil];
            });
        }
    } else if (response.threadScanResults) {
        if ([strongDelegate respondsToSelector:@selector(controller:needsThreadCredentialsWithScanResults:error:)]) {
            dispatch_async(mQueue, ^{
                [strongDelegate controller:strongController needsThreadCredentialsWithScanResults:response.threadScanResults error:nil];
            });
        }
    } else {
        MTR_LOG_ERROR("Scan succeeded but for unknown network type: %lu", static_cast<unsigned long>(mRootEndpointNetworkCommissioningFeatureMap.Raw()));
    }
}

void MTRDeviceControllerDelegateBridge::OnScanNetworksFailure(CHIP_ERROR error)
{
    MTRDeviceController * strongController = mController;
    auto * strongDelegate = static_cast<id<MTRDeviceControllerDelegate_Internal>>(mDelegate);
    // The methods on MTRDeviceControllerDelegate_Internal are required, but
    // we don't know whether our delegate actually implements the protocol,
    // so still need to do the respondsToSelector checks.

    MTR_LOG("%@ DeviceControllerDelegate network scan for nodeID 0x%016llx failed: %" CHIP_ERROR_FORMAT,
        strongController, mDeviceNodeId, error.Format());

    if (!strongController || !mQueue || !strongDelegate) {
        MTR_LOG_ERROR("Unable to handle ScanNetworks failure: missing required data: %@ %@ %@", strongController, mQueue, strongDelegate);
        return;
    }

    // We don't know which type of scan this was (see
    // https://github.com/project-chip/connectedhomeip/issues/40755), so decide
    // based on mRootEndpointNetworkCommissioningFeatureMap.
    using Feature = chip::app::Clusters::NetworkCommissioning::Feature;
    if (mRootEndpointNetworkCommissioningFeatureMap.Has(Feature::kWiFiNetworkInterface)) {
        if ([strongDelegate respondsToSelector:@selector(controller:needsWiFiCredentialsWithScanResults:error:)]) {
            dispatch_async(mQueue, ^{
                [strongDelegate controller:strongController needsWiFiCredentialsWithScanResults:nil error:[MTRError errorForCHIPErrorCode:error]];
            });
        }
    } else if (mRootEndpointNetworkCommissioningFeatureMap.Has(Feature::kThreadNetworkInterface)) {
        if ([strongDelegate respondsToSelector:@selector(controller:needsThreadCredentialsWithScanResults:error:)]) {
            dispatch_async(mQueue, ^{
                [strongDelegate controller:strongController needsThreadCredentialsWithScanResults:nil error:[MTRError errorForCHIPErrorCode:error]];
            });
        }
    } else {
        MTR_LOG_ERROR("Scan failed for unknown network type: %lu", static_cast<unsigned long>(mRootEndpointNetworkCommissioningFeatureMap.Raw()));
    }
}

CHIP_ERROR MTRDeviceControllerDelegateBridge::WiFiCredentialsNeeded(chip::EndpointId endpoint)
{
    MTRDeviceController * strongController = mController;
    auto * strongDelegate = static_cast<id<MTRDeviceControllerDelegate_Internal>>(mDelegate);
    // The methods on MTRDeviceControllerDelegate_Internal are required, but
    // we don't know whether our delegate actually implements the protocol,
    // so still need to do the respondsToSelector checks.

    if (!strongController || !mQueue || !strongDelegate) {
        MTR_LOG_ERROR("Unable to handle WiFiCredentialsNeeded: missing required data: %@ %@ %@", strongController, mQueue, strongDelegate);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (![strongDelegate respondsToSelector:@selector(controller:needsWiFiCredentialsWithScanResults:error:)]) {
        // Fail out: we have no way to provide credentials and don't want to
        // block commissioning until timeout.
        MTR_LOG_ERROR("%@ Wi-Fi credentials needed, but no way to request them", strongController);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    dispatch_async(mQueue, ^{
        [strongDelegate controller:strongController needsWiFiCredentialsWithScanResults:nil error:nil];
    });

    return CHIP_NO_ERROR;
}

CHIP_ERROR MTRDeviceControllerDelegateBridge::ThreadCredentialsNeeded(chip::EndpointId endpoint)
{
    MTRDeviceController * strongController = mController;
    auto * strongDelegate = static_cast<id<MTRDeviceControllerDelegate_Internal>>(mDelegate);
    // The methods on MTRDeviceControllerDelegate_Internal are required, but
    // we don't know whether our delegate actually implements the protocol,
    // so still need to do the respondsToSelector checks.

    if (!strongController || !mQueue || !strongDelegate) {
        MTR_LOG_ERROR("Unable to handle ThreadCredentialsNeeded: missing required data: %@ %@ %@", strongController, mQueue, strongDelegate);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (![strongDelegate respondsToSelector:@selector(controller:needsThreadCredentialsWithScanResults:error:)]) {
        // Fail out: we have no way to provide credentials and don't want to
        // block commissioning until timeout.
        MTR_LOG_ERROR("%@ Thread credentials needed, but no way to request them", strongController);
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    dispatch_async(mQueue, ^{
        [strongDelegate controller:strongController needsThreadCredentialsWithScanResults:nil error:nil];
    });

    return CHIP_NO_ERROR;
}

void MTRDeviceControllerDelegateBridge::SetDeviceNodeID(chip::NodeId deviceNodeId)
{
    mDeviceNodeId = deviceNodeId;
}

void MTRDeviceControllerDelegateBridge::SetCommissioningParameters(MTRCommissioningParameters * commissioningParameters)
{
    mCommissioningParameters = commissioningParameters;
}
