/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/MTRDefines.h>

@class MTRSetupPayload;

NS_ASSUME_NONNULL_BEGIN

MTR_DEPRECATED("Please use [MTRSetupPayload setupPayloadWithOnboardingPayload:error:]", ios(16.1, 16.4), macos(13.0, 13.3),
    watchos(9.1, 9.4), tvos(16.1, 16.4))
@interface MTRManualSetupPayloadParser : NSObject
- (instancetype)initWithDecimalStringRepresentation:(NSString *)decimalStringRepresentation;
- (MTRSetupPayload * _Nullable)populatePayload:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
