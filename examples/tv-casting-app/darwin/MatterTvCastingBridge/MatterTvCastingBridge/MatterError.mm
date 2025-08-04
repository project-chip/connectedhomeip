/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "MatterError.h"

#include <lib/core/CHIPError.h>

@implementation MatterError

MatterError * MATTER_NO_ERROR = [[MatterError alloc] initWithCode:0 message:@"No error"];

MatterError * MATTER_ERROR_INCORRECT_STATE =
    [[MatterError alloc] initWithCode:CHIP_ERROR_INCORRECT_STATE.AsInteger()
                              message:[NSString stringWithUTF8String:CHIP_ERROR_INCORRECT_STATE.AsString()]];
MatterError * MATTER_ERROR_INVALID_ARGUMENT =
    [[MatterError alloc] initWithCode:CHIP_ERROR_INVALID_ARGUMENT.AsInteger()
                              message:[NSString stringWithUTF8String:CHIP_ERROR_INVALID_ARGUMENT.AsString()]];

- (MatterError *)initWithCode:(uint32_t)code message:(NSString * _Nullable)message
{
    self = [super init];
    if (self) {
        _code = code;
        _message = message;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"MatterError: code=%d message=%@", _code, _message];
}

- (BOOL)isEqualToMatterError:(MatterError *)other
{
    return self.code == other.code;
}

- (BOOL)isEqual:(id)other
{
    if (other == nil) {
        return NO;
    }

    if (self == other) {
        return YES;
    }

    if (![other isKindOfClass:[MatterError class]]) {
        return NO;
    }

    return [self isEqualToMatterError:(MatterError *) other];
}

- (NSUInteger)hash
{
    const NSUInteger prime = 31;
    NSUInteger result = 1;

    result = prime * result + self.code;

    return result;
}

@end
