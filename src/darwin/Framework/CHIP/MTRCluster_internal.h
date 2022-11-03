/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRBaseDevice.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCluster.h"

#import "zap-generated/CHIPClusters.h"
#import "zap-generated/MTRBaseClusters.h"

NS_ASSUME_NONNULL_BEGIN

@interface MTRCluster ()
@property (readonly, nonatomic) dispatch_queue_t callbackQueue;
- (nullable instancetype)initWithQueue:(dispatch_queue_t)queue;
- (chip::ByteSpan)asByteSpan:(NSData *)value;
- (chip::CharSpan)asCharSpan:(NSString *)value;
@end

NS_ASSUME_NONNULL_END
