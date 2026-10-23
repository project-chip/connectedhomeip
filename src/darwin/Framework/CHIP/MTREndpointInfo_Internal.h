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

#import "MTREndpointInfo_Test.h"

#include <app/AttributePathParams.h>
#include <app/ClusterStateCache.h>
#include <lib/support/Span.h>

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@interface MTREndpointInfo ()

/**
 * Returns a dictionary of endpoint metadata for a node.
 *
 * The provided cache must contain the result of reading the
 * DeviceTypeList and PartsList attributes of all endpoints
 * (as exposed by `requiredAttributePaths`).
 *
 * Any relevant information will be copied out of the cache;
 * the caller is free to deallocate the cache once this method returns.
 */
+ (NSDictionary<NSNumber *, MTREndpointInfo *> *)endpointsFromAttributeCache:(const chip::app::ClusterStateCache *)cache;

/**
 * Returns the set of AttributePathParams that must be read
 * to populate endpoint information for a node.
 */
+ (chip::Span<const chip::app::AttributePathParams>)requiredAttributePaths;

@end

NS_ASSUME_NONNULL_END
