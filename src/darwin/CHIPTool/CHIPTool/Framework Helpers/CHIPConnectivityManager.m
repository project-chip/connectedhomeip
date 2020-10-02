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

#import "CHIPConnectivityManager.h"
#import "DefaultsUtils.h"
#import <CHIP/CHIP.h>
#import <UIKit/UIKit.h>

@interface CHIPConnectivityManager ()
@property (readwrite) CHIPDeviceController * chipController;
@end

@implementation CHIPConnectivityManager {
    BOOL reconnectOnForeground;
}

+ (CHIPConnectivityManager *)sharedManager
{
    static CHIPConnectivityManager * manager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the device controller
        manager = [[CHIPConnectivityManager alloc] init];
    });
    return manager;
}

- (instancetype)init
{
    if (self = [super init]) {
        self.chipController = [CHIPDeviceController sharedController];
    }
    return self;
}

@end
