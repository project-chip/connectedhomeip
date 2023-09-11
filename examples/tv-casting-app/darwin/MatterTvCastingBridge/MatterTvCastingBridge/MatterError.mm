/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MatterError.h"

#include <lib/core/CHIPError.h>

@implementation MatterError

MatterError * MATTER_NO_ERROR = [[MatterError alloc] initWithCode:0 message:nil];

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
