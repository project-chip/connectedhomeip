/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#import "PreferencesStorage.h"

@implementation PreferencesStorage
- (instancetype)initWithDomain:(NSString *)domain
{
    self = [super init];
    if (self) {
        _domain = domain;
    }

    return self;
}

- (NSData *)objectForKeyedSubscript:(NSString *)key
{
    __auto_type domainRef = (__bridge CFStringRef) self.domain;
    __auto_type keyRef = (__bridge CFStringRef) key;
    __auto_type value = CFPreferencesCopyAppValue(keyRef, domainRef);
    if (value) {
        id obj = (__bridge_transfer id) value;
        return obj;
    }
    return nil;
}

- (void)setObject:(id)obj forKeyedSubscript:(NSString *)key
{
    __auto_type domainRef = (__bridge CFStringRef) self.domain;
    __auto_type keyRef = (__bridge CFStringRef) key;
    __auto_type value = (__bridge CFPropertyListRef) obj;

    CFPreferencesSetAppValue(keyRef, value, domainRef);
    CFPreferencesAppSynchronize(domainRef);
}

- (NSArray<NSString *> *)allKeys
{
    __auto_type domainRef = (__bridge CFStringRef) self.domain;
    __auto_type keys = CFPreferencesCopyKeyList(domainRef, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);

    if (!keys) {
        return @[];
    }

    return (__bridge_transfer NSArray *) keys;
}

- (bool)reset
{
    __auto_type * keys = [self allKeys];
    __auto_type domainRef = (__bridge CFStringRef) self.domain;

    for (NSString * key in keys) {
        __auto_type keyRef = (__bridge CFStringRef) key;
        CFPreferencesSetAppValue(keyRef, NULL, domainRef);
    }

    return CFPreferencesAppSynchronize(domainRef);
}

- (void)print
{
    NSLog(@"%@:", self.domain);
    NSArray<NSString *> * keys = [self allKeys];
    for (NSString * key in keys) {
        __auto_type * data = [self objectForKeyedSubscript:key];
        NSLog(@" * %@: %@", key, data);
    }
}

#pragma mark - NSFastEnumeration

- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state objects:(id _Nullable __unsafe_unretained[])buffer count:(NSUInteger)len
{
    __auto_type * keys = [self allKeys];
    if (state->state >= keys.count) {
        return 0;
    }

    state->itemsPtr = buffer;
    state->mutationsPtr = &state->extra[0];

    NSUInteger count = 0;
    while (state->state < keys.count && count < len) {
        buffer[count] = keys[state->state];
        state->state++;
        count++;
    }
    return count;
}

@end
