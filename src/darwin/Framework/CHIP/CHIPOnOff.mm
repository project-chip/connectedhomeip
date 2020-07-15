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

#import <Foundation/Foundation.h>

#import "CHIPOnOff.h"

@interface CHIPOnOff ()

@property (readonly) CHIPDeviceController * deviceController;

@end

@implementation CHIPOnOff

- (instancetype)initWithDeviceController:(CHIPDeviceController *)deviceController
{
    if (self = [super init]) {
        _deviceController = deviceController;
    }
    return self;
}

- (BOOL)lightOn
{
    return [self.deviceController sendOnCommand];
}

- (BOOL)lightOff
{
    return [self.deviceController sendOffCommand];
}

- (BOOL)toggleLight
{
    return [self.deviceController sendToggleCommand];
}

@end
