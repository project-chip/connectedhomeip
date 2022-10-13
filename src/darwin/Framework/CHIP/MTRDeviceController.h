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

#import <Foundation/Foundation.h>

#import <Matter/MTRNOCChainIssuer.h>
#import <Matter/MTROnboardingPayloadParser.h>

@class MTRBaseDevice;

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRDeviceConnectionCallback)(MTRBaseDevice * _Nullable device, NSError * _Nullable error);

@class MTRCommissioningParameters;
@protocol MTRDevicePairingDelegate;

@interface MTRDeviceController : NSObject

@property (readonly, nonatomic) BOOL isRunning;

/**
 * Return the Node Id assigned to the controller.  Will return nil if the
 * controller is not running (and hence does not know its node id).
 */
@property (readonly, nonatomic, nullable) NSNumber * controllerNodeId;

/**
 * Set up a commissioning session for a device, using the provided setup payload
 * to discover it and connect to it.
 *
 * @param payload a setup payload (probably created from a QR code or numeric
 *                code onboarding payload).
 * @param newNodeID the planned node id for the node.
 * @error error indication if discovery can't start at all (e.g. because the
 *              setup payload is invalid).
 *
 * The IP and port for the device will be discovered automatically based on the
 * provided discriminator.
 *
 * Then a PASE session will be established with the device, unless an error
 * occurs.  MTRDevicePairingDelegate will be notified as follows:
 *
 * * Discovery fails: onStatusUpdate with MTRPairingStatusFailed.
 *
 * * Discovery succeeds but commissioning session setup fails: onPairingComplete
 *   with an error.
 *
 * * Commissioning session setup succeeds: onPairingComplete with no error.
 *
 * Once a commissioning session is set up, getDeviceBeingCommissioned
 * can be used to get an MTRBaseDevice and discover what sort of network
 * credentials the device might need, and commissionDevice can be used to
 * commission the device.
 */
- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error;

/**
 * Start pairing for a device with the given ID, using the provided setup PIN
 * to establish a PASE connection.
 *
 * The IP and port for the device will be discovered automatically based on the
 * provided discriminator.
 *
 * The pairing process will proceed until a PASE session is established or an
 * error occurs, then notify onPairingComplete on the MTRDevicePairingDelegate
 * for this controller.  That delegate is expected to call commissionDevice
 * after that point if it wants to commission the device.
 */
- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error;

/**
 * Start pairing for a device with the given ID, using the provided IP address
 * and port to connect to the device and the provided setup PIN to establish a
 * PASE connection.
 *
 * The pairing process will proceed until a PASE session is established or an
 * error occurs, then notify onPairingComplete on the MTRDevicePairingDelegate
 * for this controller.  That delegate is expected to call commissionDevice
 * after that point if it wants to commission the device.
 */
- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error;

/**
 * Start pairing for a device with the given ID and onboarding payload (QR code
 * or manual setup code).  The payload will be used to discover the device and
 * establish a PASE connection.
 *
 * The pairing process will proceed until a PASE session is established or an
 * error occurs, then notify onPairingComplete on the MTRDevicePairingDelegate
 * for this controller.  That delegate is expected to call commissionDevice
 * after that point if it wants to commission the device.
 */
- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error;
- (BOOL)commissionDevice:(uint64_t)deviceId
     commissioningParams:(MTRCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error;

- (BOOL)continueCommissioningDevice:(void *)device
           ignoreAttestationFailure:(BOOL)ignoreAttestationFailure
                              error:(NSError * __autoreleasing *)error;

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error;

- (nullable MTRBaseDevice *)getDeviceBeingCommissioned:(uint64_t)deviceId error:(NSError * __autoreleasing *)error;
- (BOOL)getBaseDevice:(uint64_t)deviceID
                queue:(dispatch_queue_t)queue
    completionHandler:(MTRDeviceConnectionCallback)completionHandler;

- (BOOL)openPairingWindow:(uint64_t)deviceID duration:(NSUInteger)duration error:(NSError * __autoreleasing *)error;
- (nullable NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                                       duration:(NSUInteger)duration
                                  discriminator:(NSUInteger)discriminator
                                       setupPIN:(NSUInteger)setupPIN
                                          error:(NSError * __autoreleasing *)error;

/**
 * Controllers are created via the MTRControllerFactory object.
 */
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Set the Delegate for the Device Pairing  as well as the Queue on which the Delegate callbacks will be triggered
 *
 * @param[in] delegate The delegate the pairing process should use
 *
 * @param[in] queue The queue on which the callbacks will be delivered
 */
- (void)setPairingDelegate:(id<MTRDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue;

/**
 * Sets this MTRDeviceController to use the given issuer for issuing operational certs. By default, the MTRDeviceController uses an
 * internal issuer.
 *
 * When a nocChainIssuer is set, the device commissioner will delegate verification to the chip::Credentials::PartialDACVerifier so
 * that DAC chain and CD validation can be performed by custom code triggered by MTRNOCChainIssuer.onNOCChainGenerationNeeded().
 * Otherwise, the device commissioner uses the chip::Credentials::DefaultDACVerifier
 *
 * @param[in] nocChainIssuer the NOC Chain issuer to use for issuer operational certs
 *
 * @param[in] queue The queue on which the callbacks will be delivered
 */
- (void)setNocChainIssuer:(id<MTRNOCChainIssuer>)nocChainIssuer queue:(dispatch_queue_t)queue;

/**
 * Return the attestation challenge for the secure session of the device being commissioned.
 *
 * Attempts to retrieve the attestation challenge for a commissionee with the given Device ID.
 * Returns nil if given Device ID does not match an active commissionee, or if a Secure Session is not availale.
 */
- (nullable NSData *)fetchAttestationChallengeForDeviceId:(uint64_t)deviceId;

/**
 * Compute a PASE verifier and passcode ID for the desired setup pincode.
 *
 * @param[in] setupPincode    The desired PIN code to use
 * @param[in] iterations      The number of iterations to use when generating the verifier
 * @param[in] salt            The 16-byte salt for verifier computation
 *
 * Returns nil on errors (e.g. salt has the wrong size), otherwise the computed
 * verifier bytes.
 */
- (nullable NSData *)computePaseVerifier:(uint32_t)setupPincode iterations:(uint32_t)iterations salt:(NSData *)salt;

/**
 * Shutdown the controller. Calls to shutdown after the first one are NO-OPs.
 */
- (void)shutdown;

@end

NS_ASSUME_NONNULL_END
