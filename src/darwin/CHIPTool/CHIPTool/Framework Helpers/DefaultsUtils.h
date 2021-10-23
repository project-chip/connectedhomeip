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

#import <CHIP/CHIP.h>
#import <Foundation/Foundation.h>

#define kCHIPToolTmpVendorId 0x1771

extern NSString * const kCHIPToolDefaultsDomain;
extern NSString * const kNetworkSSIDDefaultsKey;
extern NSString * const kNetworkPasswordDefaultsKey;
extern NSString * const kFabricIdKey;

CHIPDeviceController * InitializeCHIP(void);
id CHIPGetDomainValueForKey(NSString * domain, NSString * key);
void CHIPSetDomainValueForKey(NSString * domain, NSString * key, id value);
void CHIPRemoveDomainValueForKey(NSString * domain, NSString * key);
uint64_t CHIPGetNextAvailableDeviceID(void);
void CHIPSetNextAvailableDeviceID(uint64_t id);
BOOL CHIPIsDevicePaired(uint64_t id);
BOOL CHIPGetConnectedDevice(CHIPDeviceConnectionCallback completionHandler);
BOOL CHIPGetConnectedDeviceWithID(uint64_t deviceId, CHIPDeviceConnectionCallback completionHandler);
void CHIPUnpairDeviceWithID(uint64_t deviceId);

@interface CHIPToolPersistentStorageDelegate : NSObject <CHIPPersistentStorageDelegate>

@end
