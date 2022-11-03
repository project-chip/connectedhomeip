/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRDeviceControllerOverXPC.h"
#import "MTRDeviceControllerXPCConnection.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRDeviceControllerOverXPC ()

@property (nonatomic, readwrite, strong) id<NSCopying> _Nullable controllerID;
@property (nonatomic, readonly, strong) dispatch_queue_t workQueue;
@property (nonatomic, readonly, strong) MTRDeviceControllerXPCConnection * xpcConnection;

@end

NS_ASSUME_NONNULL_END
