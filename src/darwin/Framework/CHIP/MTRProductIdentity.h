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

#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A representation of a (vendor, product) pair that identifies a specific product.
 */
NS_SWIFT_SENDABLE
MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRProductIdentity : NSObject /* <NSCopying, NSSecureCoding> (see below) */

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithVendorID:(NSNumber *)vendorID productID:(NSNumber *)productID;

@property (nonatomic, copy, readonly) NSNumber * vendorID;
@property (nonatomic, copy, readonly) NSNumber * productID;

@end

MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRProductIdentity () <NSCopying, NSSecureCoding>
@end

NS_ASSUME_NONNULL_END
