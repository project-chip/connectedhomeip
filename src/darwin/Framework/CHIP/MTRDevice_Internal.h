/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
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

// called by controller to clean up and shutdown
- (void)invalidate;

// Called by controller when a new operational advertisement for what we think
// is this device's identity has been observed.  This could have
// false-positives, for example due to compressed fabric id collisions.
- (void)nodeMayBeAdvertisingOperational;

@property (nonatomic, readonly) MTRDeviceController * deviceController;
@property (nonatomic, readonly, copy) NSNumber * nodeID;
// Queue used for various internal bookkeeping work.  In general endWork calls
// on work items should happen on this queue, so we don't block progress of the
// asyncCallbackWorkQueue on any client code.
@property (nonatomic) dispatch_queue_t queue;
@property (nonatomic, readonly) MTRAsyncCallbackWorkQueue * asyncCallbackWorkQueue;

@end

#pragma mark - Utility for clamping numbers
// Returns a NSNumber object that is aNumber if it falls within the range [min, max].
// Returns min or max, if it is below or above, respectively.
NSNumber * MTRClampedNumber(NSNumber * aNumber, NSNumber * min, NSNumber * max);

NS_ASSUME_NONNULL_END
