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

typedef void (^SendNetworkCredentials)(NSString * ssid, NSString * password);

typedef NS_ENUM(NSUInteger, PairingStatus) {
    kSecurePairingSuccess = 0,
    kSecurePairingFailed,
    kNetworkProvisioningSuccess,
    kNetworkProvisioningFailed,
    kUnknownStatus,
};

/**
 * The protocol definition for the CHIPDevicePairingDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol CHIPDevicePairingDelegate <NSObject>
@required
/**
 * Notify the delegate when pairing requires network credentials
 *
 */
- (void)onNetworkCredentialsRequested:(SendNetworkCredentials)handler;

@optional
/**
 * Notify the delegate when pairing status gets updated
 *
 */
- (void)onStatusUpdate:(PairingStatus)status;

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
@end

NS_ASSUME_NONNULL_END
