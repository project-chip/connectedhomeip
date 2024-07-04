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
