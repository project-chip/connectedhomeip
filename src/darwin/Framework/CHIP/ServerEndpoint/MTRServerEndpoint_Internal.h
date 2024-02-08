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
#import <Matter/MTRServerEndpoint.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRServerEndpoint ()

/**
 * Mark this endpoint as associated with a particular controller.  The
 * controller can be nil to indicate that the endpoint is not associated with a
 * specific controller but rather with the controller factory.
 *
 * On failure, this method ensures that it undoes any state changes it made.
 */
- (BOOL)associateWithController:(nullable MTRDeviceController *)controller;

/**
 * Register this endpoint.  Always called on the Matter queue.
 */
- (void)registerMatterEndpoint;

/**
 * Unregister this endpoint.  Always called on the Matter queue.
 */
- (void)unregisterMatterEndpoint;

/**
 * Mark this endpoint as no longer being in use.  Can run on any thread, but
 * will either be called before registerMatterEndpoint or after
 * unregisterMatterEndpoint.  This undoes anything associateWithController did.
 */
- (void)invalidate;

/**
 * Get an MTRServerEndpoint for the root node endpoint.  This can't be done via
 * the public initializer, since we don't allow that to create an
 * MTRServerEndpoint for endpoint 0.
 */
+ (MTRServerEndpoint *)rootNodeEndpoint;

/**
 * Returns the list of access grants applicable to the given cluster ID on this
 * endpoint.  Only called on the Matter queue.
 */
- (NSArray<MTRAccessGrant *> *)matterAccessGrantsForCluster:(NSNumber *)clusterID;

/**
 * The access grants the Matter stack can observe.  Only modified while in
 * Initializing state or on the Matter queue.
 */
@property (nonatomic, strong, readonly) NSSet<MTRAccessGrant *> * matterAccessGrants;

@end

NS_ASSUME_NONNULL_END
