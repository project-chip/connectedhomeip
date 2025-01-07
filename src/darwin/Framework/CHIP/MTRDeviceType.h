/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#import <Foundation/Foundation.h>

#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2))
@interface MTRDeviceType : NSObject

/**
 * Returns an MTRDeviceType for the given ID, if the ID is known.  Returns nil
 * for unknown IDs.
 */
+ (nullable MTRDeviceType *)deviceTypeForID:(NSNumber *)deviceTypeID;

/**
 * The identifier of the device type (32-bit unsigned integer).
 */
@property (nonatomic, readonly, copy) NSNumber * id;

/**
 * Returns the name of the device type.
 */
@property (nonatomic, readonly, retain) NSString * name;

/**
 * Returns whether this is a utility device type.
 */
@property (nonatomic, readonly, assign) BOOL isUtility;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
