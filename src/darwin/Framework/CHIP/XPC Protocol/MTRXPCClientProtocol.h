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
#import <Matter/MTRDevice.h> // for MTRDeviceState

NS_ASSUME_NONNULL_BEGIN

MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2))
@protocol MTRXPCClientProtocol_MTRDevice <NSObject>
- (oneway void)device:(NSNumber *)nodeID stateChanged:(MTRDeviceState)state;
- (oneway void)device:(NSNumber *)nodeID receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport;
- (oneway void)device:(NSNumber *)nodeID receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport;
- (oneway void)deviceBecameActive:(NSNumber *)nodeID;
- (oneway void)deviceCachePrimed:(NSNumber *)nodeID;
- (oneway void)deviceConfigurationChanged:(NSNumber *)nodeID;
- (oneway void)device:(NSNumber *)nodeID internalStateUpdated:(NSDictionary *)dictionary;
@end

MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2))
@protocol MTRXPCClientProtocol_MTRDeviceController <NSObject>
// Not Supported via XPC
//- (oneway void)controller:(NSUUID *)controller statusUpdate:(MTRCommissioningStatus)status;
//- (oneway void)controller:(NSUUID *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error;
//- (oneway void)controller:(NSUUID *)controller commissioningComplete:(NSError * _Nullable)error nodeID:(NSNumber * _Nullable)nodeID metrics:(MTRMetrics * _Nullable)metrics;
//- (oneway void)controller:(NSUUID *)controller readCommissioningInfo:(MTRProductIdentity *)info;
@optional
- (oneway void)controller:(NSUUID *)controller controllerConfigurationUpdated:(NSDictionary *)configuration MTR_AVAILABLE(ios(18.3), macos(15.3), watchos(11.3), tvos(18.3));
@end

MTR_AVAILABLE(ios(18.2), macos(15.2), watchos(11.2), tvos(18.2))
@protocol MTRXPCClientProtocol <NSObject, MTRXPCClientProtocol_MTRDevice, MTRXPCClientProtocol_MTRDeviceController>
@end

NS_ASSUME_NONNULL_END
