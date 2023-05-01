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
