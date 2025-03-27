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

#import "MTRCommissioneeInfo_Internal.h"
#import "MTRDeviceController.h"
#import "MTRDeviceController_Internal.h"
#import "MTREndpointInfo_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRProductIdentity.h"
#import "MTRUtilities.h"

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

void MTRDeviceControllerDelegateBridge::OnPairingComplete(CHIP_ERROR error)
{
    MTRDeviceController * strongController = mController;

    if (error == CHIP_NO_ERROR) {
        MTR_LOG("%@ MTRDeviceControllerDelegate PASE session establishment succeeded.", strongController);
    } else {
        MTR_LOG_ERROR("%@ MTRDeviceControllerDelegate PASE session establishment failed: %" CHIP_ERROR_FORMAT, strongController, error.Format());
    }
    MATTER_LOG_METRIC_END(kMetricSetupPASESession, error);

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue && strongController) {
        if ([strongDelegate respondsToSelector:@selector(controller:commissioningSessionEstablishmentDone:)]) {
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
        auto * commissioneeInfo = [[MTRCommissioneeInfo alloc] initWithCommissioningInfo:info];
        dispatch_async(mQueue, ^{
            if (wantCommissioneeInfo) { // prefer the newer delegate method over the deprecated one
                [strongDelegate controller:strongController readCommissioneeInfo:commissioneeInfo];
            } else if (wantProductIdentity) {
                [strongDelegate controller:strongController readCommissioningInfo:commissioneeInfo.productIdentity];
            }
        });
    }
}

void MTRDeviceControllerDelegateBridge::OnCommissioningComplete(chip::NodeId nodeId, CHIP_ERROR error)
{
    MTRDeviceController * strongController = mController;

    MTR_LOG("%@ DeviceControllerDelegate Commissioning complete. NodeId 0x%016llx Status %s", strongController, nodeId, chip::ErrorStr(error));
    MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, error);

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue && strongController) {

        // Always collect the metrics to avoid unbounded growth of the stats in the collector
        MTRMetrics * metrics = [[MTRMetricsCollector sharedInstance] metricSnapshot:TRUE];
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

void MTRDeviceControllerDelegateBridge::SetDeviceNodeID(chip::NodeId deviceNodeId)
{
    mDeviceNodeId = deviceNodeId;
}
