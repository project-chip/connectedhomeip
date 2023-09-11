/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceControllerXPCConnection;

typedef NSXPCConnection * _Nonnull (^MTRXPCConnectBlock)(void);

MTR_HIDDEN
@interface MTRDeviceControllerOverXPC : MTRDeviceController

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Returns a shared remote device controller associated with an implementation specific id and implementation specific way to
 * connect to an XPC listener.
 */
+ (MTRDeviceControllerOverXPC *)sharedControllerWithID:(id<NSCopying> _Nullable)controllerID
                                       xpcConnectBlock:(MTRXPCConnectBlock)xpcConnectBlock;

@end

NS_ASSUME_NONNULL_END
