/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRSubscribeParams;

/**
 * An object that holds a cluster state cache.  It can be passed to
 * MTRBaseDevice's subscribeWithQueue to fill the cache with data the
 * subscription returns.  Then reads can happen against the cache without going
 * out to the network.
 */
@interface MTRClusterStateCacheContainer : NSObject

/**
 * Reads the given attribute path from the cluster state cache inside
 * this cache container.
 *
 * @param endpointID  endpoint ID of the attribute.  nil means wildcard.
 * @param clusterID  cluster ID of the attribute  nil means wildcard.
 * @param attributeID  attribute ID of the attribute.  nil means wildcard.
 * @param queue  client queue to dispatch the completion handler through
 * @param completion  block to receive the result.
 *                   "values" received by the block will have the same format of object as the one received by the completion block
 *                   of the MTRBaseDevice readAttributePathWithEndpointID:clusterID:attributeID:queue:completion method.
 *
 * @note: not all combinations of wildcards might be supported.
 */
- (void)readAttributePathWithEndpointID:(NSNumber * _Nullable)endpointID
                              clusterID:(NSNumber * _Nullable)clusterID
                            attributeID:(NSNumber * _Nullable)attributeID
                                  queue:(dispatch_queue_t)queue
                             completion:(MTRDeviceResponseHandler)completion;

@end

NS_ASSUME_NONNULL_END
