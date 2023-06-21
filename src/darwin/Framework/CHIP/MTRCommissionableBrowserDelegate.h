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

@class MTRCommissionableBrowserResult;

NS_ASSUME_NONNULL_BEGIN

MTR_NEWLY_AVAILABLE
@protocol MTRCommissionableBrowserDelegate <NSObject>
/**
 * Tells the delegate the commissionable manager discovered a device while scanning for devices.
 */
- (void)didDiscoverCommissionable:(MTRCommissionableBrowserResult *)device;

/**
 * Tells the delegate a previously discovered device is is no longer available.
 */
- (void)commissionableUnavailable:(MTRCommissionableBrowserResult *)device;
@end

NS_ASSUME_NONNULL_END
