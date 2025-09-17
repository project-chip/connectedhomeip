/**
 *    Copyright (c) 2025 Project CHIP Authors
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
 * Information about a network interface on a Matter node.
 */

NS_SWIFT_SENDABLE
MTR_PROVISIONALLY_AVAILABLE
@interface MTRNetworkInterfaceInfo : NSObject <NSCopying, NSSecureCoding>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * The endpoint this network interface is exposed on (i.e. the endpoint its
 * corresponding Network Commissioning server cluster instance is on).
 */
@property (nonatomic, copy, readonly) NSNumber * endpointID;

/**
 * The value of the FeatureMap attribute of the corresponding Network
 * Commissioning cluster instance (which indicates what network technology is
 * being used; see MTRNetworkCommissioningFeature).
 */
@property (nonatomic, copy, readonly) NSNumber * featureMap;

@end

NS_ASSUME_NONNULL_END
