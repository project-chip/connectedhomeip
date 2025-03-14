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

#import "ControllerStorage.h"
#import "PreferencesStorage.h"

#ifdef LOG_DEBUG_CONTROLLER_STORAGE
#include <lib/support/logging/CHIPLogging.h>
#endif // LOG_DEBUG_CONTROLLER_STORAGE

NSString * const kDarwinFrameworkToolControllerDomain = @"com.apple.darwin-framework-tool.controller";

@interface ControllerStorage ()
@property (nonatomic, readonly) PreferencesStorage * storage;
@property (nonatomic, readonly) NSString * keyScopingPrefix;

- (NSString *)_keyToControllerScopedKey:(NSString *)key;
- (NSString *)_controllerScopedKeyToKey:(NSString *)controllerKey;
- (BOOL)_isControllerScopedKey:(NSString *)controllerKey;
@end

@implementation ControllerStorage
- (instancetype)initWithControllerID:(NSUUID *)controllerID
{
    if (!(self = [super init])) {
        return nil;
    }

    _storage = [[PreferencesStorage alloc] initWithDomain:kDarwinFrameworkToolControllerDomain];
    _controllerID = controllerID;
    _keyScopingPrefix = [NSString stringWithFormat:@"%@/", [_controllerID UUIDString]];
    return self;
}

- (nullable id<NSSecureCoding>)controller:(MTRDeviceController *)controller
                              valueForKey:(NSString *)key
                            securityLevel:(MTRStorageSecurityLevel)securityLevel
                              sharingType:(MTRStorageSharingType)sharingType
{
#ifdef LOG_DEBUG_CONTROLLER_STORAGE
    ChipLogError(chipTool, "Controller(%@) Storage - Get value for %@", controller, key);
#endif // LOG_DEBUG_CONTROLLER_STORAGE

    __auto_type * controllerKey = [self _keyToControllerScopedKey:key];
    __auto_type * data = self.storage[controllerKey];
    if (data == nil) {
        return data;
    }

    NSError * error;
    id value = [NSKeyedUnarchiver unarchivedObjectOfClasses:MTRDeviceControllerStorageClasses() fromData:data error:&error];
    return value;
}

- (BOOL)controller:(MTRDeviceController *)controller
        storeValue:(id<NSSecureCoding>)value
            forKey:(NSString *)key
     securityLevel:(MTRStorageSecurityLevel)securityLevel
       sharingType:(MTRStorageSharingType)sharingType
{
#ifdef LOG_DEBUG_CONTROLLER_STORAGE
    ChipLogError(chipTool, "Controller(%@) Storage - Set value for %@", controller, key);
#endif // LOG_DEBUG_CONTROLLER_STORAGE
    NSError * error;
    NSData * data = [NSKeyedArchiver archivedDataWithRootObject:value requiringSecureCoding:YES error:&error];

    __auto_type * controllerKey = [self _keyToControllerScopedKey:key];
    self.storage[controllerKey] = data;
    return YES;
}

- (BOOL)controller:(MTRDeviceController *)controller
    removeValueForKey:(NSString *)key
        securityLevel:(MTRStorageSecurityLevel)securityLevel
          sharingType:(MTRStorageSharingType)sharingType
{
#ifdef LOG_DEBUG_CONTROLLER_STORAGE
    ChipLogError(chipTool, "Controller(%@) Storage - Remove value for %@", controller, key);
#endif // LOG_DEBUG_CONTROLLER_STORAGE

    __auto_type * controllerKey = [self _keyToControllerScopedKey:key];
    self.storage[controllerKey] = nil;
    return YES;
}

- (NSDictionary<NSString *, id<NSSecureCoding>> *)valuesForController:(MTRDeviceController *)controller securityLevel:(MTRStorageSecurityLevel)securityLevel sharingType:(MTRStorageSharingType)sharingType
{
#ifdef LOG_DEBUG_CONTROLLER_STORAGE
    ChipLogError(chipTool, "Controller(%@) Storage - Get all values", controller);
#endif // LOG_DEBUG_CONTROLLER_STORAGE

    NSMutableDictionary * valuesToReturn = [NSMutableDictionary dictionary];
    for (NSString * controllerKey in self.storage) {
        if (![self _isControllerScopedKey:controllerKey]) {
            continue;
        }
        __auto_type * key = [self _controllerScopedKeyToKey:controllerKey];
        valuesToReturn[key] = [self controller:controller valueForKey:key securityLevel:securityLevel sharingType:sharingType];
    }

    if (!valuesToReturn.count) {
        return nil;
    }

    return valuesToReturn;
}

- (BOOL)controller:(MTRDeviceController *)controller storeValues:(NSDictionary<NSString *, id<NSSecureCoding>> *)values securityLevel:(MTRStorageSecurityLevel)securityLevel sharingType:(MTRStorageSharingType)sharingType
{
#ifdef LOG_DEBUG_CONTROLLER_STORAGE
    ChipLogError(chipTool, "Controller(%@) Storage - store values", controller);
#endif // LOG_DEBUG_CONTROLLER_STORAGE

    for (NSString * key in values) {
        [self controller:controller storeValue:values[key] forKey:key securityLevel:securityLevel sharingType:sharingType];
    }

    return YES;
}

- (NSData *)valueForKey:(NSString *)key
{
    __auto_type * controllerKey = [self _keyToControllerScopedKey:key];
    return self.storage[controllerKey];
}

- (void)storeValue:(NSData *)value forKey:key
{
    __auto_type * controllerKey = [self _keyToControllerScopedKey:key];
    self.storage[controllerKey] = value;
}

- (void)print
{
    NSLog(@"%@ (%@)", kDarwinFrameworkToolControllerDomain, _keyScopingPrefix);
    for (NSString * controllerKey in self.storage) {
        if (![self _isControllerScopedKey:controllerKey]) {
            continue;
        }

        __auto_type * key = [self _controllerScopedKeyToKey:controllerKey];
        __auto_type * data = self.storage[controllerKey];
        NSLog(@" * %@: %@", key, data);
    }
}

- (NSString *)_keyToControllerScopedKey:(NSString *)key
{
    return [NSString stringWithFormat:@"%@%@", _keyScopingPrefix, key];
}

- (NSString *)_controllerScopedKeyToKey:(NSString *)controllerKey
{
    return [controllerKey substringFromIndex:_keyScopingPrefix.length];
}

- (BOOL)_isControllerScopedKey:(NSString *)controllerKey
{
    return [controllerKey hasPrefix:_keyScopingPrefix];
}
@end
