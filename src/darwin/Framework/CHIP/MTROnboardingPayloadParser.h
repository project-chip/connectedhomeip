/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRSetupPayload;

typedef NS_ENUM(NSUInteger, MTROnboardingPayloadType) {
    MTROnboardingPayloadTypeQRCode = 0,
    MTROnboardingPayloadTypeManualCode,
    MTROnboardingPayloadTypeNFC
} MTR_DEPRECATED("MTROnboardingPayloadType is unused", ios(16.1, 17.0), macos(13.0, 14.0), watchos(9.1, 10.0), tvos(16.1, 17.0));

MTR_DEPRECATED("Please use [MTRSetupPayload setupPayloadWithOnboardingPayload:error:]", ios(16.1, 17.0), macos(13.0, 14.0),
    watchos(9.1, 10.0), tvos(16.1, 17.0))
@interface MTROnboardingPayloadParser : NSObject

+ (MTRSetupPayload * _Nullable)setupPayloadForOnboardingPayload:(NSString *)onboardingPayload
                                                          error:(NSError * __autoreleasing *)error;

@end

NS_ASSUME_NONNULL_END
