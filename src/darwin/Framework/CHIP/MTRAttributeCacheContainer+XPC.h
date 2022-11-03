/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRAttributeCacheContainer.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRAttributeCacheContainer (XPC)
- (void)setXPCConnection:(MTRDeviceControllerXPCConnection *)xpcConnection
            controllerID:(id<NSCopying>)controllerID
                deviceID:(NSNumber *)deviceID;
@end

NS_ASSUME_NONNULL_END
