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

#import "MTRTestStorage.h"

@interface MTRTestStorage ()
@property (strong, nonatomic) NSMutableDictionary<NSString *, NSData *> * values;
@end

@implementation MTRTestStorage

- (nullable NSData *)storageDataForKey:(NSString *)key
{
    return _values[key];
}
- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key
{
    _values[key] = value;
    return YES;
}
- (BOOL)removeStorageDataForKey:(NSString *)key
{
    BOOL present = (_values[key] != nil);
    [_values removeObjectForKey:key];
    return present;
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

- (NSString *)dumpStorageToString
{
    return [NSString stringWithFormat:@"%@", _values];
}

@end
