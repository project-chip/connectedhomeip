/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRAsyncCallbackWorkQueue.h"

NS_ASSUME_NONNULL_BEGIN

@class MTRDevice;

@interface MTRAsyncCallbackWorkQueue ()
// The MTRDevice object is only held and passed back as a reference and is opaque to the queue
- (instancetype)initWithContext:(id _Nullable)context queue:(dispatch_queue_t)queue;

// Called by DeviceController at device clean up time
- (void)invalidate;
@end

NS_ASSUME_NONNULL_END
