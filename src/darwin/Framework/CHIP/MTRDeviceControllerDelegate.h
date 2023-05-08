/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

typedef NS_ENUM(NSInteger, MTRCommissioningStatus) {
    MTRCommissioningStatusUnknown = 0,
    MTRCommissioningStatusSuccess = 1,
    MTRCommissioningStatusFailed = 2,
    MTRCommissioningStatusDiscoveringMoreDevices MTR_DEPRECATED("MTRCommissioningStatusDiscoveringMoreDevices is not used.",
        ios(16.1, 16.5), macos(13.0, 13.4), watchos(9.1, 9.5), tvos(16.1, 16.5))
    = 3,
} API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * A representation of a (vendor, product) pair that identifies a specific product.
 */
MTR_NEWLY_AVAILABLE
@interface MTRProductIdentity : NSObject

@property (nonatomic, copy, readonly) NSNumber * vendorID;

@property (nonatomic, copy, readonly) NSNumber * productID;

- (instancetype)initWithVendorID:(NSNumber *)vendorID productID:(NSNumber *)productID;
@end

@class MTRDeviceController;

/**
 * The protocol definition for the MTRDeviceControllerDelegate.
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@protocol MTRDeviceControllerDelegate <NSObject>
@optional
/**
 * Notify the delegate when commissioning status gets updated.
 */
- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status;

/**
 * Notify the delegate when a commissioning session is established or the
 * establishment has errored out.
 */
- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error;

/**
 * Notify the delegate when commissioning is completed.
 */
- (void)controller:(MTRDeviceController *)controller
    commissioningComplete:(NSError * _Nullable)error MTR_NEWLY_DEPRECATED("Please use controller:commissioningComplete:nodeID:");

/**
 * Notify the delegate when commissioning is completed.
 *
 * Exactly one of error and nodeID will be nil.
 *
 * If nodeID is not nil, then it represents the node id the node was assigned, as encoded in its operational certificate.
 */
- (void)controller:(MTRDeviceController *)controller
    commissioningComplete:(NSError * _Nullable)error
                   nodeID:(NSNumber * _Nullable)nodeID MTR_NEWLY_AVAILABLE;

/**
 * Notify the delegate when commissioning infomation has been read from the Basic
 * Information cluster of the commissionee.
 *
 * At the point when this notification happens, device attestation has not been performed yet,
 * so the information delivered by this notification should not be trusted.
 */
- (void)controller:(MTRDeviceController *)controller readCommissioningInfo:(MTRProductIdentity *)info MTR_NEWLY_AVAILABLE;
@end

typedef NS_ENUM(NSUInteger, MTRPairingStatus) {
    MTRPairingStatusUnknown MTR_DEPRECATED(
        "Please use MTRCommissioningStatusUnknown", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 0,
    MTRPairingStatusSuccess MTR_DEPRECATED(
        "Please use MTRCommissioningStatusSuccess", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 1,
    MTRPairingStatusFailed MTR_DEPRECATED(
        "Please use MTRCommissioningStatusFailed", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 2,
    MTRPairingStatusDiscoveringMoreDevices MTR_DEPRECATED("MTRPairingStatusDiscoveringMoreDevices is not used.", ios(16.1, 16.4),
        macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
    = 3
} MTR_DEPRECATED("Please use MTRCommissioningStatus", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

MTR_DEPRECATED("Please use MTRDeviceControllerDelegate", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4))
@protocol MTRDevicePairingDelegate <NSObject>
@optional
- (void)onStatusUpdate:(MTRPairingStatus)status;
- (void)onPairingComplete:(NSError * _Nullable)error;
- (void)onCommissioningComplete:(NSError * _Nullable)error;
- (void)onPairingDeleted:(NSError * _Nullable)error;

@end

NS_ASSUME_NONNULL_END
