/**
 *
 * SPDX-FileCopyrightText: 2020-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRCertificates.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

@interface MTRDeviceAttestationDeviceInfo : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * The vendor ID from the Device Attestation Certificate. May be nil only if attestation was unsuccessful.
 */
@property (nonatomic, readonly, nullable) NSNumber * vendorID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The product ID from the Device Attestation Certificate. May be nil only if attestation was unsuccessful.
 */
@property (nonatomic, readonly, nullable) NSNumber * productID API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The vendor ID value from the device's Basic Information cluster that was used
 * for device attestation.  If attestation succeeds, this must match the vendor
 * ID from the certification declaration.
 */
@property (nonatomic, readonly) NSNumber * basicInformationVendorID API_AVAILABLE(ios(16.6), macos(13.5), watchos(9.6), tvos(16.6));

/**
 * The product ID value from the device's Basic Information cluster that was
 * used for device attestation.  If attestation succeeds, this must match one of
 * the product IDs from the certification declaration.
 */
@property (nonatomic, readonly) NSNumber * basicInformationProductID API_AVAILABLE(ios(16.6), macos(13.5), watchos(9.6), tvos(16.6))
    ;

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
 * If -deviceAttestationFailedForController:opaqueDeviceHandle:error: is implemented, then it will
 * be called when device attestation fails, and the client can decide to continue or stop the
 * commissioning.
 *
 * If -deviceAttestationCompletedForController:opaqueDeviceHandle:attestationDeviceInfo:error: is
 * implemented, then it will always be called when device attestation completes.
 */

/**
 * Notify the delegate when device attestation completed with device info for additional verification. If
 * this callback is implemented, continueCommissioningDevice on MTRDeviceController is expected
 * to be called if commisioning should continue.
 *
 * This allows the delegate to stop commissioning after examining the device info (DAC, PAI, CD).
 *
 * @param controller Controller corresponding to the commissioning process
 * @param opaqueDeviceHandle Handle of device being commissioned
 * @param attestationDeviceInfo Attestation information for the device
 * @param error NSError representing the error code on attestation failure. Nil if success.
 */
- (void)deviceAttestationCompletedForController:(MTRDeviceController *)controller
                             opaqueDeviceHandle:(void *)opaqueDeviceHandle
                          attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                                          error:(NSError * _Nullable)error
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Notify the delegate when device attestation fails.  If this callback is implemented,
 * continueCommissioningDevice on MTRDeviceController is expected to be called if commisioning
 * should continue.
 *
 * @param controller Controller corresponding to the commissioning process
 * @param opaqueDeviceHandle Handle of device being commissioned
 * @param error NSError representing the error code for the failure
 */
- (void)deviceAttestationFailedForController:(MTRDeviceController *)controller
                          opaqueDeviceHandle:(void *)opaqueDeviceHandle
                                       error:(NSError * _Nonnull)error
    API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

- (void)deviceAttestation:(MTRDeviceController *)controller
       completedForDevice:(void *)device
    attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                    error:(NSError * _Nullable)error
    MTR_DEPRECATED("Please implement deviceAttestationCompletedForController:opaqueDeviceHandle:attestationDeviceInfo:error:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (void)deviceAttestation:(MTRDeviceController *)controller
          failedForDevice:(void *)device
                    error:(NSError * _Nonnull)error
    MTR_DEPRECATED("Please implement deviceAttestationFailedForController:opaqueDeviceHandle:error:", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

NS_ASSUME_NONNULL_END
