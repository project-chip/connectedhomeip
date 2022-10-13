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

#import "MTRDevicePairingDelegateBridge.h"
#import "MTRError_Internal.h"

MTRDevicePairingDelegateBridge::MTRDevicePairingDelegateBridge(void)
    : mDelegate(nil)
{
}

MTRDevicePairingDelegateBridge::~MTRDevicePairingDelegateBridge(void) {}

void MTRDevicePairingDelegateBridge::setDelegate(id<MTRDevicePairingDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mDelegate = delegate;
        mQueue = queue;
    } else {
        mDelegate = nil;
        mQueue = nil;
    }
}

MTRPairingStatus MTRDevicePairingDelegateBridge::MapStatus(chip::Controller::DevicePairingDelegate::Status status)
{
    MTRPairingStatus rv = MTRPairingStatusUnknown;
    switch (status) {
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingSuccess:
        rv = MTRPairingStatusSuccess;
        break;
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingFailed:
        rv = MTRPairingStatusFailed;
        break;
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingDiscoveringMoreDevices:
        rv = MTRPairingStatusDiscoveringMoreDevices;
        break;
    }
    return rv;
}

void MTRDevicePairingDelegateBridge::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status)
{
    NSLog(@"DevicePairingDelegate status updated: %d", status);

    id<MTRDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onStatusUpdate:)]) {
        if (strongDelegate && mQueue) {
            MTRPairingStatus pairingStatus = MapStatus(status);
            dispatch_async(mQueue, ^{
                [strongDelegate onStatusUpdate:pairingStatus];
            });
        }
    }
}

void MTRDevicePairingDelegateBridge::OnPairingComplete(CHIP_ERROR error)
{
    NSLog(@"DevicePairingDelegate Pairing complete. Status %s", chip::ErrorStr(error));

    id<MTRDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onPairingComplete:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate onPairingComplete:nsError];
            });
        }
    }
}

void MTRDevicePairingDelegateBridge::OnPairingDeleted(CHIP_ERROR error)
{
    NSLog(@"DevicePairingDelegate Pairing deleted. Status %s", chip::ErrorStr(error));

    id<MTRDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onPairingDeleted:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate onPairingDeleted:nsError];
            });
        }
    }
}

void MTRDevicePairingDelegateBridge::OnCommissioningComplete(chip::NodeId nodeId, CHIP_ERROR error)
{
    NSLog(@"DevicePairingDelegate Commissioning complete. NodeId %llu Status %s", nodeId, chip::ErrorStr(error));

    id<MTRDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onCommissioningComplete:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [MTRError errorForCHIPErrorCode:error];
                [strongDelegate onCommissioningComplete:nsError];
            });
        }
    }
}
