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

#ifndef CHIP_DEVICE_H
#define CHIP_DEVICE_H

#import <Foundation/Foundation.h>

#import "CHIPError.h"

NS_ASSUME_NONNULL_BEGIN

@interface CHIPDevice : NSObject

- (BOOL)sendMessage:(NSData *)message error:(NSError * __autoreleasing *)error;
- (BOOL)sendIdentifyCommandWithDuration:(NSTimeInterval)duration;
- (BOOL)disconnect:(NSError * __autoreleasing *)error;
- (BOOL)isActive;

/**
 * Test whether a given message is likely to be a data model command.
 */
+ (BOOL)isDataModelCommand:(NSData * _Nonnull)message;

/**
 * Given a data model command, convert it to some sort of human-readable
 * string that describes what it is, as far as we can tell.
 */
+ (NSString *)commandToString:(NSData * _Nonnull)command;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_H */
