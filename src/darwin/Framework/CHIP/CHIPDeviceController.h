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

#import <CHIP/CHIPOnboardingPayloadParser.h>

@class CHIPDevice;

NS_ASSUME_NONNULL_BEGIN

typedef void (^CHIPDeviceConnectionCallback)(CHIPDevice * _Nullable device, NSError * _Nullable error);

@protocol CHIPDevicePairingDelegate;
@protocol CHIPPersistentStorageDelegate;
@protocol CHIPKeypair;

@interface CHIPDeviceController : NSObject

@property (readonly, nonatomic) BOOL isRunning;

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error;

- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error;

- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error;

- (void)setListenPort:(uint16_t)port;
- (BOOL)unpairDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;
- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;
- (void)updateDevice:(uint64_t)deviceID fabricId:(uint64_t)fabricId;

- (BOOL)isDevicePaired:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;
- (nullable CHIPDevice *)getDeviceBeingCommissioned:(uint64_t)deviceId error:(NSError * __autoreleasing *)error;
- (BOOL)getConnectedDevice:(uint64_t)deviceID
                     queue:(dispatch_queue_t)queue
         completionHandler:(CHIPDeviceConnectionCallback)completionHandler;

- (BOOL)openPairingWindow:(uint64_t)deviceID duration:(NSUInteger)duration error:(NSError * __autoreleasing *)error;
- (nullable NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                                       duration:(NSUInteger)duration
                                  discriminator:(NSUInteger)discriminator
                                       setupPIN:(NSUInteger)setupPIN
                                          error:(NSError * __autoreleasing *)error;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Return the single CHIPDeviceController we support existing.
 */
+ (CHIPDeviceController *)sharedController;

/**
 * Return the Node Id assigned to the controller.
 */
- (NSNumber *)getControllerNodeId;

/**
 * Set the Delegate for the Device Pairing  as well as the Queue on which the Delegate callbacks will be triggered
 *
 * @param[in] delegate The delegate the pairing process should use
 *
 * @param[in] queue The queue on which the callbacks will be delivered
 */
- (void)setPairingDelegate:(id<CHIPDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue;

/**
 * Start the CHIP Stack. Repeated calls to startup without calls to shutdown in between are NO-OPs. Use the isRunning property to
 * check if the stack needs to be started up.
 *
 * @param[in] storageDelegate The delegate for persistent storage
 * @param[in] vendorId The vendor ID of the commissioner application
 * @param[in] nocSigner The CHIPKeypair that is used to generate and sign Node Operational Credentials
 */
- (BOOL)startup:(_Nullable id<CHIPPersistentStorageDelegate>)storageDelegate
       vendorId:(uint16_t)vendorId
      nocSigner:(nullable id<CHIPKeypair>)nocSigner;

/**
 * Shutdown the CHIP Stack. Repeated calls to shutdown without calls to startup in between are NO-OPs.
 */
- (BOOL)shutdown;

@end

NS_ASSUME_NONNULL_END

#endif /* CHIP_DEVICE_CONTROLLER_H */
