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
- (void)device:(MTRDevice *)device stateChanged:(MTRDeviceState)state;
- (void)device:(MTRDevice *)device receivedAttributeReport:(NSArray<NSDictionary<NSString *, id> *> *)attributeReport;
- (void)device:(MTRDevice *)device receivedEventReport:(NSArray<NSDictionary<NSString *, id> *> *)eventReport;
- (void)deviceBecameActive:(MTRDevice *)device;
- (void)deviceCachePrimed:(MTRDevice *)device;
- (void)deviceConfigurationChanged:(MTRDevice *)device;
@end

@protocol MTRXPCClientProtocol_MTRDeviceController <NSObject>
- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status;
- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error;
- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError * _Nullable)error nodeID:(NSNumber * _Nullable)nodeID metrics:(MTRMetrics *)metrics;
- (void)controller:(MTRDeviceController *)controller readCommissioningInfo:(MTRProductIdentity *)info;
@end

@protocol MTRXPCClientProtocol <NSObject, MTRXPCClientProtocol_MTRDevice, MTRXPCClientProtocol_MTRDeviceController>
@end

