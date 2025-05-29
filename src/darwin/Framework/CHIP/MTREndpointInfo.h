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

@class MTRDeviceTypeRevision;

NS_ASSUME_NONNULL_BEGIN

/**
 * Meta-data about an endpoint of a Matter node.
 */
NS_SWIFT_SENDABLE
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTREndpointInfo : NSObject <NSCopying, NSSecureCoding>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@property (nonatomic, copy, readonly) NSNumber * endpointID;

@property (nonatomic, copy, readonly) NSArray<MTRDeviceTypeRevision *> * deviceTypes;
@property (nonatomic, copy, readonly) NSArray<NSNumber *> * partsList;

/**
 * The direct children of this endpoint. This excludes indirect descendants
 * even if they are listed in the PartsList attribute of this endpoint due
 * to the Full-Family Pattern being used. Refer to Endpoint Composition Patterns
 * in the Matter specification for details.
 */
@property (nonatomic, copy, readonly) NSArray<MTREndpointInfo *> * children;

@end

NS_ASSUME_NONNULL_END
