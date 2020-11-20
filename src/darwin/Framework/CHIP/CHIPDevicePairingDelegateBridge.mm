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
#import "CHIPError.h"
#import <Foundation/Foundation.h>

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

PairingStatus CHIPDevicePairingDelegateBridge::MapStatus(chip::RendezvousSessionDelegate::Status status)
{
    PairingStatus rv = kUnknownStatus;
    switch (status) {
    case chip::RendezvousSessionDelegate::Status::SecurePairingSuccess:
        rv = kSecurePairingSuccess;
        break;
    case chip::RendezvousSessionDelegate::Status::SecurePairingFailed:
        rv = kSecurePairingFailed;
        break;
    case chip::RendezvousSessionDelegate::Status::NetworkProvisioningSuccess:
        rv = kNetworkProvisioningSuccess;
        break;
    case chip::RendezvousSessionDelegate::Status::NetworkProvisioningFailed:
        rv = kNetworkProvisioningFailed;
        break;
    }
    return rv;
}

void CHIPDevicePairingDelegateBridge::OnStatusUpdate(chip::RendezvousSessionDelegate::Status status)
{
    NSLog(@"DevicePairingDelegate status updated: %d", status);

    id<CHIPDevicePairingDelegate> strongDelegate = mDelegate;
    if ([strongDelegate respondsToSelector:@selector(onStatusUpdate:)]) {
        if (strongDelegate && mQueue) {
            PairingStatus pairingStatus = MapStatus(status);
            dispatch_async(mQueue, ^{
                [strongDelegate onStatusUpdate:pairingStatus];
            });
        }
    }
}

void CHIPDevicePairingDelegateBridge::OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback)
{
    NSLog(@"DevicePairingDelegate Requesting network credentials");

    mCallback = callback;
    mHandler = ^(NSString * ssid, NSString * passwd) {
        mCallback->SendNetworkCredentials([ssid UTF8String], [passwd UTF8String]);
    };

    id<CHIPDevicePairingDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        dispatch_async(mQueue, ^{
            [strongDelegate onNetworkCredentialsRequested:mHandler];
        });
    }
}

void CHIPDevicePairingDelegateBridge::OnOperationalCredentialsRequested(
    const char * csr, size_t csr_length, chip::RendezvousDeviceCredentialsDelegate * callback)
{
    NSLog(@"DevicePairingDelegate Requesting operational credentials");
}

void CHIPDevicePairingDelegateBridge::OnPairingComplete(CHIP_ERROR error)
{
    NSLog(@"DevicePairingDelegate Pairing complete. Status %d", error);

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
    NSLog(@"DevicePairingDelegate Pairing deleted. Status %d", error);

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
