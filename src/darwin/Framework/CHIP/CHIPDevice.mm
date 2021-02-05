/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#import "CHIPDevice_Internal.h"

@interface CHIPDevice ()

@property (nonatomic, readonly, strong, nonnull) NSRecursiveLock * lock;
@property (readonly) chip::Controller::Device * cppDevice;

@end

@implementation CHIPDevice

- (instancetype)init
{
    if (self = [super init]) {
        _lock = [[NSRecursiveLock alloc] init];
    }
    return self;
}

- (instancetype)initWithDevice:(chip::Controller::Device *)device
{
    if (self = [super init]) {
        _cppDevice = device;
    }
    return self;
}

- (chip::Controller::Device *)internalDevice
{
    return _cppDevice;
}

- (BOOL)isActive
{
    bool isActive = false;

    [self.lock lock];
    isActive = self.cppDevice->IsActive();
    [self.lock unlock];

    return isActive ? YES : NO;
}
@end
