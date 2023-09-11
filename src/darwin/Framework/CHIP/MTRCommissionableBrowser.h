/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MTRCommissionableBrowserDelegate;
@class MTRDeviceController;

MTR_HIDDEN
@interface MTRCommissionableBrowser : NSObject
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)initWithDelegate:(id<MTRCommissionableBrowserDelegate>)delegate
                      controller:(MTRDeviceController *)controller
                           queue:(dispatch_queue_t)queue;
/**
 * Start browsing the available networks (e.g IP, BLE) for commissionable nodes.
 *
 * If a browse is already ongoing this will not start a new browse and the return value will be NO.
 */
- (BOOL)start;

/**
 * Stop browsing the network for commissionable nodes.
 */
- (BOOL)stop;
@end

NS_ASSUME_NONNULL_END
