/**
 *    Copyright (c) 2024 Project CHIP Authors
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
#import <Matter/MTRError.h>

#import "MTRDefines_Internal.h"

NS_ASSUME_NONNULL_BEGIN

MTR_TESTABLE
@interface MTRError : NSObject

+ (NSError *)errorWithCode:(MTRErrorCode)code;

// For tests only, since we can't use CHIP_ERROR from there.  The "code"s used
// here are integer representations of CHIP_ERROR.  Otherwise these functions
// are just like errorForCHIPErrorCode and errorToCHIPErrorCode.
+ (NSError *)errorForCHIPIntegerCode:(uint32_t)code;
+ (uint32_t)errorToCHIPIntegerCode:(NSError * _Nullable)error;

@end

NS_ASSUME_NONNULL_END
