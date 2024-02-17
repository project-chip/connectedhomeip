//
/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRDeviceControllerLocalTestStorage.h"

#if MTR_PER_CONTROLLER_STORAGE_ENABLED

static NSString * const kLocalTestUserDefaultDomain = @"org.csa-iot.matter.darwintest";
static NSString * const kLocalTestUserDefaultEnabledKey = @"enableTestStorage";

@implementation MTRDeviceControllerLocalTestStorage {
    id<MTRDeviceControllerStorageDelegate> _passThroughStorage;
}

+ (BOOL)localTestStorageEnabled
{
    NSUserDefaults * defaults = [[NSUserDefaults alloc] initWithSuiteName:kLocalTestUserDefaultDomain];
    return [defaults boolForKey:kLocalTestUserDefaultEnabledKey];
}

+ (void)setLocalTestStorageEnabled:(BOOL)localTestStorageEnabled
{
    NSUserDefaults * defaults = [[NSUserDefaults alloc] initWithSuiteName:kLocalTestUserDefaultDomain];
    [defaults setBool:localTestStorageEnabled forKey:kLocalTestUserDefaultEnabledKey];
}

- (instancetype)initWithPassThroughStorage:(id<MTRDeviceControllerStorageDelegate>)passThroughStorage
{
    if (self = [super init]) {
        _passThroughStorage = passThroughStorage;
    }
    return self;
}

- (nullable id<NSSecureCoding>)controller:(MTRDeviceController *)controller
                              valueForKey:(NSString *)key
                            securityLevel:(MTRStorageSecurityLevel)securityLevel
                              sharingType:(MTRStorageSharingType)sharingType
{
    if (sharingType == MTRStorageSharingTypeNotShared) {
        NSUserDefaults * defaults = [[NSUserDefaults alloc] initWithSuiteName:kLocalTestUserDefaultDomain];
        NSData * storedData = [defaults dataForKey:key];
        NSError * error;
        id value = [NSKeyedUnarchiver unarchivedObjectOfClasses:MTRDeviceControllerStorageClasses() fromData:storedData error:&error];
        return value;
    } else {
        return [_passThroughStorage controller:controller valueForKey:key securityLevel:securityLevel sharingType:sharingType];
    }
}

- (BOOL)controller:(MTRDeviceController *)controller
        storeValue:(id<NSSecureCoding>)value
            forKey:(NSString *)key
     securityLevel:(MTRStorageSecurityLevel)securityLevel
       sharingType:(MTRStorageSharingType)sharingType
{
    if (sharingType == MTRStorageSharingTypeNotShared) {
        NSError * error;
        NSData * data = [NSKeyedArchiver archivedDataWithRootObject:value requiringSecureCoding:YES error:&error];
        NSUserDefaults * defaults = [[NSUserDefaults alloc] initWithSuiteName:kLocalTestUserDefaultDomain];
        [defaults setObject:data forKey:key];
        return YES;
    } else {
        return [_passThroughStorage controller:controller storeValue:value forKey:key securityLevel:securityLevel sharingType:sharingType];
    }
}

- (BOOL)controller:(MTRDeviceController *)controller
    removeValueForKey:(NSString *)key
        securityLevel:(MTRStorageSecurityLevel)securityLevel
          sharingType:(MTRStorageSharingType)sharingType
{
    if (sharingType == MTRStorageSharingTypeNotShared) {
        NSUserDefaults * defaults = [[NSUserDefaults alloc] initWithSuiteName:kLocalTestUserDefaultDomain];
        [defaults removeObjectForKey:key];
        return YES;
    } else {
        return [_passThroughStorage controller:controller removeValueForKey:key securityLevel:securityLevel sharingType:sharingType];
    }
}
@end

#endif // MTR_PER_CONTROLLER_STORAGE_ENABLED
