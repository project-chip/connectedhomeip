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

#ifndef CHIP_DEVICE_CONTROLLER_H
#define CHIP_DEVICE_CONTROLLER_H

#import "CHIPError.h"
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef void (^ControllerOnMessageBlock)(NSData * message, NSString * ipAddress, UInt16 port);
typedef void (^ControllerOnErrorBlock)(NSError * error);

@interface CHIPDeviceController : NSObject

@property (readonly) dispatch_queue_t appCallbackQueue;
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

- (instancetype)init:(dispatch_queue_t)appCallbackQueue;
- (BOOL)connect:(NSString *)ipAddress
           port:(UInt16)port
          error:(NSError * __autoreleasing *)error
      onMessage:(ControllerOnMessageBlock)onMessage
        onError:(ControllerOnErrorBlock)onError;
- (BOOL)sendMessage:(NSData *)message error:(NSError * __autoreleasing *)error;
- (BOOL)disconnect:(NSError * __autoreleasing *)error;
- (BOOL)isConnected;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_CONTROLLER_H */
