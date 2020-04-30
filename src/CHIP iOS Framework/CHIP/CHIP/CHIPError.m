//
//  CHIPError.m
//  CHIPQRCodeReader
//
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

#import "CHIPError.h"

NSString *const CHIPErrorDomain = @"CHIPErrorDomain";


@implementation CHIPError

+ (NSError *)errorForCHIPErrorCode:(int32_t)errorCode
{
    switch (errorCode) {
        case 4030:
            return [NSError errorWithDomain:CHIPErrorDomain
                                       code:CHIPErrorCodeInvalidStringLength
                                   userInfo:@{
                                   NSLocalizedDescriptionKey: NSLocalizedString(@"A list length is invalid.", nil)}];
        case 4145:
            return [NSError errorWithDomain:CHIPErrorDomain
                                       code:CHIPErrorCodeInvalidIntegerValue
                                   userInfo:@{
                                       NSLocalizedDescriptionKey: NSLocalizedString(@"Unexpected integer value.", nil)}];
        case 4046:
                   return [NSError errorWithDomain:CHIPErrorDomain
                                              code:CHIPErrorCodeInvalidArgument
                                          userInfo:@{
                                              NSLocalizedDescriptionKey: NSLocalizedString(@"An argument is invalid.", nil)}];
        case 4023:
                   return [NSError errorWithDomain:CHIPErrorDomain
                                              code:CHIPErrorCodeInvalidMessageLength
                                          userInfo:@{
                                              NSLocalizedDescriptionKey: NSLocalizedString(@"A message length is invalid.", nil)}];
        default:
            return [NSError errorWithDomain:CHIPErrorDomain
                                       code:CHIPErrorCodeUndefinedError
                                   userInfo:@{
                                       NSLocalizedDescriptionKey: NSLocalizedString(@"Undefined error.", nil)}];;
    }
}
@end
