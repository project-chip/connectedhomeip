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

- (oneway void)getStateWithReply:(void(^ _Nonnull)(MTRDeviceState state))reply;
- (oneway void)getDeviceCachePrimedWithReply:(void(^ _Nonnull)(BOOL primed))reply;
- (oneway void)getEstimatedStartTimeWithReply:(void(^ _Nonnull)(NSDate * _Nullable estimatedStartTime))reply;
- (oneway void)getNodeIDWithReply:(void(^ _Nonnull)(NSNumber * _Nonnull nodeID))reply;
- (oneway void)getEstimatedSubscriptionLatencyWithReply:(void(^ _Nonnull)(NSNumber * _Nullable estimatedSubscriptionLatency))reply;

- (oneway void)readAttributeWithEndpointID:(NSNumber * _Nonnull)endpointID clusterID:(NSNumber * _Nonnull)clusterID attributeID:(NSNumber * _Nonnull)attributeID params:(MTRReadParams * _Nullable)params withReply:(void(^_Nonnull)(NSDictionary<NSString *, id> * _Nullable))reply;
- (oneway void)writeAttributeWithEndpointID:(NSNumber * _Nonnull)endpointID clusterID:(NSNumber * _Nonnull)clusterID attributeID:(NSNumber * _Nonnull)attributeID value:(id _Nullable)value expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedWriteTimeout:(NSNumber * _Nullable)timeout;

- (oneway void)invokeCommandWithEndpointID:(NSNumber * _Nonnull)endpointID clusterID:(NSNumber * _Nonnull)clusterID commandID:(NSNumber * _Nonnull)commandID commandFields:(id _Nonnull)commandFields expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedInvokeTimeout:(NSNumber * _Nullable)timeout completion:(MTRDeviceResponseHandler _Nonnull)completion;

// Not Supported via XPC
//- (oneway void)openCommissioningWindowWithSetupPasscode:(NSNumber * _Nonnull)setupPasscode discriminator:(NSNumber * _Nonnull)discriminator duration:(NSNumber * _Nonnull)duration completion:(MTRDeviceOpenCommissioningWindowHandler _Nonnull)completion;

- (oneway void)getClientDataKeysWithReply:(void(^ _Nonnull)(NSArray * _Nullable))reply;
- (oneway void)clientDataForKey:(NSString * _Nonnull)key withReply:(void(^ _Nonnull)(id<NSSecureCoding> _Nullable))reply;
- (oneway void)setClientDataForKey:(NSString * _Nonnull)key value:(id<NSSecureCoding> _Nonnull)value;
- (oneway void)removeClientDataForKey:(NSString * _Nonnull)key;
- (oneway void)clientDataKeysForEndpointID:(NSNumber * _Nonnull)endpointID withReply:(void(^ _Nonnull)(NSArray * _Nullable))reply;
- (oneway void)clientDataForKey:(NSString * _Nonnull)key endpointID:(NSNumber * _Nonnull)endpointID withReply:(void(^ _Nonnull)(id<NSSecureCoding> _Nullable))reply;
- (oneway void)setClientDataForKey:(NSString * _Nonnull)key endpointID:(NSNumber * _Nonnull)endpointID value:(id<NSSecureCoding> _Nullable)value;
- (oneway void)removeClientDataForKey:(NSString * _Nonnull)key endpointID:(NSNumber * _Nonnull)endpointID;

// Not Supported via XPC
// - (oneway void)downloadLogOfType:(MTRDiagnosticLogType)type timeout:(NSTimeInterval)timeout completion:(void (^ _Nonnull)(NSURL * _Nullable url, NSError * _Nullable error))completion;
@end

@protocol MTRXPCServerProtocol_MTRDeviceController <NSObject>

- (oneway void)getIsRunningWithReply:(void(^ _Nonnull)(BOOL response))reply;
- (oneway void)getUniqueIdentifierWithReply:(void(^ _Nonnull)(NSUUID * _Nonnull))reply;
- (oneway void)controllerNodeIDWithReply:(void(^ _Nonnull)(NSNumber * _Nonnull nodeID))reply;

// Not Supported via XPC
// - (oneway void)setupCommissioningSessionWithPayload:(MTRSetupPayload * _Nonnull)payload newNodeID:(NSNumber * _Nonnull)newNodeID withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult * _Nonnull)discoveredDevice payload:(MTRSetupPayload * _Nonnull)payload newNodeID:(NSNumber * _Nonnull)newNodeID withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)commissionNodeWithID:(NSNumber * _Nonnull)nodeID commissioningParams:(MTRCommissioningParameters * _Nonnull)commissioningParams withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)continueCommissioningDevice:(void * _Nonnull)opaqueDeviceHandle ignoreAttestationFailure:(BOOL)ignoreAttestationFailure withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)cancelCommissioningForNodeID:(NSNumber * _Nonnull)nodeID withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (nullable MTRBaseDevice *)deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error;
// - (oneway void)startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate withReply:(void(^ _Nonnull)(BOOL success))reply;
// - (oneway void )stopBrowseForCommissionablesWithReply:(void(^ _Nonnull)(BOOL success))reply;
// - (oneway void)attestationChallengeForDeviceID:(NSNumber * _Nonnull)deviceID withReply:(void(^ _Nonnull)(NSData * _Nullable))reply;

//- (oneway void)addServerEndpoint:(MTRServerEndpoint * _Nonnull)endpoint withReply:(void(^ _Nonnull)(BOOL success))reply;
//- (oneway void)removeServerEndpoint:(MTRServerEndpoint * _Nonnull)endpoint;

- (oneway void)shutdown;

@end

@protocol MTRXPCServerProtocol <NSObject, MTRXPCServerProtocol_MTRDevice, MTRXPCServerProtocol_MTRDeviceController>
- (oneway void)checkInWithContext:(NSDictionary * _Nonnull)context;
@end
