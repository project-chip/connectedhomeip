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
#import <Matter/MTRServerAttribute.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * A representation of a server cluster implemented by an MTRDeviceController.
 *
 * MTRServerCluster's API can be accessed from any thread.
 */
NS_SWIFT_SENDABLE
MTR_AVAILABLE(ios(17.6), macos(14.6), watchos(10.6), tvos(17.6))
@interface MTRServerCluster : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * The provided clusterID must not be MTRClusterIDTypeDescriptorID; see
 * newDescriptorCluster.
 *
 * Otherwise, it must be a valid cluster identifier.  That means:
 *
 * * In the range 0-0x7FFF for standard clusters.
 * * In the range 0xVVVVFC00-0xVVVVFFFE for vendor-specific clusters, where VVVV
 *   is the vendor identifier.
 *
 * The provided revision must be in the range 1-65535.
 *
 */
- (nullable instancetype)initWithClusterID:(NSNumber *)clusterID revision:(NSNumber *)revision;

/**
 * Add an access grant to the cluster.  If the same access grant is added
 * multiple times, it will be treated as if it were added once (and removing
 * it once will remove it).
 */
- (void)addAccessGrant:(MTRAccessGrant *)accessGrant;

/**
 * Remove an access grant from the cluster.
 */
- (void)removeAccessGrant:(MTRAccessGrant *)accessGrant;

/**
 * Add an attribute to the cluster.  This can only be done before the endpoint
 * the cluster is a part of has been added to a controller.
 *
 * The attribute must not have the same attribute ID as another attribute in
 * this cluster.
 *
 * The attribute must not already be added to another cluster.
 *
 * If this cluster is the Descriptor cluster (id MTRClusterIDTypeDescriptorID),
 * it must not define any values for DeviceTypeList, ServerList, ClientList, PartsList;
 * those values will be determined automatically.
 *
 * For all clusters, the global AttributeList, AcceptedCommandList,
 * GeneratedCommandList attributes will be determined automatically and must not
 * be included in the attributes added on the cluster.
 *
 * For all clusters, the FeatureMap attribute will be assumed to be 0 unless
 * otherwise specified and may be omitted from the attributes added to the cluster.
 *
 * For all clusters, ClusterRevision will be determined automatically based on
 * this object's clusterRevision property, and must not be explicitly added to
 * the cluster.
 */
- (BOOL)addAttribute:(MTRServerAttribute *)attribute;

/**
 * Create a cluster description for the descriptor cluster.  This will set
 * clusterRevision to the current version implemented by Matter.framework.
 */
+ (MTRServerCluster *)newDescriptorCluster;

@property (atomic, copy, readonly) NSNumber * clusterID;

@property (atomic, copy, readonly) NSNumber * clusterRevision;

/**
 * The list of entities that are allowed to access this cluster instance.  This
 * list is in addition to any endpoint-wide access grants that exist.
 *
 * Defaults to empty list, which means no additional access grants.
 */
@property (atomic, copy, readonly) NSArray<MTRAccessGrant *> * accessGrants;

/**
 * The list of attributes supported by the cluster.
 */
@property (atomic, copy, readonly) NSArray<MTRServerAttribute *> * attributes;

@end

NS_ASSUME_NONNULL_END
