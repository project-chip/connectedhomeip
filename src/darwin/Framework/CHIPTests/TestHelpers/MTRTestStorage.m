/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
