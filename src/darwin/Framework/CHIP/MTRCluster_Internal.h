/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MTRBaseDevice.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRCluster.h"

#import "zap-generated/MTRBaseClusters.h"

#include <app/ReadPrepareParams.h>
#include <lib/core/DataModelTypes.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRCluster ()
@property (readonly, nonatomic) dispatch_queue_t callbackQueue;
@property (nonatomic, readonly) chip::EndpointId endpoint;

- (instancetype)initWithEndpointID:(NSNumber *)endpointID queue:(dispatch_queue_t)queue;
- (chip::ByteSpan)asByteSpan:(NSData *)value;
- (chip::CharSpan)asCharSpan:(NSString *)value;
@end

@interface MTRReadParams ()
/**
 * Copy state from this MTRReadParams to the ReadPreparaParams.
 */
- (void)toReadPrepareParams:(chip::app::ReadPrepareParams &)readPrepareParams;
@end

@interface MTRSubscribeParams ()
/**
 * Copy state from this MTRReadParams to the ReadPreparaParams.
 */
- (void)toReadPrepareParams:(chip::app::ReadPrepareParams &)readPrepareParams;
@end

NS_ASSUME_NONNULL_END
