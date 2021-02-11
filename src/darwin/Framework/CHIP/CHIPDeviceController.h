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

#import <Foundation/Foundation.h>

@class CHIPDevice;

NS_ASSUME_NONNULL_BEGIN

@protocol CHIPDevicePairingDelegate;
@protocol CHIPPersistentStorageDelegate;

@interface CHIPDeviceController : NSObject

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error;
- (BOOL)unpairDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;
- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;

- (CHIPDevice *)getPairedDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the single CHIPDeviceController we support existing.
 */
+ (CHIPDeviceController *)sharedController;

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

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_CONTROLLER_H */
