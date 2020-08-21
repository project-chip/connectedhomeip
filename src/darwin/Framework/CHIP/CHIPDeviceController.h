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

@protocol CHIPDeviceControllerDelegate;

@interface AddressInfo : NSObject

@property (readonly, copy) NSString * ip;
- (instancetype)initWithIP:(NSString *)ip;

@end

@interface CHIPDeviceController : NSObject

- (BOOL)connect:(NSString *)ipAddress error:(NSError * __autoreleasing *)error;
- (BOOL)connect:(uint16_t)discriminator setupPINCode:(uint32_t)setupPINCode error:(NSError * __autoreleasing *)error;
- (BOOL)connectWithoutSecurePairing:(NSString *)ipAddress
                              error:(NSError * __autoreleasing *)error
    __attribute__((deprecated("Available until Rendezvous is fully integrated")));
- (nullable AddressInfo *)getAddressInfo;
- (BOOL)sendMessage:(NSData *)message error:(NSError * __autoreleasing *)error;
- (BOOL)sendOnCommand;
- (BOOL)sendOffCommand;
- (BOOL)sendToggleCommand;
- (BOOL)disconnect:(NSError * __autoreleasing *)error;
- (BOOL)isConnected;

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
