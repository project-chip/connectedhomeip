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

#import <CHIP/CHIPDeviceStatusDelegate.h>
#import <Foundation/Foundation.h>

@class CHIPDevice;

NS_ASSUME_NONNULL_BEGIN

typedef void (^ControllerOnConnectedBlock)(void);
typedef void (^ControllerOnMessageBlock)(NSData * message);
typedef void (^ControllerOnErrorBlock)(NSError * error);

@protocol CHIPDeviceControllerDelegate;
@protocol CHIPDevicePairingDelegate;
@protocol CHIPPersistentStorageDelegate;

@interface AddressInfo : NSObject

@property (readonly, copy) NSString * ip;
- (instancetype)initWithIP:(NSString *)ip;

@end

@interface CHIPDeviceController : NSObject <CHIPDeviceStatusDelegate>

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error;
- (BOOL)unpairDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;
- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;

- (CHIPDevice *)getPairedDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;

- (BOOL)disconnect:(NSError * __autoreleasing *)error;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the single CHIPDeviceController we support existing.
 */
+ (CHIPDeviceController *)sharedController;

/**
 * Set the Delegate for the Device Controller as well as the Queue on which the Delegate callbacks will be triggered
 *
 * @param[in] delegate The delegate the Device Controller should use
 *
 * @param[in] queue The queue on which the Device Controller will deliver callbacks
 */
- (void)setDelegate:(id<CHIPDeviceControllerDelegate>)delegate queue:(dispatch_queue_t)queue;

/**
 * Set the Delegate for the Device Pairing  as well as the Queue on which the Delegate callbacks will be triggered
 *
 * @param[in] delegate The delegate the pairing process should use
 *
 * @param[in] queue The queue on which the callbacks will be delivered
 */
- (void)setPairingDelegate:(id<CHIPDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue;

/**
 * Set the Delegate for the persistent storage  as well as the Queue on which the Delegate callbacks will be triggered
 *
 * @param[in] delegate The delegate for persistent storage
 *
 * @param[in] queue The queue on which the callbacks will be delivered
 */
- (void)setPersistentStorageDelegate:(id<CHIPPersistentStorageDelegate>)delegate queue:(dispatch_queue_t)queue;

@end

/**
 * The protocol definition for the CHIPDeviceControllerDelegate
 *
 * All delegate methods will be called on the supplied Delegate Queue.
 */
@protocol CHIPDeviceControllerDelegate <NSObject>

/**
 * Notify the delegate when a connection request succeeds
 *
 */
- (void)deviceControllerOnConnected;

/**
 * Notify the delegate that a message was received
 *
 * @param[in] message The received message
 */
- (void)deviceControllerOnMessage:(NSData *)message;

/**
 * Notify the Delegate that an error occurred
 *
 * @param[in] error The error that occurred
 */
- (void)deviceControllerOnError:(NSError *)error;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_CONTROLLER_H */
