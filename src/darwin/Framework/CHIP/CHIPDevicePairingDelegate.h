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

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, CHIPPairingStatus) {
    kSecurePairingSuccess = 0,
    kSecurePairingFailed,
    kNetworkProvisioningSuccess,
    kNetworkProvisioningFailed,
    kUnknownStatus,
};

typedef NS_ENUM(NSUInteger, CHIPNetworkCredentialType) {
    kNetworkCredentialTypeWiFi = 0,
    kNetworkCredentialTypeThread,
    kNetworkCredentialTypeAll,
};

/**
 * The protocol definition for the CHIPDevicePairingDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol CHIPDevicePairingDelegate <NSObject>
@required
/**
 * Notify the delegate when pairing requires network credentials along with the NetworkCredentialType requested
 *
 */
- (void)onNetworkCredentialsRequested:(CHIPNetworkCredentialType)type;

@optional
/**
 * Notify the delegate when pairing status gets updated
 *
 */
- (void)onStatusUpdate:(CHIPPairingStatus)status;

/**
 * Notify the delegate when pairing is completed
 *
 */
- (void)onPairingComplete:(NSError *)error;

/**
 * Notify the delegate when pairing is deleted
 *
 */
- (void)onPairingDeleted:(NSError *)error;

/**
 * Notify the delegate when address is updated
 *
 */
- (void)onAddressUpdated:(NSError *)error;

@end

NS_ASSUME_NONNULL_END
