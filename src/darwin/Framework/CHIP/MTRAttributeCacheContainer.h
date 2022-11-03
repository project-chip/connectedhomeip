/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import <Matter/MTRBaseDevice.h>
#import <Matter/MTRDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRSubscribeParams;

@interface MTRAttributeCacheContainer : NSObject

/**
 * Reads an attribute with specific attribute path
 *
 * @param endpointID  endpoint ID of the attribute
 * @param clusterID  cluster ID of the attribute
 * @param attributeID  attribute ID of the attribute
 * @param clientQueue  client queue to dispatch the completion handler through
 * @param completion  block to receive the result.
 *                   "values" received by the block will have the same format of object as the one received by completion block
 *                   of MTRBaseDevice readAttributeWithEndpointID:clusterID:attributeID:clientQueue:completion method.
 */
- (void)readAttributeWithEndpointID:(NSNumber * _Nullable)endpointID
                          clusterID:(NSNumber * _Nullable)clusterID
                        attributeID:(NSNumber * _Nullable)attributeID
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion MTR_NEWLY_AVAILABLE;

@end

@interface MTRAttributeCacheContainer (Deprecated)

- (void)readAttributeWithEndpointId:(NSNumber * _Nullable)endpointId
                          clusterId:(NSNumber * _Nullable)clusterId
                        attributeId:(NSNumber * _Nullable)attributeId
                        clientQueue:(dispatch_queue_t)clientQueue
                         completion:(MTRDeviceResponseHandler)completion
    MTR_NEWLY_DEPRECATED("Please use readAttributeWithEndpointID:clusterID:attributeID:clientQueue:completion:");

@end

NS_ASSUME_NONNULL_END
