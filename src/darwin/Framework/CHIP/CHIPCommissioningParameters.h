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

/**
 * The class definition for the CHIPCommissioningParameters
 *
 */
@interface CHIPCommissioningParameters : NSObject

/**
 *  The CSRNonce
 */
@property (nonatomic, nullable, copy, readwrite) NSData * CSRNonce;
/**
 *  The AttestationNonce
 */
@property (nonatomic, nullable, copy, readwrite) NSData * attestationNonce;
/**
 *  The Wi-Fi SSID
 */
@property (nonatomic, nullable, copy, readwrite) NSData * wifiSSID;
/**
 *  The Wi-Fi Credentials
 */
@property (nonatomic, nullable, copy, readwrite) NSData * wifiCredentials;
/**
 *  The Thread operational dataset
 */
@property (nonatomic, nullable, copy, readwrite) NSData * threadOperationalDataset;

@end

NS_ASSUME_NONNULL_END
