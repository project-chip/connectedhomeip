/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceControllerDelegateBridge.h"
#import "MTRDeviceController.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"

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
    MTR_LOG_DEFAULT("DeviceControllerDelegate status updated: %d", status);

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    MTRDeviceController * strongController = mController;
    if (strongDelegate && mQueue && strongController) {
        if ([strongDelegate respondsToSelector:@selector(controller:statusUpdate:)]) {
            MTRCommissioningStatus commissioningStatus = MapStatus(status);
            dispatch_async(mQueue, ^{
                [strongDelegate controller:strongController statusUpdate:commissioningStatus];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnPairingComplete(CHIP_ERROR error)
{
    MTR_LOG_DEFAULT("DeviceControllerDelegate Pairing complete. Status %s", chip::ErrorStr(error));

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    MTRDeviceController * strongController = mController;
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
    MTR_LOG_DEFAULT("DeviceControllerDelegate Pairing deleted. Status %s", chip::ErrorStr(error));

    // This is never actually called; just do nothing.
}

void MTRDeviceControllerDelegateBridge::OnReadCommissioningInfo(const chip::Controller::ReadCommissioningInfo & info)
{
    chip::VendorId vendorId = info.basic.vendorId;
    uint16_t productId = info.basic.productId;

    MTR_LOG_DEFAULT("DeviceControllerDelegate Read Commissioning Info. VendorId %u ProductId %u", vendorId, productId);

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    MTRDeviceController * strongController = mController;
    if (strongDelegate && mQueue && strongController) {
        if ([strongDelegate respondsToSelector:@selector(controller:readCommissioningInfo:)]) {
            dispatch_async(mQueue, ^{
                auto * info = [[MTRProductIdentity alloc] initWithVendorID:@(vendorId) productID:@(productId)];
                [strongDelegate controller:strongController readCommissioningInfo:info];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnCommissioningComplete(chip::NodeId nodeId, CHIP_ERROR error)
{
    MTR_LOG_DEFAULT("DeviceControllerDelegate Commissioning complete. NodeId %llu Status %s", nodeId, chip::ErrorStr(error));

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    MTRDeviceController * strongController = mController;
    if (strongDelegate && mQueue && strongController) {
        if ([strongDelegate respondsToSelector:@selector(controller:commissioningComplete:nodeID:)]) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                NSNumber * nodeID = nil;
                if (error == CHIP_NO_ERROR) {
                    nodeID = @(nodeId);
                }
                [strongDelegate controller:strongController commissioningComplete:nsError nodeID:nodeID];
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

@implementation MTRProductIdentity

- (instancetype)initWithVendorID:(NSNumber *)vendorID productID:(NSNumber *)productID
{
    if (self = [super init]) {
        _vendorID = vendorID;
        _productID = productID;
    }
    return self;
}

@end
