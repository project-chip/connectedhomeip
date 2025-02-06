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

#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

MTR_EXTERN NSString * const MTRDeviceControllerRegistrationNodeIDsKey MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2));
MTR_EXTERN NSString * const MTRDeviceControllerRegistrationNodeIDKey MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2));
MTR_EXTERN NSString * const MTRDeviceControllerRegistrationControllerContextKey MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2));
MTR_EXTERN NSString * const MTRDeviceControllerRegistrationControllerNodeIDKey MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));
MTR_EXTERN NSString * const MTRDeviceControllerRegistrationControllerIsRunningKey MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));
MTR_EXTERN NSString * const MTRDeviceControllerRegistrationDeviceInternalStateKey MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));
MTR_EXTERN NSString * const MTRDeviceControllerRegistrationControllerCompressedFabricIDKey MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));

MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2))
@protocol MTRXPCServerProtocol_MTRDevice <NSObject>

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID getStateWithReply:(void (^)(MTRDeviceState state))reply;
- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID getDeviceCachePrimedWithReply:(void (^)(BOOL primed))reply;
- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID getEstimatedStartTimeWithReply:(void (^)(NSDate * _Nullable estimatedStartTime))reply;
- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID getEstimatedSubscriptionLatencyWithReply:(void (^)(NSNumber * _Nullable estimatedSubscriptionLatency))reply;

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID params:(MTRReadParams * _Nullable)params withReply:(void (^)(NSDictionary<NSString *, id> * _Nullable))reply;
- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID writeAttributeWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID attributeID:(NSNumber *)attributeID value:(id)value expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedWriteTimeout:(NSNumber * _Nullable)timeout;

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID readAttributePaths:(NSArray<MTRAttributeRequestPath *> *)attributePaths withReply:(void (^)(NSArray<NSDictionary<NSString *, id> *> *))reply;

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommandWithEndpointID:(NSNumber *)endpointID clusterID:(NSNumber *)clusterID commandID:(NSNumber *)commandID commandFields:(id)commandFields expectedValues:(NSArray<NSDictionary<NSString *, id> *> * _Nullable)expectedValues expectedValueInterval:(NSNumber * _Nullable)expectedValueInterval timedInvokeTimeout:(NSNumber * _Nullable)timeout serverSideProcessingTimeout:(NSNumber * _Nullable)serverSideProcessingTimeout
                     completion:(MTRDeviceResponseHandler)completion;

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID openCommissioningWindowWithSetupPasscode:(NSNumber *)setupPasscode discriminator:(NSNumber *)discriminator duration:(NSNumber *)duration completion:(MTRDeviceOpenCommissioningWindowHandler)completion;

- (oneway void)downloadLogOfType:(MTRDiagnosticLogType)type nodeID:(NSNumber *)nodeID timeout:(NSTimeInterval)timeout completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;

@optional
/* Note: The consumer of the completion block should move the file that the url points to or open it for reading before the
 * completion handler returns. Otherwise, the file will be deleted, and the data will be lost.
 */
- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID downloadLogOfType:(MTRDiagnosticLogType)type timeout:(NSTimeInterval)timeout completion:(void (^)(NSURL * _Nullable url, NSError * _Nullable error))completion;

- (oneway void)deviceController:(NSUUID *)controller nodeID:(NSNumber *)nodeID invokeCommands:(NSArray<NSArray<MTRCommandWithRequiredResponse *> *> *)commands completion:(MTRDeviceResponseHandler)completion MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

@end

MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3))
@protocol MTRXPCServerProtocol_MTRDeviceController <NSObject>

@optional
// - (oneway void)deviceController:(NSUUID *)controller setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload newNodeID:(NSNumber *)newNodeID withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller setupCommissioningSessionWithDiscoveredDevice:(MTRCommissionableBrowserResult *)discoveredDevice payload:(MTRSetupPayload *)payload newNodeID:(NSNumber *)newNodeID withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller commissionNodeWithID:(NSNumber *)nodeID commissioningParams:(MTRCommissioningParameters *)commissioningParams withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller continueCommissioningDevice:(void *)opaqueDeviceHandle ignoreAttestationFailure:(BOOL)ignoreAttestationFailure withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;
// - (oneway void)deviceController:(NSUUID *)controller cancelCommissioningForNodeID:(NSNumber *)nodeID withReply:(void (^)(BOOL success, NSError * _Nullable error))reply;
// - (nullable MTRBaseDevice *)deviceController:(NSUUID *)controller deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error;
// - (oneway void)deviceController:(NSUUID *)controller startBrowseForCommissionables:(id<MTRCommissionableBrowserDelegate>)delegate withReply:(void (^)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID *)controller stopBrowseForCommissionablesWithReply:(void (^)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID *)controller attestationChallengeForDeviceID:(NSNumber *)deviceID withReply:(void (^)(NSData * _Nullable))reply;

// - (oneway void)deviceController:(NSUUID *)controller addServerEndpoint:(MTRServerEndpoint *)endpoint withReply:(void (^)(BOOL success))reply;
// - (oneway void)deviceController:(NSUUID *)controller removeServerEndpoint:(MTRServerEndpoint *)endpoint;

- (oneway void)deviceController:(NSUUID *)controller deleteNodeID:(NSNumber *)nodeID MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));

- (oneway void)deviceController:(NSUUID *)controller registerNodeID:(NSNumber *)nodeID;
- (oneway void)deviceController:(NSUUID *)controller unregisterNodeID:(NSNumber *)nodeID;
- (oneway void)deviceController:(NSUUID *)controller updateControllerConfiguration:(NSDictionary *)controllerState MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));

- (oneway void)deviceController:(NSUUID *)controller getNodesWithStoredDataWithReply:(void (^)(NSArray<NSNumber *> *))reply MTR_AVAILABLE(ios(18.4), macos(15.4), watchos(11.4), tvos(18.4));
@end

MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3))
@protocol MTRXPCServerProtocol <NSObject, MTRXPCServerProtocol_MTRDevice, MTRXPCServerProtocol_MTRDeviceController>
@optional
- (oneway void)deviceController:(NSUUID *)controller checkInWithContext:(NSDictionary *)context;
@end

NS_ASSUME_NONNULL_END
