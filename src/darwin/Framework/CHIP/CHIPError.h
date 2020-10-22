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
FOUNDATION_EXPORT NSErrorDomain const CHIPErrorDomain;

typedef int32_t CHIP_ERROR;

typedef NS_ERROR_ENUM(CHIPErrorDomain, CHIPErrorCode) {
    CHIPErrorCodeUndefinedError = 0,
    CHIPErrorCodeInvalidStringLength = 1,
    CHIPErrorCodeInvalidIntegerValue = 2,
    CHIPErrorCodeInvalidArgument = 3,
    CHIPErrorCodeInvalidMessageLength = 4,
    CHIPErrorCodeInvalidState = 5,
    CHIPErrorCodeWrongAddressType = 6,
    CHIPErrorCodeIntegrityCheckFailed = 7,
    CHIPSuccess = 8,
};

@interface CHIPError : NSObject
+ (nullable NSError *)errorForCHIPErrorCode:(CHIP_ERROR)errorCode;
+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError *)errorCode;
@end

NS_ASSUME_NONNULL_END
