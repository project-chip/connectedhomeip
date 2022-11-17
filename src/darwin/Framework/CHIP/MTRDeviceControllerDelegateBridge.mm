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

#import "MTRDeviceControllerDelegateBridge.h"
#import "MTRError_Internal.h"

MTRDeviceControllerDelegateBridge::MTRDeviceControllerDelegateBridge(void)
    : mDelegate(nil)
{
}

MTRDeviceControllerDelegateBridge::~MTRDeviceControllerDelegateBridge(void) {}

void MTRDeviceControllerDelegateBridge::setDelegate(id<MTRDeviceControllerDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mDelegate = delegate;
        mQueue = queue;
    } else {
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
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingDiscoveringMoreDevices:
        rv = MTRCommissioningStatusDiscoveringMoreDevices;
        break;
    }
    return rv;
}

void MTRDeviceControllerDelegateBridge::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status)
{
    NSLog(@"DeviceControllerDelegate status updated: %d", status);

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onStatusUpdate:)]) {
        if (strongDelegate && mQueue) {
            MTRCommissioningStatus commissioningStatus = MapStatus(status);
            dispatch_async(mQueue, ^{
                [strongDelegate onStatusUpdate:commissioningStatus];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnPairingComplete(CHIP_ERROR error)
{
    NSLog(@"DeviceControllerDelegate Pairing complete. Status %s", chip::ErrorStr(error));

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onPairingComplete:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate onPairingComplete:nsError];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnPairingDeleted(CHIP_ERROR error)
{
    NSLog(@"DeviceControllerDelegate Pairing deleted. Status %s", chip::ErrorStr(error));

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onPairingDeleted:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate onPairingDeleted:nsError];
            });
        }
    }
}

void MTRDeviceControllerDelegateBridge::OnCommissioningComplete(chip::NodeId nodeId, CHIP_ERROR error)
{
    NSLog(@"DeviceControllerDelegate Commissioning complete. NodeId %llu Status %s", nodeId, chip::ErrorStr(error));

    id<MTRDeviceControllerDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onCommissioningComplete:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate onCommissioningComplete:nsError];
            });
        }
    }
}
