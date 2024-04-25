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

#import <Matter/MTRDeviceController.h>
#import <Matter/MTRServerCluster.h>

#include <lib/core/DataModelTypes.h>

// TODO: These attribute-*.h and Span bits are a hack that should eventually go away.
#include <app/util/attribute-metadata.h>
#include <lib/support/Span.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRServerCluster ()

/**
 * Mark this cluster as associated with a particular controller.  The
 * controller can be nil to indicate that the endpoint is not associated with a
 * specific controller but rather with the controller factory.  This method does
 * NOT perform any cleanup on failure; it's the caller's responsibility to call
 * invalidate if it fails.
 */
- (BOOL)associateWithController:(nullable MTRDeviceController *)controller;

/**
 * Register this cluster.  Always called on the Matter queue.
 */
- (void)registerMatterCluster;

/**
 * Unregister this cluster.  Always called on the Matter queue.
 */
- (void)unregisterMatterCluster;

/**
 * Mark this cluster as part of an endpoint that is no longer being used.  Can
 * run on any thread, but will either be called before registerMatterCluster or
 * after unregisterMatterCluster.  This undoes anything associateWithController
 * did.
 */
- (void)invalidate;

/**
 * Add the cluster to an endpoint with the given endpoint ID.  Will return NO
 * if the cluster is already added to an endpoint.
 */
- (BOOL)addToEndpoint:(chip::EndpointId)endpoint;

/**
 * The access grants the Matter stack can observe.  Only modified while
 * associating with a controller or on the Matter queue.
 */
@property (atomic, copy, readonly) NSArray<MTRAccessGrant *> * matterAccessGrants;

/**
 * parentEndpoint will be kInvalidEndpointId until the cluster is added to an endpoint.
 */
@property (atomic, assign, readonly) chip::EndpointId parentEndpoint;

/**
 * The attribute metadata for the cluster.  Only valid after associateWithController: has succeeded.
 */
@property (atomic, assign, readonly) chip::Span<const EmberAfAttributeMetadata> matterAttributeMetadata;

/**
 * The list of accepted command IDs.
 */
@property (atomic, copy, nullable) NSArray<NSNumber *> * acceptedCommands;

/**
 * The list of generated command IDs.
 */
@property (atomic, copy, nullable) NSArray<NSNumber *> * generatedCommands;

/**
 * The list of accepted commands IDs in the format the Matter stack needs.
 */
@property (atomic, assign, nullable, readonly) chip::CommandId * matterAcceptedCommands;

/**
 * The list of generated commands IDs in the format the Matter stack needs.
 */
@property (atomic, assign, nullable, readonly) chip::CommandId * matterGeneratedCommands;

@end

NS_ASSUME_NONNULL_END
