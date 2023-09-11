/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

MTR_HIDDEN
@interface MTRControllerAccessControl : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
+ (instancetype)alloc NS_UNAVAILABLE;

/**
 * Initialize the access control module. Must be called on the Matter task
 * queue.
 */
+ (void)init;

@end

NS_ASSUME_NONNULL_END
