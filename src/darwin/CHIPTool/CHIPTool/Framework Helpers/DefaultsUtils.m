/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "DefaultsUtils.h"

NSString * const kCHIPToolDefaultsDomain = @"com.apple.chiptool";
NSString * const kNetworkSSIDDefaultsKey = @"networkSSID";
NSString * const kNetworkPasswordDefaultsKey = @"networkPassword";
NSString * const kCHIPNextAvailableDeviceIDKey = @"nextDeviceID";
NSString * const kFabricIdKey = @"fabricId";

id CHIPGetDomainValueForKey(NSString * domain, NSString * key)
{
    id value = (id) CFBridgingRelease(CFPreferencesCopyAppValue((CFStringRef) key, (CFStringRef) domain));
    if (value) {
        return value;
    }
    return nil;
}

void CHIPSetDomainValueForKey(NSString * domain, NSString * key, id value)
{
    CFPreferencesSetAppValue((CFStringRef) key, (__bridge CFPropertyListRef _Nullable)(value), (CFStringRef) domain);
    CFPreferencesAppSynchronize((CFStringRef) domain);
}

void CHIPRemoveDomainValueForKey(NSString * domain, NSString * key)
{
    CFPreferencesSetAppValue((CFStringRef) key, NULL, (CFStringRef) domain);
    CFPreferencesAppSynchronize((CFStringRef) domain);
}

uint64_t CHIPGetNextAvailableDeviceID(void)
{
    uint64_t nextAvailableDeviceIdentifier = 1;
    NSNumber * value = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kCHIPNextAvailableDeviceIDKey);
    if (!value) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kCHIPNextAvailableDeviceIDKey,
            [NSNumber numberWithUnsignedLongLong:nextAvailableDeviceIdentifier]);
    } else {
        nextAvailableDeviceIdentifier = [value unsignedLongLongValue];
    }

    return nextAvailableDeviceIdentifier;
}

void CHIPSetNextAvailableDeviceID(uint64_t id)
{
    CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kCHIPNextAvailableDeviceIDKey, [NSNumber numberWithUnsignedLongLong:id]);
}

CHIPDeviceController * InitializeCHIP(void)
{
    static CHIPToolPersistentStorageDelegate * storage = nil;
    static dispatch_once_t onceToken;
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    dispatch_once(&onceToken, ^{
        storage = [[CHIPToolPersistentStorageDelegate alloc] init];
        [controller startup:storage];
    });

    return controller;
}

BOOL CHIPGetConnectedDevice(CHIPDeviceConnectionCallback completionHandler)
{
    CHIPDeviceController * controller = InitializeCHIP();

    uint64_t deviceId = CHIPGetNextAvailableDeviceID();
    if (deviceId > 1) {
        // Let's use the last device that was paired
        deviceId--;
        NSError * error;
        return [controller getConnectedDevice:deviceId
                            completionHandler:completionHandler
                                        queue:dispatch_get_main_queue()
                                        error:&error];
    }

    return NO;
}

BOOL CHIPGetConnectedDeviceWithID(uint64_t deviceId, CHIPDeviceConnectionCallback completionHandler)
{
    CHIPDeviceController * controller = InitializeCHIP();

    NSError * error;
    return [controller getConnectedDevice:deviceId
                        completionHandler:completionHandler
                                    queue:dispatch_get_main_queue()
                                    error:&error];
}

BOOL CHIPIsDevicePaired(uint64_t deviceId)
{
    CHIPDeviceController * controller = InitializeCHIP();

    NSError * error;
    bool paired = [controller isDevicePaired:deviceId error:&error];
    if (error.code != CHIPSuccess) {
        NSLog(@"Error retrieving device info for deviceId %llu", deviceId);
        paired = NO;
    }
    return paired;
}

void CHIPUnpairDeviceWithID(uint64_t deviceId)
{
    CHIPDeviceController * controller = InitializeCHIP();

    NSError * error;
    [controller unpairDevice:deviceId error:&error];
}

@implementation CHIPToolPersistentStorageDelegate

// MARK: CHIPPersistentStorageDelegate

- (NSString *)CHIPGetKeyValue:(NSString *)key
{
    NSString * value = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, key);
    NSLog(@"CHIPPersistentStorageDelegate Get Value for Key: %@, value %@", key, value);
    return value;
}

- (void)CHIPSetKeyValue:(NSString *)key value:(NSString *)value
{
    CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, key, value);
}

- (void)CHIPDeleteKeyValue:(NSString *)key
{
    CHIPRemoveDomainValueForKey(kCHIPToolDefaultsDomain, key);
}

@end
