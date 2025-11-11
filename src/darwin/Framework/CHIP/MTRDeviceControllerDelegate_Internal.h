/**
 *    Copyright (c) 2025 Project CHIP Authors
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
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRDeviceControllerDelegate.h>
#import <Matter/MTRSetupPayload.h>
#import <Matter/MTRStructsObjc.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MTRDeviceControllerDelegate_Internal <MTRDeviceControllerDelegate>
@required
- (void)controller:(MTRDeviceController *)controller
    needsWiFiCredentialsWithScanResults:(nullable NSArray<MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct *> *)networks
                                  error:(nullable NSError *)error;
- (void)controller:(MTRDeviceController *)controller
    needsThreadCredentialsWithScanResults:(nullable NSArray<MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct *> *)networks
                                    error:(nullable NSError *)error;
- (void)controllerStartingNetworkScan:(MTRDeviceController *)controller;
- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error forPayload:(MTRSetupPayload * _Nullable)payload;
@end
NS_ASSUME_NONNULL_END
