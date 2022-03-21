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

#import <CHIP/CHIPDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

@interface CHIPAttributeCacheContainer : NSObject

/**
 * Subscribes to all attributes to update attribute cache.
 *
 * @param deviceController   device controller to retrieve connected device from
 * @param deviceId  device identifier of the device to cache attributes of
 * @param clientQueue  client queue to dispatch the completion handler through
 * @param completion  completion handler
 */
- (void)subscribeWithDeviceController:(CHIPDeviceController *)deviceController
                             deviceId:(uint64_t)deviceId
                          clientQueue:(dispatch_queue_t)clientQueue
                           completion:(void (^)(NSError * _Nullable error))completion;

/**
 * Reads an attribute with specific attribute path
 *
 * @param endpointId  endpoint ID of the attribute
 * @param clusterId  cluster ID of the attribute
 * @param attributeId  attribute ID of the attribute
 * @param clientQueue  client queue to dispatch the completion handler through
 * @param completion  block to receive the result.
 *                   "values" received by the block will have the same format of object as the one received by completion block
 *                   of CHIPDevice readAttributeWithEndpointId:clusterId:attributeId:clientQueue:completion method.
 */
- (void)readAttributeWithEndpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(void (^)(NSArray<NSDictionary<NSString *, id> *> * _Nullable values,
                                        NSError * _Nullable error))completion;

@end

NS_ASSUME_NONNULL_END
