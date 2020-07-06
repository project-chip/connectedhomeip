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
#import "ChipBleDelegate.h"
#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

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
- (nullable AddressInfo *)getAddressInfo;
- (BOOL)sendMessage:(NSData *)message error:(NSError * __autoreleasing *)error;
// We can't include definitions of ChipZclClusterId_t and ChipZclCommandId_t
// here, but they're just integers, so pass them that way.
- (BOOL)sendCHIPCommand:(uint16_t)cluster command:(uint16_t)command;
- (BOOL)disconnect:(NSError * __autoreleasing *)error;
- (BOOL)isConnected;

@property (strong) PreparationCompleteHandler BleConnectionPreparationCompleteHandler;
@property (readonly) CBPeripheral * blePeripheral;
@property (atomic, readonly) dispatch_queue_t WorkQueue;
@property (atomic, strong, readonly) ChipBleDelegate * mBleDelegate;

- (void)connectBle:(CBPeripheral *)peripheral;

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
                onMessage:(ControllerOnMessageBlock)onMessage
                  onError:(ControllerOnErrorBlock)onError;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_CONTROLLER_H */
