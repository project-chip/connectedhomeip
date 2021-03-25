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
    CHIPSuccess = 0,
    CHIPErrorCodeUndefinedError = 1,
    CHIPErrorCodeInvalidStringLength = 2,
    CHIPErrorCodeInvalidIntegerValue = 3,
    CHIPErrorCodeInvalidArgument = 4,
    CHIPErrorCodeInvalidMessageLength = 5,
    CHIPErrorCodeInvalidState = 6,
    CHIPErrorCodeWrongAddressType = 7,
    CHIPErrorCodeIntegrityCheckFailed = 8,
    CHIPErrorCodeDuplicateExists = 9,
};

@interface CHIPError : NSObject
+ (nullable NSError *)errorForCHIPErrorCode:(CHIP_ERROR)errorCode;
+ (CHIP_ERROR)errorToCHIPErrorCode:(NSError *)errorCode;
@end

NS_ASSUME_NONNULL_END
