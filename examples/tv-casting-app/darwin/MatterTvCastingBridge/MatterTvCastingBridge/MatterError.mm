/*
 * SPDX-FileCopyrightText: (c) 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "MatterError.h"

@implementation MatterError

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

@end
