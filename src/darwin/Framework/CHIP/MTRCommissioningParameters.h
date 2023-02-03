/**
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

@protocol MTRDeviceAttestationDelegate;

/**
 * Information that can be provided to commissionWithNodeID to commision devices.
 */
@interface MTRCommissioningParameters : NSObject

/**
 * The nonce to use when requesting a CSR for the node's operational
 * certificate.
 *
 * If nil, a random nonce will be generated automatically.
 *
 * If not nil, must be 32 bytes of data.
 */
@property (nonatomic, copy, nullable) NSData * csrNonce API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The nonce to use when requesting attestation information from the device.
 *
 * If nil, a random nonce will be generated automatically.
 *
 * If not nil, must be 32 bytes of data.
 */
@property (nonatomic, copy, nullable) NSData * attestationNonce;

/**
 * The Wi-Fi SSID, if available.
 */
@property (nonatomic, copy, nullable) NSData * wifiSSID;

/**
 * The Wi-Fi Credentials.  Allowed to be nil or 0-length data for an open
 * network, as long as wifiSSID is not nil.
 */
@property (nonatomic, copy, nullable) NSData * wifiCredentials;

/**
 *  The Thread operational dataset, if available.
 */
@property (nonatomic, copy, nullable) NSData * threadOperationalDataset;

/**
 * An optional delegate that can be notified upon completion of device
 * attestation.  See documentation for MTRDeviceAttestationDelegate for
 * details.
 *
 * The delegate methods will be invoked on an arbitrary thread.
 */
@property (nonatomic, strong, nullable) id<MTRDeviceAttestationDelegate> deviceAttestationDelegate;

/**
 * The timeout, in seconds, to set for the fail-safe when calling into the
 * deviceAttestationDelegate and waiting for it to respond.
 *
 * If nil, the fail-safe will not be extended before calling into the
 * deviceAttestationDelegate.
 */
@property (nonatomic, copy, nullable) NSNumber * failSafeTimeout API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

@interface MTRCommissioningParameters (Deprecated)

@property (nonatomic, copy, nullable) NSData * CSRNonce API_DEPRECATED(
    "Please use csrNonce", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy, nullable) NSNumber * failSafeExpiryTimeoutSecs API_DEPRECATED(
    "Please use failSafeTimeout", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

NS_ASSUME_NONNULL_END
