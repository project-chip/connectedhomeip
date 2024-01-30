/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A representation of a "device type revision" in the sense used in the Matter
 * specification.  This has an identifier and a version number.
 */
MTR_NEWLY_AVAILABLE
@interface MTRDeviceTypeRevision : NSObject <NSCopying>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * The provided deviceTypeID must be in the range 0xVVVV0000-0xVVVVBFFF, where
 * VVVV is the vendor identifier (0 for standard device types).
 *
 * The provided deviceTypeRevision must be in the range 1-65535.
 */
- (nullable instancetype)initWithDeviceTypeID:(NSNumber *)deviceTypeID revision:(NSNumber *)revision;

@property (nonatomic, copy, readonly) NSNumber * deviceTypeID;
@property (nonatomic, copy, readonly) NSNumber * deviceTypeRevision;

@end

NS_ASSUME_NONNULL_END
