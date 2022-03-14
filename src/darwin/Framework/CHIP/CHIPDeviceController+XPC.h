/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * Extended methods for CHIPDeviceController object over XPC
 */
@interface CHIPDeviceController (XPC)

/**
 * Returns a shared device controller proxy for the controller object over XPC connection.
 *
 * @param controllerId  an implementation specific id in case multiple shared device controllers are available over XPC connection
 * @param connectBlock  block to connect to an XPC listener serving the shared device controllers in an implementation specific way
 */
+ (CHIPDeviceController *)sharedControllerWithId:(id<NSCopying> _Nullable)controllerId
                                 xpcConnectBlock:(NSXPCConnection * (^)(void) )connectBlock;

@end

/**
 * Protocol that remote object must support over XPC
 */
@protocol CHIPDeviceControllerServerProtocol <NSObject>

/**
 * Gets device controller ID corresponding to a specific fabric Id
 */
- (void)getDeviceControllerWithFabricId:(uint64_t)fabricId
                             completion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion;

/**
 * Gets any available device controller ID
 */
- (void)getAnyDeviceControllerWithCompletion:(void (^)(id _Nullable controller, NSError * _Nullable error))completion;

/**
 * Requests reading attribute
 */
- (void)readAttributeWithController:(id _Nullable)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                        attributeId:(NSUInteger)attributeId
                         completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion;

/**
 * Requests writing attribute
 */
- (void)writeAttributeWithController:(id _Nullable)controller
                              nodeId:(uint64_t)nodeId
                          endpointId:(NSUInteger)endpointId
                           clusterId:(NSUInteger)clusterId
                         attributeId:(NSUInteger)attributeId
                               value:(id)value
                          completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion;

/**
 * Requests invoking command
 */
- (void)invokeCommandWithController:(id _Nullable)controller
                             nodeId:(uint64_t)nodeId
                         endpointId:(NSUInteger)endpointId
                          clusterId:(NSUInteger)clusterId
                          commandId:(NSUInteger)commandId
                             fields:(id)fields
                         completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion;

/**
 * Requests subscribing attribute
 */
- (void)subscribeAttributeWithController:(id _Nullable)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSUInteger)endpointId
                               clusterId:(NSUInteger)clusterId
                             attributeId:(NSUInteger)attributeId
                             minInterval:(NSUInteger)minInterval
                             maxInterval:(NSUInteger)maxInterval
                      establishedHandler:(void (^)(void))establishedHandler;

/**
 * Requests a specific node attribute subscription into a cache
 */
- (void)subscribeAttributeCacheWithController:(id _Nullable)controller
                                       nodeId:(uint64_t)nodeId
                                   completion:(void (^)(NSError * _Nullable error))completion;

/**
 * Requests reading attribute cache
 */
- (void)readAttributeCacheWithController:(id _Nullable)controller
                                  nodeId:(uint64_t)nodeId
                              endpointId:(NSUInteger)endpointId
                               clusterId:(NSUInteger)clusterId
                             attributeId:(NSUInteger)attributeId
                              completion:(void (^)(id _Nullable values, NSError * _Nullable error))completion;

@end

/**
 * Protocol that the XPC client local object must support
 */
@protocol CHIPDeviceControllerClientProtocol <NSObject>

/**
 * Handles a report received by a device controller
 */
- (void)handleReportWithController:(id _Nullable)controller
                            nodeId:(uint64_t)nodeId
                             value:(id _Nullable)value
                             error:(NSError * _Nullable)error;

@end

NS_ASSUME_NONNULL_END
