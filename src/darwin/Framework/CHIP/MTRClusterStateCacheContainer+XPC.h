/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRClusterStateCacheContainer.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRClusterStateCacheContainer (XPC)
- (void)setXPCConnection:(MTRDeviceControllerXPCConnection *)xpcConnection
            controllerID:(id<NSCopying>)controllerID
                deviceID:(NSNumber *)deviceID;
@end

NS_ASSUME_NONNULL_END
