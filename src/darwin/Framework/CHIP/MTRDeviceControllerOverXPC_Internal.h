/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceControllerOverXPC.h"
#import "MTRDeviceControllerXPCConnection.h"

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRFetchControllerIDCompletion)(
    id _Nullable controllerID, MTRDeviceControllerXPCProxyHandle * _Nullable handle, NSError * _Nullable error);

@interface MTRDeviceControllerOverXPC ()

@property (nonatomic, readwrite, strong) id<NSCopying> _Nullable controllerXPCID;
@property (nonatomic, readonly, strong) dispatch_queue_t workQueue;
@property (nonatomic, readonly, strong) MTRDeviceControllerXPCConnection * xpcConnection;

// Guarantees that completion is called with either a non-nil controllerID or a
// non-nil error.
- (void)fetchControllerIdWithQueue:(dispatch_queue_t)queue completion:(MTRFetchControllerIDCompletion)completion;
@end

NS_ASSUME_NONNULL_END
