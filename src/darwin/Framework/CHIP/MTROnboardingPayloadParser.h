/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRSetupPayload;

typedef NS_ENUM(NSUInteger, MTROnboardingPayloadType) {
    MTROnboardingPayloadTypeQRCode = 0,
    MTROnboardingPayloadTypeManualCode,
    MTROnboardingPayloadTypeNFC
};

@interface MTROnboardingPayloadParser : NSObject

+ (nullable MTRSetupPayload *)setupPayloadForOnboardingPayload:(NSString *)onboardingPayload
                                                         error:(NSError * __autoreleasing *)error;

@end

NS_ASSUME_NONNULL_END
