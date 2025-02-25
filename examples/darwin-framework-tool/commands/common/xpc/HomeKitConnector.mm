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
