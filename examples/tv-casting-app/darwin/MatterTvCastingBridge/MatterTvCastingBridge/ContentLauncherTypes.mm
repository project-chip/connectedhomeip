/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "ContentLauncherTypes.h"

@implementation ContentLauncher_AdditionalInfo
- (ContentLauncher_AdditionalInfo * _Nonnull)initWithName:(NSString * _Nonnull)name value:(NSString * _Nonnull)value
{
    self = [super init];
    if (self) {
        _name = name;
        _value = value;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"ContentLauncher_AdditionalInfo: name=%@ value=%@", _name, _value];
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    return [[ContentLauncher_AdditionalInfo alloc] initWithName:_name value:_value];
}
@end

@implementation ContentLauncher_Parameter
- (ContentLauncher_Parameter * _Nonnull)initWithType:(ContentLauncher_ParameterEnum)type
                                               value:(NSString * _Nonnull)value
                                      externalIDList:(NSArray * _Nullable)externalIDList
{
    self = [super init];
    if (self) {
        _type = type;
        _value = value;
        _externalIDList = externalIDList;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"ContentLauncher_Parameter: type=%hhu value=%@", _type, _value];
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    return [[ContentLauncher_Parameter alloc] initWithType:_type value:_value externalIDList:_externalIDList];
}
@end

@implementation ContentLauncher_ContentSearch
- (ContentLauncher_ContentSearch * _Nonnull)initWithParameterList:(NSArray * _Nonnull)parameterList
{
    self = [super init];
    if (self) {
        _parameterList = parameterList;
    }
    return self;
}

- (id)copyWithZone:(NSZone * _Nullable)zone
{
    return [[ContentLauncher_ContentSearch alloc] initWithParameterList:_parameterList];
}
@end
