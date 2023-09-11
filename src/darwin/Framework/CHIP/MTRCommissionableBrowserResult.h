/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
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
