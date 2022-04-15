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
NSString * const kDevicePairedKey = @"Paired";

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

static CHIPToolPersistentStorageDelegate * storage = nil;

static uint16_t kTestVendorId = 0xFFF1u;

static CHIPDeviceController * sController = nil;

CHIPDeviceController * InitializeCHIP(void)
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        CHIPToolPersistentStorageDelegate * storage = [[CHIPToolPersistentStorageDelegate alloc] init];
        __auto_type * factory = [MatterControllerFactory sharedInstance];
        __auto_type * factoryParams = [[MatterControllerFactoryParams alloc] initWithStorage:storage];
        if (![factory startup:factoryParams]) {
            return;
        }

        __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:nil];
        params.vendorId = kTestVendorId;
        params.fabricId = 1;

        // We're not sure whether we have a fabric configured already; try as if
        // we did, and if not fall back to creating a new one.
        sController = [factory startControllerOnExistingFabric:params];
        if (sController == nil) {
            sController = [factory startControllerOnNewFabric:params];
        }
    });

    return sController;
}

CHIPDeviceController * CHIPRestartController(CHIPDeviceController * controller)
{
    NSLog(@"Shutting down the stack");
    [controller shutdown];

    NSLog(@"Starting up the stack");
    __auto_type * params = [[CHIPDeviceControllerStartupParams alloc] initWithKeypair:nil];
    params.vendorId = kTestVendorId;
    params.fabricId = 1;

    sController = [[MatterControllerFactory sharedInstance] startControllerOnExistingFabric:params];

    return sController;
}

uint64_t CHIPGetLastPairedDeviceId(void)
{
    uint64_t deviceId = CHIPGetNextAvailableDeviceID();
    if (deviceId > 1) {
        deviceId--;
    }
    return deviceId;
}

BOOL CHIPGetConnectedDevice(CHIPDeviceConnectionCallback completionHandler)
{
    CHIPDeviceController * controller = InitializeCHIP();

    // Let's use the last device that was paired
    uint64_t deviceId = CHIPGetLastPairedDeviceId();
    return [controller getConnectedDevice:deviceId queue:dispatch_get_main_queue() completionHandler:completionHandler];
}

CHIPDevice * CHIPGetDeviceBeingCommissioned(void)
{
    NSError * error;
    CHIPDeviceController * controller = InitializeCHIP();
    CHIPDevice * device = [controller getDeviceBeingCommissioned:CHIPGetLastPairedDeviceId() error:&error];
    if (error) {
        NSLog(@"Error retrieving device being commissioned for deviceId %llu", CHIPGetLastPairedDeviceId());
        return nil;
    }
    return device;
}

BOOL CHIPGetConnectedDeviceWithID(uint64_t deviceId, CHIPDeviceConnectionCallback completionHandler)
{
    CHIPDeviceController * controller = InitializeCHIP();

    return [controller getConnectedDevice:deviceId queue:dispatch_get_main_queue() completionHandler:completionHandler];
}

BOOL CHIPIsDevicePaired(uint64_t deviceId)
{
    NSString * PairedString = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, KeyForPairedDevice(deviceId));
    return [PairedString boolValue];
}

void CHIPSetDevicePaired(uint64_t deviceId, BOOL paired)
{
    CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, KeyForPairedDevice(deviceId), paired ? @"YES" : @"NO");
}

NSString * KeyForPairedDevice(uint64_t deviceId) { return [NSString stringWithFormat:@"%@%llu", kDevicePairedKey, deviceId]; }

void CHIPUnpairDeviceWithID(uint64_t deviceId)
{
    CHIPSetDevicePaired(deviceId, NO);
    CHIPGetConnectedDeviceWithID(deviceId, ^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
        if (error) {
            NSLog(@"Failed to unpair device %llu still removing from CHIPTool. %@", deviceId, error);
            return;
        }
        NSLog(@"Attempting to unpair device %llu", deviceId);
        CHIPOperationalCredentials * opCredsCluster = [[CHIPOperationalCredentials alloc] initWithDevice:device
                                                                                                endpoint:0
                                                                                                   queue:dispatch_get_main_queue()];
        [opCredsCluster
            readAttributeCurrentFabricIndexWithCompletionHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                if (error) {
                    NSLog(@"Failed to get current fabric index for device %llu still removing from CHIPTool. %@", deviceId, error);
                    return;
                }
                CHIPOperationalCredentialsClusterRemoveFabricParams * params =
                    [[CHIPOperationalCredentialsClusterRemoveFabricParams alloc] init];
                params.fabricIndex = value;
                [opCredsCluster removeFabricWithParams:params
                                     completionHandler:^(CHIPOperationalCredentialsClusterNOCResponseParams * _Nullable data,
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
