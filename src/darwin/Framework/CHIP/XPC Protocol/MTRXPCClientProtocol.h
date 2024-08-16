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

@protocol MTRXPCClientProtocol_MTRDevice <NSObject>
- (oneway void)device:(NSNumber * _Nonnull)nodeID stateChanged:(MTRDeviceState)state;
- (oneway void)device:(NSNumber * _Nonnull)nodeID receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> * _Nonnull)attributeReport;
- (oneway void)device:(NSNumber * _Nonnull)nodeID receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> * _Nonnull)eventReport;
- (oneway void)deviceBecameActive:(NSNumber * _Nonnull)nodeID;
- (oneway void)deviceCachePrimed:(NSNumber * _Nonnull)nodeID;
- (oneway void)deviceConfigurationChanged:(NSNumber * _Nonnull)nodeID;
@end

@protocol MTRXPCClientProtocol_MTRDeviceController <NSObject>
// Not Supported via XPC
//- (oneway void)controller:(NSUUID * _Nonnull)controller statusUpdate:(MTRCommissioningStatus)status;
//- (oneway void)controller:(NSUUID * _Nonnull)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error;
//- (oneway void)controller:(NSUUID * _Nonnull)controller commissioningComplete:(NSError * _Nullable)error nodeID:(NSNumber * _Nullable)nodeID metrics:(MTRMetrics * _Nullable)metrics;
//- (oneway void)controller:(NSUUID * _Nonnull)controller readCommissioningInfo:(MTRProductIdentity * _Nonnull)info;
@end

@protocol MTRXPCClientProtocol <NSObject, MTRXPCClientProtocol_MTRDevice, MTRXPCClientProtocol_MTRDeviceController>
@end
