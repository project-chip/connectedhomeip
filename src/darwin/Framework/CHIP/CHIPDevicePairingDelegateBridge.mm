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

#import "CHIPDevicePairingDelegateBridge.h"
#import "CHIPError_Internal.h"

CHIPDevicePairingDelegateBridge::CHIPDevicePairingDelegateBridge(void)
    : mDelegate(nil)
{
}

CHIPDevicePairingDelegateBridge::~CHIPDevicePairingDelegateBridge(void) {}

void CHIPDevicePairingDelegateBridge::setDelegate(id<CHIPDevicePairingDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mDelegate = delegate;
        mQueue = queue;
    } else {
        mDelegate = nil;
        mQueue = nil;
    }
}

CHIPPairingStatus CHIPDevicePairingDelegateBridge::MapStatus(chip::Controller::DevicePairingDelegate::Status status)
{
    CHIPPairingStatus rv = kUnknownStatus;
    switch (status) {
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingSuccess:
        rv = kSecurePairingSuccess;
        break;
    case chip::Controller::DevicePairingDelegate::Status::SecurePairingFailed:
        rv = kSecurePairingFailed;
        break;
    }
    return rv;
}

void CHIPDevicePairingDelegateBridge::OnStatusUpdate(chip::Controller::DevicePairingDelegate::Status status)
{
    NSLog(@"DevicePairingDelegate status updated: %d", status);

    id<CHIPDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onStatusUpdate:)]) {
        if (strongDelegate && mQueue) {
            CHIPPairingStatus pairingStatus = MapStatus(status);
            dispatch_async(mQueue, ^{
                [strongDelegate onStatusUpdate:pairingStatus];
            });
        }
    }
}

void CHIPDevicePairingDelegateBridge::OnPairingComplete(CHIP_ERROR error)
{
    NSLog(@"DevicePairingDelegate Pairing complete. Status %s", chip::ErrorStr(error));

    id<CHIPDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onPairingComplete:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [CHIPError errorForCHIPErrorCode:error];
                [strongDelegate onPairingComplete:nsError];
            });
        }
    }
}

void CHIPDevicePairingDelegateBridge::OnPairingDeleted(CHIP_ERROR error)
{
    NSLog(@"DevicePairingDelegate Pairing deleted. Status %s", chip::ErrorStr(error));

    id<CHIPDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onPairingDeleted:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [CHIPError errorForCHIPErrorCode:error];
                [strongDelegate onPairingDeleted:nsError];
            });
        }
    }
}

void CHIPDevicePairingDelegateBridge::OnAddressUpdateComplete(chip::NodeId nodeId, CHIP_ERROR error)
{
    NSLog(@"OnAddressUpdateComplete. Status %s", chip::ErrorStr(error));

    id<CHIPDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onAddressUpdated:)]) {
        if (strongDelegate && mQueue) {
            dispatch_async(mQueue, ^{
                NSError * nsError = [CHIPError errorForCHIPErrorCode:error];
                [strongDelegate onAddressUpdated:nsError];
            });
        }
    }
}
