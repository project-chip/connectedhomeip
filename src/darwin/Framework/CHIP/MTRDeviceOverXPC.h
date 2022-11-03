/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRBaseDevice.h"
#import "MTRDeviceControllerXPCConnection.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceOverXPC : MTRBaseDevice

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
- (void)subscribeWithQueue:(dispatch_queue_t)queue
                minInterval:(NSNumber *)minInterval
                maxInterval:(NSNumber *)maxInterval
              reportHandler:(void (^)(NSArray * _Nullable value, NSError * _Nullable error))reportHandler
    subscriptionEstablished:(dispatch_block_t _Nullable)subscriptionEstablishedHandler NS_UNAVAILABLE;

- (instancetype)initWithController:(id<NSCopying>)controller
                          deviceID:(NSNumber *)deviceID
                     xpcConnection:(MTRDeviceControllerXPCConnection *)xpcConnection;

@end

NS_ASSUME_NONNULL_END
