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

- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID getStateWithReply:(void (^_Nonnull)(MTRDeviceState state))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID getDeviceCachePrimedWithReply:(void (^_Nonnull)(BOOL primed))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID getEstimatedStartTimeWithReply:(void (^_Nonnull)(NSDate * _Nullable estimatedStartTime))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID getEstimatedSubscriptionLatencyWithReply:(void (^_Nonnull)(NSNumber * _Nullable estimatedSubscriptionLatency))reply;

- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID readAttributeWithEndpointID:(NSNumber * _Nonnull)endpointID clusterID:(NSNumber * _Nonnull)clusterID attributeID:(NSNumber * _Nonnull)attributeID params:(MTRReadParams * _Nullable)params withReply:(void (^_Nonnull)(NSDictionary<NSString *, id> * _Nullable))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID writeAttributeWithEndpointID:(NSNumber * _Nonnull)endpointID clusterID:(NSNumber * _Nonnull)clusterID attributeID:(NSNumber * _Nonnull)attributeID value:(id _Nullable)value expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedWriteTimeout:(NSNumber * _Nullable)timeout;

- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID invokeCommandWithEndpointID:(NSNumber * _Nonnull)endpointID clusterID:(NSNumber * _Nonnull)clusterID commandID:(NSNumber * _Nonnull)commandID commandFields:(id _Nonnull)commandFields expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedInvokeTimeout:(NSNumber * _Nullable)timeout completion:(MTRDeviceResponseHandler _Nonnull)completion;

// Not Supported via XPC
//- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID openCommissioningWindowWithSetupPasscode:(NSNumber * _Nonnull)setupPasscode discriminator:(NSNumber * _Nonnull)discriminator duration:(NSNumber * _Nonnull)duration completion:(MTRDeviceOpenCommissioningWindowHandler _Nonnull)completion;

- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID getClientDataKeysWithReply:(void (^_Nonnull)(NSArray * _Nullable))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID clientDataForKey:(NSString * _Nonnull)key withReply:(void (^_Nonnull)(id<NSSecureCoding> _Nullable))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID setClientDataForKey:(NSString * _Nonnull)key value:(id<NSSecureCoding> _Nonnull)value;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID removeClientDataForKey:(NSString * _Nonnull)key;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID clientDataKeysForEndpointID:(NSNumber * _Nonnull)endpointID withReply:(void (^_Nonnull)(NSArray * _Nullable))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID clientDataForKey:(NSString * _Nonnull)key endpointID:(NSNumber * _Nonnull)endpointID withReply:(void (^_Nonnull)(id<NSSecureCoding> _Nullable))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID setClientDataForKey:(NSString * _Nonnull)key endpointID:(NSNumber * _Nonnull)endpointID value:(id<NSSecureCoding> _Nullable)value;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller nodeID:(NSNumber * _Nonnull)nodeID removeClientDataForKey:(NSString * _Nonnull)key endpointID:(NSNumber * _Nonnull)endpointID;

// Not Supported via XPC
// - (oneway void)downloadLogOfType:(MTRDiagnosticLogType)type nodeID:(NSNumber * _Nonnull)nodeID timeout:(NSTimeInterval)timeout completion:(void (^ _Nonnull)(NSURL * _Nullable url, NSError * _Nullable error))completion;
@end

@protocol MTRXPCServerProtocol_MTRDeviceController <NSObject>

- (oneway void)deviceController:(NSUUID * _Nonnull)controller getIsRunningWithReply:(void (^_Nonnull)(BOOL response))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller getUniqueIdentifierWithReply:(void (^_Nonnull)(NSUUID * _Nonnull))reply;
- (oneway void)deviceController:(NSUUID * _Nonnull)controller controllerNodeIDWithReply:(void (^_Nonnull)(NSNumber * _Nonnull nodeID))reply;

// Not Supported via XPC
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller setupCommissioningSessionWithPayload:(MTRSetupPayload * _Nonnull)payload newNodeID:(NSNumber * _Nonnull)newNodeID withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult * _Nonnull)discoveredDevice payload:(MTRSetupPayload * _Nonnull)payload newNodeID:(NSNumber * _Nonnull)newNodeID withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller commissionNodeWithID:(NSNumber * _Nonnull)nodeID commissioningParams:(MTRCommissioningParameters * _Nonnull)commissioningParams withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller continueCommissioningDevice:(void * _Nonnull)opaqueDeviceHandle ignoreAttestationFailure:(BOOL)ignoreAttestationFailure withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller cancelCommissioningForNodeID:(NSNumber * _Nonnull)nodeID withReply:(void(^ _Nonnull)(BOOL success, NSError * _Nullable error))reply;
// - (nullable MTRBaseDevice *)deviceController:(NSUUID * _Nonnull)controller deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error;
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate withReply:(void(^ _Nonnull)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller stopBrowseForCommissionablesWithReply:(void(^ _Nonnull)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID * _Nonnull)controller attestationChallengeForDeviceID:(NSNumber * _Nonnull)deviceID withReply:(void(^ _Nonnull)(NSData * _Nullable))reply;

//- (oneway void)deviceController:(NSUUID * _Nonnull)controller addServerEndpoint:(MTRServerEndpoint * _Nonnull)endpoint withReply:(void(^ _Nonnull)(BOOL success))reply;
//- (oneway void)deviceController:(NSUUID * _Nonnull)controller removeServerEndpoint:(MTRServerEndpoint * _Nonnull)endpoint;

- (oneway void)shutdownDeviceController:(NSUUID * _Nonnull)controller;

@end

@protocol MTRXPCServerProtocol <NSObject, MTRXPCServerProtocol_MTRDevice, MTRXPCServerProtocol_MTRDeviceController>
- (oneway void)deviceController:(NSUUID * _Nonnull)controller checkInWithContext:(NSDictionary * _Nonnull)context;
@end
