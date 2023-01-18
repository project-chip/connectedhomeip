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
#import <Matter/MTRCertificates.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

@interface MTRDeviceAttestationDeviceInfo : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * The vendor ID for the device from the Device Attestation Certificate. May be nil only if attestation was unsucessful.
 */
@property (nonatomic, readonly, nullable) NSNumber * vendorID MTR_NEWLY_AVAILABLE;

/**
 * The product ID for the device from the Device Attestation Certificate. May be nil only if attestation was unsucessful.
 */
@property (nonatomic, readonly, nullable) NSNumber * productID MTR_NEWLY_AVAILABLE;

@property (nonatomic, readonly) MTRCertificateDERBytes dacCertificate;
@property (nonatomic, readonly) MTRCertificateDERBytes dacPAICertificate;
@property (nonatomic, readonly, nullable) NSData * certificateDeclaration;

@end

/**
 * The protocol definition for the MTRDeviceAttestationDelegate.
 */
@protocol MTRDeviceAttestationDelegate <NSObject>
@optional
/**
 * Only one of the following delegate callbacks should be implemented.
 *
 * If -deviceAttestationFailedForController:device:error: is implemented, then it will be called when device
 * attestation fails, and the client can decide to continue or stop the commissioning.
 *
 * If -deviceAttestationCompletedForController:device:attestationDeviceInfo:error: is implemented, then it
 * will always be called when device attestation completes.
 */

/**
 * Notify the delegate when device attestation completed with device info for additional verification. If
 * this callback is implemented, continueCommissioningDevice on MTRDeviceController is expected
 * to be called if commisioning should continue.
 *
 * This allows the delegate to stop commissioning after examining the device info (DAC, PAI, CD).
 *
 * @param controller Controller corresponding to the commissioning process
 * @param device Handle of device being commissioned
 * @param attestationDeviceInfo Attestation information for the device
 * @param error NSError representing the error code on attestation failure. Nil if success.
 */
- (void)deviceAttestationCompletedForController:(MTRDeviceController *)controller
                                         device:(void *)device
                          attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                                          error:(NSError * _Nullable)error MTR_NEWLY_AVAILABLE;

/**
 * Notify the delegate when device attestation fails
 *
 * @param controller Controller corresponding to the commissioning process
 * @param device Handle of device being commissioned
 * @param error NSError representing the error code for the failure
 */
- (void)deviceAttestationFailedForController:(MTRDeviceController *)controller
                                      device:(void *)device
                                       error:(NSError * _Nonnull)error MTR_NEWLY_AVAILABLE;

- (void)deviceAttestation:(MTRDeviceController *)controller
       completedForDevice:(void *)device
    attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                    error:(NSError * _Nullable)error
    MTR_NEWLY_DEPRECATED("Please implement deviceAttestationCompletedForController:device:attestationDeviceInfo:error:");

- (void)deviceAttestation:(MTRDeviceController *)controller
          failedForDevice:(void *)device
                    error:(NSError * _Nonnull)error
    MTR_NEWLY_DEPRECATED("Please implement deviceAttestationFailedForController:device:error:");

@end

NS_ASSUME_NONNULL_END
