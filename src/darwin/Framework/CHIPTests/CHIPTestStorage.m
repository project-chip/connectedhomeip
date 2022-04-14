/**
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "CHIPTestStorage.h"

@interface CHIPTestStorage ()
@property (strong, nonatomic) NSMutableDictionary<NSString *, NSString *> * values;
@end

@implementation CHIPTestStorage

- (NSString *)CHIPGetKeyValue:(NSString *)key
{
    return _values[key];
}
- (void)CHIPSetKeyValue:(NSString *)key value:(NSString *)value
{
    _values[key] = value;
}
- (void)CHIPDeleteKeyValue:(NSString *)key
{
    [_values removeObjectForKey:key];
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    _values = [[NSMutableDictionary alloc] init];
    if (_values == nil) {
        return nil;
    }

    return self;
}

@end
