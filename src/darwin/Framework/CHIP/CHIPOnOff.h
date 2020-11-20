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

#ifndef CHIP_ONOFF_H
#define CHIP_ONOFF_H

#import <CHIP/CHIPDeviceCallback.h>
#import <Foundation/Foundation.h>

@class CHIPDevice;

NS_ASSUME_NONNULL_BEGIN

@interface CHIPOnOff : NSObject

- (nullable instancetype)initWithDevice:(CHIPDevice *)device endpoint:(uint8_t)endpoint queue:(dispatch_queue_t)queue;
- (BOOL)lightOn:(CHIPDeviceCallback)onCompletion;
- (BOOL)lightOff:(CHIPDeviceCallback)onCompletion;
- (BOOL)toggleLight:(CHIPDeviceCallback)onCompletion;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_ONOFF_H */
