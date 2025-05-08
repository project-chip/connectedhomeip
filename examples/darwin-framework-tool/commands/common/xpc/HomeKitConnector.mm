/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#import "HomeKitConnector.h"
#import "../CHIPCommandBridge.h"
#import <lib/support/logging/CHIPLogging.h>

#import <HomeKit/HomeKit.h>

const int64_t kHomeManagerSetupTimeout = 10LL * NSEC_PER_SEC;
NSString * kControllerIdPrefixStr = @(kControllerIdPrefix);

@interface HomeKitConnector () <HMHomeManagerDelegate>
@property (nonatomic, assign) BOOL connectorStarted;
@property (nonatomic, strong) HMHomeManager * homeManager;
@property (nonatomic, assign) BOOL homeManagerReady;
@end

@implementation HomeKitConnector {
    dispatch_group_t _homeManagerReadyGroup;
}

+ (instancetype)sharedInstance
{
    static HomeKitConnector * sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[HomeKitConnector alloc] init];
    });
    return sharedInstance;
}

- (void)start
{
    VerifyOrReturn(!_connectorStarted);
    _connectorStarted = YES;

    _homeManagerReady = NO;
    _homeManagerReadyGroup = dispatch_group_create();
    dispatch_group_enter(_homeManagerReadyGroup);

    _homeManager = [[HMHomeManager alloc] init];
    _homeManager.delegate = self;

    // Wait until homeManagerDidUpdateHomes is called or timeout
    dispatch_group_wait(_homeManagerReadyGroup, dispatch_time(DISPATCH_TIME_NOW, kHomeManagerSetupTimeout));

    [self printHomes];
}

- (void)stop
{
    VerifyOrReturn(_connectorStarted);

    _homeManager.delegate = nil;
    _homeManager = nil;
}

- (void)homeManagerDidUpdateHomes:(HMHomeManager *)manager
{
    VerifyOrReturn(!_homeManagerReady);
    dispatch_group_leave(_homeManagerReadyGroup);
}

- (HMHome *)homeFor:(NSString *)controllerID
{
    [[HomeKitConnector sharedInstance] start];

    __auto_type * homes = _homeManager.homes;
    VerifyOrReturnValue(0 != homes.count, nil, ChipLogError(chipTool, "HomeKit is not configured with any homes."));

    NSNumber * fabricId = nil;
    if ([controllerID hasPrefix:kControllerIdPrefixStr]) {
        __auto_type * fabricIdString = [controllerID substringFromIndex:kControllerIdPrefixStr.length];
        fabricId = @([fabricIdString integerValue]);
    } else {
        fabricId = CHIPCommandBridge::GetCommissionerFabricId([controllerID UTF8String]);
    }

    // When multiple homes exist, the first controller corresponds to the first home, the second controller to the second home, etc.
    // If there are fewer homes than controllers, controllers beyond the last home will be associated with the final home in the list.
    NSUInteger index = [fabricId unsignedShortValue] - 1;
    if (index >= homes.count) {
        index = homes.count - 1;
    }

    return homes[index];
}

- (NSString *)paddedString:(NSString *)string width:(NSUInteger)width
{
    // Using length might not account for all unicode width details, but it's a simple approximation.
    NSUInteger length = string.length;
    if (length >= width) {
        return string;
    }
    NSMutableString * result = [NSMutableString stringWithString:string];
    for (NSUInteger i = 0; i < (width - length); i++) {
        [result appendString:@" "];
    }
    return result;
}

- (NSString *)repeatString:(NSString *)string count:(NSUInteger)count
{
    NSMutableString * result = [NSMutableString string];
    for (NSUInteger i = 0; i < count; i++) {
        [result appendString:string];
    }
    return result;
}

- (void)printHomes
{
    for (HMHome * home in _homeManager.homes) {
        NSUInteger maxNameLength = 0;
        NSUInteger maxNodeIDLength = 0;
        NSUInteger maxManufacturerLength = 0;
        NSUInteger maxModelLength = 0;

        __auto_type * sortedAccessories = [home.accessories sortedArrayUsingComparator:^NSComparisonResult(HMAccessory * a, HMAccessory * b) {
            return [a.name localizedCaseInsensitiveCompare:b.name];
        }];

        for (HMAccessory * accessory in sortedAccessories) {
            maxNameLength = MAX(maxNameLength, accessory.name.length);
            maxManufacturerLength = MAX(maxManufacturerLength, accessory.manufacturer.length);
            maxModelLength = MAX(maxModelLength, accessory.model.length);
            maxNodeIDLength = MAX(maxNodeIDLength, [accessory.matterNodeID stringValue].length);
        }

        __auto_type * rows = [NSMutableArray arrayWithCapacity:sortedAccessories.count];
        [sortedAccessories enumerateObjectsUsingBlock:^(HMAccessory * accessory, NSUInteger idx, BOOL * stop) {
            if (accessory.matterNodeID == nil || [accessory.matterNodeID integerValue] == 0) {
                return;
            }

            __auto_type * name = [self paddedString:accessory.name width:maxNameLength];
            __auto_type * manufacturer = [self paddedString:accessory.manufacturer width:maxManufacturerLength];
            __auto_type * model = [self paddedString:accessory.model width:maxModelLength];
            __auto_type * nodeID = [self paddedString:[accessory.matterNodeID stringValue] width:maxNodeIDLength];
            __auto_type * formattedString = [NSString stringWithFormat:@" %@ │ %@ │ %@ │ %@ ", name, manufacturer, model, nodeID];
            [rows addObject:formattedString];
        }];

        NSUInteger tableWidth = 1 + maxNameLength + 3 + maxManufacturerLength + 3 + maxModelLength + 3 + maxNodeIDLength + 1;
        NSLog(@"╔%@╗", [self repeatString:@"═" count:tableWidth]);
        NSLog(@"║%@║", [self paddedString:[NSString stringWithFormat:@" %@ [%@] ", home.name, home.matterControllerID] width:tableWidth]);
        NSLog(@"╠%@╣", [self repeatString:@"═" count:tableWidth]);
        for (NSString * row in rows) {
            NSLog(@"║%@║", row);
        }
        NSLog(@"╚%@╝", [self repeatString:@"═" count:tableWidth]);
    }
}

- (NSString *)homeControllerIDFor:(NSString *)controllerID
{
    __auto_type * home = [self homeFor:controllerID];
    return home.matterControllerID;
}

- (NSXPCConnection * (^)(void) )connectBlockFor:(NSString *)controllerID;
{
    __auto_type * home = [self homeFor:controllerID];
    ChipLogProgress(chipTool, "Controller '%s' will be associated with home '%s'.", [controllerID UTF8String], [home.matterControllerID UTF8String]);

    if ([controllerID hasPrefix:kControllerIdPrefixStr]) {
        if ([home respondsToSelector:NSSelectorFromString(@"matterStartupParametersXPCConnectBlock")]) {
            return [home valueForKey:@"matterStartupParametersXPCConnectBlock"];
        }

        ChipLogError(chipTool, "Error: 'matterStartupParametersXPCConnectBlock' not available for controller '%s'.", [controllerID UTF8String]);
        return nil;
    } else {
        return home.matterControllerXPCConnectBlock;
    }
}
@end
