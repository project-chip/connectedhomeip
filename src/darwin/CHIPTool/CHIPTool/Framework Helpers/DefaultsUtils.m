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
#import "FabricKeys.h"

NSString * const MTRToolDefaultsDomain = @"com.matter.CHIPTool";
NSString * const kNetworkSSIDDefaultsKey = @"networkSSID";
NSString * const kNetworkPasswordDefaultsKey = @"networkPassword";
NSString * const MTRNextAvailableDeviceIDKey = @"nextDeviceID";
NSString * const kFabricIdKey = @"fabricId";
NSString * const kDevicePairedKey = @"Paired";

id MTRGetDomainValueForKey(NSString * domain, NSString * key)
{
    id value = (id) CFBridgingRelease(CFPreferencesCopyAppValue((CFStringRef) key, (CFStringRef) domain));
    if (value) {
        return value;
    }
    return nil;
}

BOOL MTRSetDomainValueForKey(NSString * domain, NSString * key, id value)
{
    CFPreferencesSetAppValue((CFStringRef) key, (__bridge CFPropertyListRef _Nullable)(value), (CFStringRef) domain);
    return CFPreferencesAppSynchronize((CFStringRef) domain) == true;
}

void MTRRemoveDomainValueForKey(NSString * domain, NSString * key)
{
    CFPreferencesSetAppValue((CFStringRef) key, NULL, (CFStringRef) domain);
    CFPreferencesAppSynchronize((CFStringRef) domain);
}

uint64_t MTRGetNextAvailableDeviceID(void)
{
    uint64_t nextAvailableDeviceIdentifier = 1;
    NSNumber * value = MTRGetDomainValueForKey(MTRToolDefaultsDomain, MTRNextAvailableDeviceIDKey);
    if (!value) {
        MTRSetDomainValueForKey(MTRToolDefaultsDomain, MTRNextAvailableDeviceIDKey,
            [NSNumber numberWithUnsignedLongLong:nextAvailableDeviceIdentifier]);
    } else {
        nextAvailableDeviceIdentifier = [value unsignedLongLongValue];
    }

    return nextAvailableDeviceIdentifier;
}

void MTRSetNextAvailableDeviceID(uint64_t id)
{
    MTRSetDomainValueForKey(MTRToolDefaultsDomain, MTRNextAvailableDeviceIDKey, [NSNumber numberWithUnsignedLongLong:id]);
}

static CHIPToolPersistentStorageDelegate * storage = nil;

static uint16_t kTestVendorId = 0xFFF1u;

static MTRDeviceController * sController = nil;

MTRDeviceController * InitializeMTR(void)
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        CHIPToolPersistentStorageDelegate * storage = [[CHIPToolPersistentStorageDelegate alloc] init];
        __auto_type * factory = [MTRControllerFactory sharedInstance];
        __auto_type * factoryParams = [[MTRControllerFactoryParams alloc] initWithStorage:storage];
        if (![factory startup:factoryParams]) {
            return;
        }

        __auto_type * keys = [[FabricKeys alloc] init];
        if (keys == nil) {
            return;
        }

        __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithSigningKeypair:keys fabricId:1 ipk:keys.ipk];
        params.vendorId = @(kTestVendorId);

        // We're not sure whether we have a fabric configured already; try as if
        // we did, and if not fall back to creating a new one.
        sController = [factory startControllerOnExistingFabric:params];
        if (sController == nil) {
            sController = [factory startControllerOnNewFabric:params];
        }
    });

    return sController;
}

MTRDeviceController * MTRRestartController(MTRDeviceController * controller)
{
    __auto_type * keys = [[FabricKeys alloc] init];
    if (keys == nil) {
        NSLog(@"No keys, can't restart controller");
        return controller;
    }

    NSLog(@"Shutting down the stack");
    [controller shutdown];

    NSLog(@"Starting up the stack");
    __auto_type * params = [[MTRDeviceControllerStartupParams alloc] initWithSigningKeypair:keys fabricId:1 ipk:keys.ipk];

    sController = [[MTRControllerFactory sharedInstance] startControllerOnExistingFabric:params];

    return sController;
}

