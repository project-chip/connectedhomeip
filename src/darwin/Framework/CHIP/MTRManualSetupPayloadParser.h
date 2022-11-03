/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

@class MTRSetupPayload;

NS_ASSUME_NONNULL_BEGIN

@interface MTRManualSetupPayloadParser : NSObject
- (instancetype)initWithDecimalStringRepresentation:(NSString *)decimalStringRepresentation;
- (nullable MTRSetupPayload *)populatePayload:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
