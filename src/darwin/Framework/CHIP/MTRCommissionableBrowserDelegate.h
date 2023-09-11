/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

@class MTRCommissionableBrowserResult;
@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

API_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
@protocol MTRCommissionableBrowserDelegate <NSObject>
/**
 * Tells the delegate the commissionable manager discovered a device while scanning for devices.
 */
- (void)controller:(MTRDeviceController *)controller didFindCommissionableDevice:(MTRCommissionableBrowserResult *)device;

/**
 * Tells the delegate a previously discovered device is is no longer available.
 */
- (void)controller:(MTRDeviceController *)controller didRemoveCommissionableDevice:(MTRCommissionableBrowserResult *)device;
@end

NS_ASSUME_NONNULL_END
