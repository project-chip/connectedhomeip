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
#import <Matter/MTRServerAttribute.h>

#include <app/ConcreteClusterPath.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRServerAttribute ()

/**
 * Mark this attribute as associated with a particular controller.  The
 * controller can be nil to indicate that the endpoint is not associated with a
 * specific controller but rather with the controller factory.
 */
- (BOOL)associateWithController:(nullable MTRDeviceController *)controller;

/**
 * Mark this attribute as part of an endpoint that is no longer being used.
 */
- (void)invalidate;

/**
 * Add the attribute to a cluster with the given cluster path.  Will return NO
 * if the attribute is already added to a cluster.
 */
- (BOOL)addToCluster:(const chip::app::ConcreteClusterPath &)cluster;

/**
 * Update the parent cluster path of the attribute.  Should only be done for
 * attributes that are already added to a cluster, when the endpoint id needs to
 * be updated.
 */
- (void)updateParentCluster:(const chip::app::ConcreteClusterPath &)cluster;

/**
 * serializedValue is either an NSData or an NSArray<NSData *>, depending on
 * whether the attribute is list-typed.
 */
@property (nonatomic, strong, readonly) id serializedValue;

/**
 * parentCluster will have kInvalidClusterId for the cluster ID until the
 * attribute is added to a cluster.
 */
@property (nonatomic, assign, readonly) const chip::app::ConcreteClusterPath & parentCluster;

@end

NS_ASSUME_NONNULL_END
