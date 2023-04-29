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

@class MTRSetupPayload;

typedef NS_ENUM(NSUInteger, MTROnboardingPayloadType) {
    MTROnboardingPayloadTypeQRCode = 0,
    MTROnboardingPayloadTypeManualCode,
    MTROnboardingPayloadTypeNFC
} MTR_NEWLY_DEPRECATED("MTROnboardingPayloadType is unused");

MTR_NEWLY_DEPRECATED("Please use [MTRSetupPayload setupPayloadWithOnboardingPayload:error:]")
@interface MTROnboardingPayloadParser : NSObject

+ (MTRSetupPayload * _Nullable)setupPayloadForOnboardingPayload:(NSString *)onboardingPayload
                                                          error:(NSError * __autoreleasing *)error;

@end

NS_ASSUME_NONNULL_END
