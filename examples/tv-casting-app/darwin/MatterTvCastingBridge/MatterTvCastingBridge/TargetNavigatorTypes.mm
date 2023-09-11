/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "TargetNavigatorTypes.h"

@implementation TargetNavigator_TargetInfoStruct

- (TargetNavigator_TargetInfoStruct * _Nonnull)initWithIdentifier:(NSNumber * _Nonnull)identifier name:(NSString * _Nonnull)name
{
    self = [super init];
    if (self) {
        _identifier = identifier;
        _name = name;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"TargetNavigator_TargetInfoStruct: identifier=%@ name=%@", _identifier, _name];
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    return [[TargetNavigator_TargetInfoStruct alloc] initWithIdentifier:_identifier name:_name];
}
@end
