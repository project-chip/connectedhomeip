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

#include "CHIPCommandStorageDelegate.h"

#import <Matter/Matter.h>

#import "PreferencesStorage.h"

NSString * const kDarwinFrameworkToolCertificatesDomain = @"com.apple.chiptool";

@interface CHIPToolPersistentStorageDelegate ()
@property (nonatomic, readonly) PreferencesStorage * storage;
@end

@implementation CHIPToolPersistentStorageDelegate

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    _storage = [[PreferencesStorage alloc] initWithDomain:kDarwinFrameworkToolCertificatesDomain];
    return self;
}

- (BOOL)deleteAllStorage
{
    return [_storage reset];
}

// MARK: CHIPPersistentStorageDelegate

- (nullable NSData *)storageDataForKey:(NSString *)key
{
    return _storage[key];
}

- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key
{
    _storage[key] = value;
    return YES;
}

- (BOOL)removeStorageDataForKey:(NSString *)key
{
    if (_storage[key] == nil) {
        return NO;
    }
    _storage[key] = nil;
    return YES;
}

@end
