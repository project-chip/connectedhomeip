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

@class MTRBaseDevice;

NS_ASSUME_NONNULL_BEGIN

MTR_NEWLY_DEPRECATED("Please use MTRBaseDevice deviceWithNodeID")
typedef void (^MTRDeviceConnectionCallback)(MTRBaseDevice * _Nullable device, NSError * _Nullable error);

@class MTRCommissioningParameters;
@class MTRSetupPayload;
@protocol MTRDevicePairingDelegate;
@protocol MTRDeviceControllerDelegate;

@interface MTRDeviceController : NSObject

/**
 * If true, the controller has not been shut down yet.
 */
@property (readonly, nonatomic, getter=isRunning) BOOL running;

/**
 * Return the Node ID assigned to the controller.  Will return nil if the
 * controller is not running (and hence does not know its node id).
 */
@property (readonly, nonatomic, nullable) NSNumber * controllerNodeID MTR_NEWLY_AVAILABLE;

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
 * occurs.  MTRDeviceControllerDelegate will be notified as follows:
 *
 * * Discovery fails: onStatusUpdate with MTRCommissioningStatusFailed.
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
                                       error:(NSError * __autoreleasing *)error
    API_AVAILABLE(ios(16.2), macos(13.1), watchos(9.2), tvos(16.2));

/**
 * Commission the node with the given node ID.  The node ID must match the node
 * ID that was used to set up the commissioning session.
 */
- (BOOL)commissionNodeWithID:(NSNumber *)nodeID
         commissioningParams:(MTRCommissioningParameters *)commissioningParams
                       error:(NSError * __autoreleasing *)error MTR_NEWLY_AVAILABLE;

- (BOOL)continueCommissioningDevice:(void *)device
           ignoreAttestationFailure:(BOOL)ignoreAttestationFailure
                              error:(NSError * __autoreleasing *)error;

/**
 * Cancel commissioning for the given node id.  This will shut down any existing
 * commissioning session for that node id.
 */
- (BOOL)cancelCommissioningForNodeID:(NSNumber *)nodeID error:(NSError * __autoreleasing *)error MTR_NEWLY_AVAILABLE;

/**
 * Get an MTRBaseDevice for a commissioning session that was set up for the
 * given node ID.  Returns nil if no such commissioning session is available.
 */
- (nullable MTRBaseDevice *)deviceBeingCommissionedWithNodeID:(NSNumber *)nodeID
                                                        error:(NSError * __autoreleasing *)error MTR_NEWLY_AVAILABLE;

/**
 * Prepare the controller for setting up a commissioning session.
 *
 * This method is intended to be used when it is known that a setting up a commissioning session
 * will happen soon.
 * For example it may ask different subsystems to look for useful informations onto the network
 * ahead of commissioning that may then be re-used during commissioning.
 */
- (BOOL)prepareCommissioningSession:(NSError * __autoreleasing *)error MTR_NEWLY_AVAILABLE;

/**
 * Controllers are created via the MTRDeviceControllerFactory object.
 */
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Set the Delegate for the device controller  as well as the Queue on which the Delegate callbacks will be triggered
 *
 * @param[in] delegate The delegate the commissioning process should use
 *
 * @param[in] queue The queue on which the callbacks will be delivered
 */
- (void)setDeviceControllerDelegate:(id<MTRDeviceControllerDelegate>)delegate queue:(dispatch_queue_t)queue MTR_NEWLY_AVAILABLE;

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
- (NSData * _Nullable)attestationChallengeForDeviceID:(NSNumber *)deviceID MTR_NEWLY_AVAILABLE;

/**
 * Compute a PASE verifier for the desired setup passcode.
 *
 * @param[in] setupPasscode   The desired passcode to use.
 * @param[in] iterations      The number of iterations to use when generating the verifier.
 * @param[in] salt            The 16-byte salt for verifier computation.
 *
 * Returns nil on errors (e.g. salt has the wrong size), otherwise the computed
 * verifier bytes.
 */
+ (nullable NSData *)computePASEVerifierForSetupPasscode:(NSNumber *)setupPasscode
                                              iterations:(NSNumber *)iterations
                                                    salt:(NSData *)salt
                                                   error:(NSError * __autoreleasing *)error MTR_NEWLY_AVAILABLE;

/**
 * Shut down the controller. Calls to shutdown after the first one are NO-OPs.
 * This must be called, either directly or via shutting down the
 * MTRDeviceControllerFactory, to avoid leaking the controller.
 */
- (void)shutdown;

@end

@interface MTRDeviceController (Deprecated)

@property (readonly, nonatomic, nullable) NSNumber * controllerNodeId MTR_NEWLY_DEPRECATED("Please use controllerNodeID");

- (nullable NSData *)fetchAttestationChallengeForDeviceId:(uint64_t)deviceId
    MTR_NEWLY_DEPRECATED("Please use attestationChallengeForDeviceID");

- (BOOL)getBaseDevice:(uint64_t)deviceID
                queue:(dispatch_queue_t)queue
    completionHandler:(MTRDeviceConnectionCallback)completionHandler
    MTR_NEWLY_DEPRECATED("Please use [MTRBaseDevice deviceWithNodeID:controller:]");

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
    MTR_NEWLY_DEPRECATED("Please use setupCommissioningSessionWithPayload:newNodeID:error:");
- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
    MTR_NEWLY_DEPRECATED("Please use setupCommissioningSessionWithPayload:newNodeID:error:");
- (BOOL)pairDevice:(uint64_t)deviceID
    onboardingPayload:(NSString *)onboardingPayload
                error:(NSError * __autoreleasing *)error
    MTR_NEWLY_DEPRECATED("Please use setupCommissioningSessionWithPayload:newNodeID:error:");
- (BOOL)commissionDevice:(uint64_t)deviceId
     commissioningParams:(MTRCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error
    MTR_NEWLY_DEPRECATED("Please use commissionNodeWithID:commissioningParams:error:");

- (BOOL)stopDevicePairing:(uint64_t)deviceID
                    error:(NSError * __autoreleasing *)error MTR_NEWLY_DEPRECATED("Please use cancelCommissioningForNodeID:error:");

- (nullable MTRBaseDevice *)getDeviceBeingCommissioned:(uint64_t)deviceId
                                                 error:(NSError * __autoreleasing *)error
    MTR_NEWLY_DEPRECATED("Please use deviceBeingCommissionedWithNodeID:error:");

- (BOOL)openPairingWindow:(uint64_t)deviceID
                 duration:(NSUInteger)duration
                    error:(NSError * __autoreleasing *)error
    MTR_NEWLY_DEPRECATED("Please use MTRDevice or MTRBaseDevice openCommissioningWindowWithSetupPasscode");
- (nullable NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                                       duration:(NSUInteger)duration
                                  discriminator:(NSUInteger)discriminator
                                       setupPIN:(NSUInteger)setupPIN
                                          error:(NSError * __autoreleasing *)error
    MTR_NEWLY_DEPRECATED("Please use MTRDevice or MTRBaseDevice openCommissioningWindowWithSetupPasscode");

- (nullable NSData *)computePaseVerifier:(uint32_t)setupPincode
                              iterations:(uint32_t)iterations
                                    salt:(NSData *)salt
    MTR_NEWLY_DEPRECATED("Please use computePASEVerifierForSetupPasscode:iterations:salt:error:");

- (void)setPairingDelegate:(id<MTRDevicePairingDelegate>)delegate
                     queue:(dispatch_queue_t)queue MTR_NEWLY_DEPRECATED("Please use setDeviceControllerDelegate:");

@end

NS_ASSUME_NONNULL_END