uint64_t MTRGetLastPairedDeviceId(void)
{
    uint64_t deviceId = MTRGetNextAvailableDeviceID();
    if (deviceId > 1) {
        deviceId--;
    }
    return deviceId;
}

BOOL MTRGetConnectedDevice(MTRDeviceConnectionCallback completionHandler)
{
    MTRDeviceController * controller = InitializeMTR();

    // Let's use the last device that was paired
    uint64_t deviceId = MTRGetLastPairedDeviceId();
    return [controller getBaseDevice:deviceId queue:dispatch_get_main_queue() completionHandler:completionHandler];
}

MTRBaseDevice * MTRGetDeviceBeingCommissioned(void)
{
    NSError * error;
    MTRDeviceController * controller = InitializeMTR();
    MTRBaseDevice * device = [controller getDeviceBeingCommissioned:MTRGetLastPairedDeviceId() error:&error];
    if (error) {
        NSLog(@"Error retrieving device being commissioned for deviceId %llu", MTRGetLastPairedDeviceId());
        return nil;
    }
    return device;
}

BOOL MTRGetConnectedDeviceWithID(uint64_t deviceId, MTRDeviceConnectionCallback completionHandler)
{
    MTRDeviceController * controller = InitializeMTR();

    return [controller getBaseDevice:deviceId queue:dispatch_get_main_queue() completionHandler:completionHandler];
}

BOOL MTRIsDevicePaired(uint64_t deviceId)
{
    NSString * PairedString = MTRGetDomainValueForKey(MTRToolDefaultsDomain, KeyForPairedDevice(deviceId));
    return [PairedString boolValue];
}

void MTRSetDevicePaired(uint64_t deviceId, BOOL paired)
{
    MTRSetDomainValueForKey(MTRToolDefaultsDomain, KeyForPairedDevice(deviceId), paired ? @"YES" : @"NO");
}

NSString * KeyForPairedDevice(uint64_t deviceId) { return [NSString stringWithFormat:@"%@%llu", kDevicePairedKey, deviceId]; }

void MTRUnpairDeviceWithID(uint64_t deviceId)
{
    MTRSetDevicePaired(deviceId, NO);
    MTRGetConnectedDeviceWithID(deviceId, ^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
        if (error) {
            NSLog(@"Failed to unpair device %llu still removing from CHIPTool. %@", deviceId, error);
            return;
        }
        NSLog(@"Attempting to unpair device %llu", deviceId);
        MTRBaseClusterOperationalCredentials * opCredsCluster =
            [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:device endpoint:0 queue:dispatch_get_main_queue()];
        [opCredsCluster
            readAttributeCurrentFabricIndexWithCompletionHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                if (error) {
                    NSLog(@"Failed to get current fabric index for device %llu still removing from CHIPTool. %@", deviceId, error);
                    return;
                }
                MTROperationalCredentialsClusterRemoveFabricParams * params =
                    [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
                params.fabricIndex = value;
                [opCredsCluster removeFabricWithParams:params
                                     completionHandler:^(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                         NSError * _Nullable error) {
                                         if (error) {
                                             NSLog(@"Failed to remove current fabric index %@ for device %llu. %@",
                                                 params.fabricIndex, deviceId, error);
                                             return;
                                         }
                                         NSLog(@"Successfully unpaired deviceId %llu", deviceId);
                                     }];
            }];
    });
}

@implementation CHIPToolPersistentStorageDelegate

// MARK: MTRPersistentStorageDelegate

- (nullable NSData *)storageDataForKey:(NSString *)key
{
    NSData * value = MTRGetDomainValueForKey(MTRToolDefaultsDomain, key);
    NSLog(@"MTRPersistentStorageDelegate Get Value for Key: %@, value %@", key, value);
    return value;
}

- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key
{
    return MTRSetDomainValueForKey(MTRToolDefaultsDomain, key, value);
}

- (BOOL)removeStorageDataForKey:(NSString *)key
{
    if (MTRGetDomainValueForKey(MTRToolDefaultsDomain, key) == nil) {
        return NO;
    }
    MTRRemoveDomainValueForKey(MTRToolDefaultsDomain, key);
    return YES;
}

@end
