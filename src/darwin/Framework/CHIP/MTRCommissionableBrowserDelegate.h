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

@class MTRCommissionableBrowserResult;
@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0))
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
