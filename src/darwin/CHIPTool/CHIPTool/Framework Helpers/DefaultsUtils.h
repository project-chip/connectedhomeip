/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString * const MTRToolDefaultsDomain;
extern NSString * const kNetworkSSIDDefaultsKey;
extern NSString * const kNetworkPasswordDefaultsKey;
extern NSString * const kFabricIdKey;

MTRDeviceController * _Nullable InitializeMTR(void);
MTRDeviceController * _Nullable MTRRestartController(MTRDeviceController * controller);
id _Nullable MTRGetDomainValueForKey(NSString * domain, NSString * key);
BOOL MTRSetDomainValueForKey(NSString * domain, NSString * key, id _Nullable value);
void MTRRemoveDomainValueForKey(NSString * domain, NSString * key);
uint64_t MTRGetNextAvailableDeviceID(void);
NSString * KeyForPairedDevice(uint64_t id);
uint64_t MTRGetLastPairedDeviceId(void);
void MTRSetNextAvailableDeviceID(uint64_t id);
void MTRSetDevicePaired(uint64_t id, BOOL paired);
BOOL MTRIsDevicePaired(uint64_t id);
BOOL MTRGetConnectedDevice(MTRDeviceConnectionCallback completionHandler);
BOOL MTRGetConnectedDeviceWithID(uint64_t deviceId, MTRDeviceConnectionCallback completionHandler);
void MTRUnpairDeviceWithID(uint64_t deviceId);
MTRBaseDevice * _Nullable MTRGetDeviceBeingCommissioned(void);

@interface CHIPToolPersistentStorageDelegate : NSObject <MTRPersistentStorageDelegate>
- (nullable NSData *)storageDataForKey:(NSString *)key;
- (BOOL)setStorageData:(NSData *)value forKey:(NSString *)key;
- (BOOL)removeStorageDataForKey:(NSString *)key;
@end

NS_ASSUME_NONNULL_END
