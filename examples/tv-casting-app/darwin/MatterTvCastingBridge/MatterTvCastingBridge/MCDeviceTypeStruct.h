/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#ifndef MCDeviceTypeStruct_h
#define MCDeviceTypeStruct_h

@interface MCDeviceTypeStruct : NSObject

@property (nonatomic, strong, readonly) NSNumber * _Nonnull deviceType;

@property (nonatomic, strong, readonly) NSNumber * _Nonnull revision;

- (instancetype _Nonnull)initWithDeviceType:(uint32_t)deviceType revision:(uint16_t)revision;

@end

#endif /* MCDeviceTypeStruct_h */
