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

typedef NS_ENUM(NSUInteger, MTRCommissioningStatus) {
    MTRCommissioningStatusUnknown = 0,
    MTRCommissioningStatusSuccess = 1,
    MTRCommissioningStatusFailed = 2,
    MTRCommissioningStatusDiscoveringMoreDevices = 3
};

@class MTRDeviceController;

/**
 * The protocol definition for the MTRDeviceControllerDelegate.
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol MTRDeviceControllerDelegate <NSObject>
@optional
/**
 * Notify the delegate when commissioning status gets updated.
 */
- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status;

/**
 * Notify the delegate when a commissioning session is established or the
 * establishment has errored out.
 */
- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error;

/**
 * Notify the delegate when commissioning is completed.
 */
- (void)controller:(MTRDeviceController *)controller commissioningComplete:(NSError * _Nullable)error;

@end

NS_ASSUME_NONNULL_END
