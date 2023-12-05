/**
 *
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

#import <Foundation/Foundation.h>

#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@interface MTRCommissionableBrowserResult : NSObject

/**
 * For a node advertising over DNS-SD, the instance name is a dynamic, pseudo-randomly selected, 64-bit temporary unique identifier,
 * expressed as a fixed-length sixteen-character hexadecimal string, encoded as ASCII text using capital letters.
 *
 * For a node advertising over Bluetooth Low Energy, the instance name is always "BLE".
 */
@property (readonly, nonatomic) NSString * instanceName;

/**
 * A 16-bit unsigned value identifying the device manufacturer.
 */
@property (readonly, nonatomic) NSNumber * vendorID;

/**
 * A 16-bit unsigned value identifying the product.
 */
@property (readonly, nonatomic) NSNumber * productID;

/**
 * A 12-bit value matching the field of the same name in MTRSetupPayload.
 */
@property (readonly, nonatomic) NSNumber * discriminator;

/**
 * A boolean indicating whether the device has a commissioning window open.
 */
@property (readonly, nonatomic) BOOL commissioningMode;

@end

NS_ASSUME_NONNULL_END
