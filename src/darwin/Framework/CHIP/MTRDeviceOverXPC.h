/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTRDefines.h>

#import "MTRBaseDevice.h"
#import "MTRCluster.h" // For MTRSubscriptionEstablishedHandler
#import "MTRDeviceControllerXPCConnection.h"

@class MTRDeviceControllerOverXPC;

NS_ASSUME_NONNULL_BEGIN

MTR_HIDDEN
@interface MTRDeviceOverXPC : MTRBaseDevice

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithControllerOverXPC:(MTRDeviceControllerOverXPC *)controllerOverXPC
                                 deviceID:(NSNumber *)deviceID
                            xpcConnection:(MTRDeviceControllerXPCConnection *)xpcConnection;

@end

NS_ASSUME_NONNULL_END
