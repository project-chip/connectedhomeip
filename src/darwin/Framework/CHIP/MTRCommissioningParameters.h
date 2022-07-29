/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 * The class definition for the CHIPCommissioningParameters
 *
 */
@interface MTRCommissioningParameters : NSObject

/**
 *  The CSRNonce
 */
@property (nonatomic, copy, nullable) NSData * CSRNonce;
/**
 *  The AttestationNonce
 */
@property (nonatomic, copy, nullable) NSData * attestationNonce;
/**
 *  The Wi-Fi SSID, if available.
 */
@property (nonatomic, copy, nullable) NSData * wifiSSID;
/**
 *  The Wi-Fi Credentials.  Allowed to be nil or 0-length data for an open
 *  network, as long as wifiSSID is not nil.
 */
@property (nonatomic, copy, nullable) NSData * wifiCredentials;
/**
 *  The Thread operational dataset, if available.
 */
@property (nonatomic, copy, nullable) NSData * threadOperationalDataset;
/**
 *  The Device Attestation status delegate
 */
@property (nonatomic, strong, nullable) id<MTRDeviceAttestationDelegate> deviceAttestationDelegate;
/**
 *  The timeout in secs to set for fail-safe when attestation fails
 */
@property (nonatomic, copy, nullable) NSNumber * failSafeExpiryTimeoutSecs;

@end

NS_ASSUME_NONNULL_END
