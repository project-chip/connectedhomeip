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
#import <Matter/MTRAccessGrant.h>
#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceType.h>
#import <Matter/MTRServerClusterDescription.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A representation of an endpoint implemented by an MTRDeviceController.
 */
MTR_NEWLY_AVAILABLE
@interface MTREndpointDescription : NSObject <NSCopying>

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * The provided endpointID must be in the range 1-65535.  The list of device
 * types provided must be nonempty (but may include vendor-specific device
 * types).
 */
- (nullable instancetype)initWithEndpointID:(NSNumber *)endpointID deviceTypes:(NSArray<MTRDeviceType *> *)deviceTypes error:(NSError * __autoreleasing *)error;

@property (nonatomic, copy, readonly) NSNumber * endpointID;

@property (nonatomic, copy, readonly) NSArray<MTRDeviceType *> * deviceTypes;

/**
 * A list of entities that are allowed to access all clusters on this endpoint.
 * If more fine-grained access control is desired, access grants should be
 * defined on individual clusters.
 *
 * Defaults to no access granted.
 */
@property (nonatomic, copy) NSArray<MTRAccessGrant *> * accessGrants;

/**
 * A list of server clusters supported on this endpoint.  The Descriptor cluster
 * does not need to be included unless a TagList attribute is desired on it or
 * unless it has a non-empty PartsList.  If not included, the Descriptor cluster
 * will be generated automatically.
 */
@property (nonatomic, copy) NSArray<MTRServerClusterDescription *> * serverClusters;

/**
 * Whether the endpoint should be enabled as soon as the description is
 * provided.  Defaults to YES.
 */
@property (nonatomic, assign) BOOL autoEnable;

@end

NS_ASSUME_NONNULL_END
