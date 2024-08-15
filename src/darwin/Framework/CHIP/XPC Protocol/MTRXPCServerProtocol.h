//
/**
 *    Copyright (c) 2023 Project CHIP Authors
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

@protocol MTRXPCServerProtocol_MTRDevice <NSObject>

- (MTRDeviceState) state;
- (BOOL) deviceCachePrimed;
- (NSDate *)estimatedStartTime;
- (NSNumber *) nodeID;

- (NSNumber *) estimatedSubscriptionLatency;

- (NSDictionary<NSString *, id> * _Nullable)readAttributeWithEndpointID:(NSNumber *)endpointID
                                                              clusterID:(NSNumber *)clusterID
                                                            attributeID:(NSNumber *)attributeID
                                                                 params:(MTRReadParams * _Nullable)params;

- (void)writeAttributeWithEndpointID:(NSNumber *)endpointID
                           clusterID:(NSNumber *)clusterID
                         attributeID:(NSNumber *)attributeID
                               value:(id)value
               expectedValueInterval:(NSNumber *)expectedValueInterval
                   timedWriteTimeout:(NSNumber * _Nullable)timeout;

- (void)invokeCommandWithEndpointID:(NSNumber *)endpointID
                          clusterID:(NSNumber *)clusterID
                          commandID:(NSNumber *)commandID
                      commandFields:(id)commandFields
                     expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues
              expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval
                 timedInvokeTimeout:(NSNumber * _Nullable)timeout
                              queue:(dispatch_queue_t)queue
                         completion:(MTRDeviceResponseHandler)completion;

- (void)openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode
                                   discriminator:(NSNumber *)discriminator
                                        duration:(NSNumber *)duration
                                           queue:(dispatch_queue_t)queue
                                      completion:(MTRDeviceOpenCommissioningWindowHandler)completion;

- (NSArray * _Nullable)clientDataKeys;
- (id<NSSecureCoding> _Nullable)clientDataForKey:(NSString *)key;
- (void)setClientDataForKey:(NSString *)key value:(id<NSSecureCoding>)value;
- (void)removeClientDataForKey:(NSString *)key;
- (NSArray * _Nullable)clientDataKeysForEndpointID:(NSNumber *)endpointID;
- (id<NSSecureCoding> _Nullable)clientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID;
- (void)setClientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID value:(id<NSSecureCoding>)value;
- (void)removeClientDataForKey:(NSString *)key endpointID:(NSNumber *)endpointID;
- (void)downloadLogOfType:(MTRDiagnosticLogType)type
                  timeout:(NSTimeInterval)timeout
                    queue:(dispatch_queue_t)queue
               completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;
@end

@protocol MTRXPCServerProtocol_MTRDeviceController <NSObject>

- (BOOL)isRunning;
- (NSUUID *)uniqueIdentifier;
- (NSNumber *)controllerNodeID;
- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error;

- (BOOL)setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult *)discoveredDevice
                                              payload:(MTRSetupPayload *)payload
                                            newNodeID:(NSNumber *)newNodeID
                                                error:(NSError * __autoreleasing *)error;

- (BOOL)commissionNodeWithID:(NSNumber *)nodeID
         commissioningParams:(MTRCommissioningParameters *)commissioningParams
                       error:(NSError * __autoreleasing *)error;

- (BOOL)continueCommissioningDevice:(void *)opaqueDeviceHandle
           ignoreAttestationFailure:(BOOL)ignoreAttestationFailure
                              error:(NSError * __autoreleasing *)error;

- (BOOL)cancelCommissioningForNodeID:(NSNumber *)nodeID
                               error:(NSError * __autoreleasing *)error;

- (nullable MTRBaseDevice *)deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID
                                                        error:(NSError * __autoreleasing *)error;

- (void)setDeviceControllerDelegate:(id<MTRDeviceControllerDelegate>)delegate
                              queue:(dispatch_queue_t)queue;

- (BOOL)startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate queue:(dispatch_queue_t)queue;

- (BOOL)stopBrowseForCommissionables;

- (NSData * _Nullable)attestationChallengeForDeviceID:(NSNumber *)deviceID;

- (BOOL)addServerEndpoint:(MTRServerEndpoint *)endpoint;

- (void)removeServerEndpoint:(MTRServerEndpoint *)endpoint queue:(dispatch_queue_t)queue completion:(dispatch_block_t)completion;

- (void)removeServerEndpoint:(MTRServerEndpoint *)endpoint;

- (void)shutdown;

@end

@protocol MTRXPCServerProtocol <NSObject, MTRXPCServerProtocol_MTRDevice, MTRXPCServerProtocol_MTRDeviceController>
- (void)checkInWithContext:(NSDictionary *)context;
@end

