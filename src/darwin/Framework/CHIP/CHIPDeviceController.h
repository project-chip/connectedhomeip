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

typedef void (^ControllerOnConnectedBlock)(void);
typedef void (^ControllerOnMessageBlock)(NSData * message);
typedef void (^ControllerOnErrorBlock)(NSError * error);

@interface AddressInfo : NSObject

@property (readonly, copy) NSString * ip;
- (instancetype)initWithIP:(NSString *)ip;

@end

@interface CHIPDeviceController : NSObject

- (BOOL)connect:(NSString *)ipAddress
      local_key:(NSData *)local_key
       peer_key:(NSData *)peer_key
          error:(NSError * __autoreleasing *)error;
- (BOOL)connect:(uint16_t)discriminator setupPINCode:(uint32_t)setupPINCode error:(NSError * __autoreleasing *)error;
- (nullable AddressInfo *)getAddressInfo;
- (BOOL)sendMessage:(NSData *)message error:(NSError * __autoreleasing *)error;
- (BOOL)sendOnCommand;
- (BOOL)sendOffCommand;
- (BOOL)sendToggleCommand;
- (BOOL)disconnect:(NSError * __autoreleasing *)error;
- (BOOL)isConnected;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the single CHIPDeviceController we support existing.
 */
+ (CHIPDeviceController *)sharedController;

/**
 * Register callbacks for network activity.
 *
 * @param[in] appCallbackQueue the queue that should be used to deliver the
 *                             message/error callbacks for this consumer.
 *
 * @param[in] onMessage the block to call when the controller gets a message
 *                      from the network.
 *
 * @param[in] onError the block to call when there is a network error.
 */
- (void)registerCallbacks:(dispatch_queue_t)appCallbackQueue
              onConnected:(ControllerOnConnectedBlock)onConnected
                onMessage:(ControllerOnMessageBlock)onMessage
                  onError:(ControllerOnErrorBlock)onError;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_CONTROLLER_H */
