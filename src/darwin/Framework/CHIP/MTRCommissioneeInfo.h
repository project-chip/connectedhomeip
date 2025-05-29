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

@class MTRProductIdentity;
@class MTREndpointInfo;

NS_ASSUME_NONNULL_BEGIN

/**
 * Information read from the commissionee device during commissioning.
 */
NS_SWIFT_SENDABLE
MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4))
@interface MTRCommissioneeInfo : NSObject <NSCopying, NSSecureCoding>

/**
 * The product identity (VID / PID) of the commissionee.
 */
@property (nonatomic, copy, readonly) MTRProductIdentity * productIdentity;

/**
 * Endpoint information for all endpoints of the commissionee.
 * Will be present only if readEndpointInformation is set to YES on MTRCommissioningParameters.
 *
 * Use `rootEndpoint` and `-[MTREndpointInfo children]` to traverse endpoints in composition order.
 */
@property (nonatomic, copy, readonly, nullable) NSDictionary<NSNumber *, MTREndpointInfo *> * endpointsById;

/**
 * Endpoint information for the root endpoint of the commissionee.
 * Will be present only if readEndpointInformation is set to YES on MTRCommissioningParameters.
 */
@property (nonatomic, copy, readonly, nullable) MTREndpointInfo * rootEndpoint;

@end

NS_ASSUME_NONNULL_END
