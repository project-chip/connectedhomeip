/**
 *    Copyright (c) 2025 Project CHIP Authors
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
#import <Matter/MTRCommissioningDelegate.h>
#import <Matter/MTRCommissioningParameters.h> // TODO: Needed?
#import <Matter/MTRDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

MTR_UNSTABLE_API
@interface MTRCommissioningOperation
/**
 * Initialized via initWithParameters:setupPayload:.
 */
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Prepare to do a new commissioning of a device with the given parameters and
 * the given setup payload (QR code, manual pairing code, etc).  Returns nil if
 * the payload is not valid.
 *
 * The deviceAttestationDelegate property of MTRCommissioningParameters will be
 * ignored. Device attestation notifications will be delivered to any
 * registered MTRCommissioningDelegates instead.
 *
 * TODO: Decide whether MTRCommissioningParameters should be an argument here or
 * whether MTRCommissioningOperation should just inherit from
 * MTRCommissioningParameters.  Or whether we should move some subset of those
 * configuration options onto this object.
 */
- (instancetype)initWithParameters:(MTRCommissioningParameters *)parameters
                      setupPayload:(NSString *)payload;

/**
 * Start commissioning with the given controller (which identifies the fabric
 * the commissionee should be commissioned into).
 *
 * TODO: Figure out how to communicate failures here.  Perhaps just a completion
 * for the whole thing?  Need to be a bit careful about racing with the delegate
 * succeeded/failed notifications.  Maybe we should just async-dispatch those as
 * needed even for sync failures (if any) in this method?
 */
- (void)startWithController:(MTRDeviceController *)controller;

/**
 * Stop commissioning.  This will typically result in
 * commissioning:failedWithError: callbacks to delegates.
 */
- (void)stop;

/**
 * Add a delegate that will be notified about various things as commissioning
 * proceeds.  The calls into the delegate will happen on the provided queue.
 *
 * MTRCommissioningOperation will not hold a strong reference to the delegate;
 * the delegate should be kept alive by the caller.
 */
- (void)addDelegate:(id<MTRCommissioningDelegate>)delegate queue:(dispatch_queue_t)queue;

/**
 * Remove a delegate.
 */
- (void)removeDelegate:(id<MTRCommissioningDelegate>)delegate;

/**
 * Whether to perform a network scan as part of commissioning, if network
 * commissioning is required.  Defaults to NO.
 *
 * Changing this value after commissioning has started is not allowed, attempts
 * to do so will just not change the value.
 *
 * If this is set to YES and a scan is performed, commissioning will pause after
 * the scan, until the completion from the relevant delegate notification is
 * called.
 */
@property (nonatomic, readwrite, assign) BOOL scanNetworks;

// TODO: Figure out whether MTROperationalCertificateIssuer remains tied to the
// controller or whether we can/should move it here somehow.
// MTROperationalCertificateIssuer in its current form doesn't have a way to
// tell _which_ commissioning is involved, so we probably need to either change
// it (e.g. by passing it an MTRCommissioningOperation) or use something else
// here instead and figure out how to make that work.

// TODO: Once we start commissioning, should we expose the controller we got
// started with?

@end

NS_ASSUME_NONNULL_END
