/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

@class MTRSetupPayload;

NS_ASSUME_NONNULL_BEGIN

@interface MTRQRCodeSetupPayloadParser : NSObject
- (instancetype)initWithBase38Representation:(NSString *)base38Representation;
- (nullable MTRSetupPayload *)populatePayload:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
