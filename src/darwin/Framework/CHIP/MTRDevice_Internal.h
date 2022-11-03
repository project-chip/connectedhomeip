/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRBaseDevice.h"
#import "MTRDevice.h"

#include <app/DeviceProxy.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRDevicePerformAsyncBlock)(MTRBaseDevice * baseDevice);

@interface MTRDevice ()
- (instancetype)initWithNodeID:(NSNumber *)nodeID controller:(MTRDeviceController *)controller;

// Called from MTRClusters for writes and commands
- (void)setExpectedValues:(NSArray<NSDictionary<NSString *, id> *> *)values
    expectedValueInterval:(NSNumber *)expectedValueIntervalMs;

@property (nonatomic, readonly) MTRDeviceController * deviceController;
@property (nonatomic, readonly, copy) NSNumber * nodeID;
@property (nonatomic, readonly) MTRAsyncCallbackWorkQueue * asyncCallbackWorkQueue;

@end

#pragma mark - Utility for clamping numbers
// Returns a NSNumber object that is aNumber if it falls within the range [min, max].
// Returns min or max, if it is below or above, respectively.
NSNumber * MTRClampedNumber(NSNumber * aNumber, NSNumber * min, NSNumber * max);

NS_ASSUME_NONNULL_END
