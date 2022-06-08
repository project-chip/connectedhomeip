/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "CastingServerBridge.h"
#import "CastingServer.h"

#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>

@implementation CastingServerBridge

+ (CastingServerBridge *)getSharedInstance
{
    static CastingServerBridge * instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[self alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    if (self = [super init]) {
        CHIP_ERROR err = chip::Platform::MemoryInit();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "MemoryInit failed: %s", ErrorStr(err));
            return nil;
        }

        err = chip::DeviceLayer::PlatformMgr().InitChipStack();
        if (err != CHIP_NO_ERROR) {
            ChipLogError(AppServer, "InitChipStack failed: %s", ErrorStr(err));
            return nil;
        }

        CastingServer::GetInstance()->Init();
    }
    return self;
}

// TBD: placeholder will be replaced with true CastingServer functions
- (int)add:(int)a secondNum:(int)b
{
    return a + b;
}

@end
