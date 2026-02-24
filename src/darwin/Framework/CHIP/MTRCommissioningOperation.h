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
#import <Matter/MTRCommissioningParameters.h>
#import <Matter/MTRDeviceController.h>

NS_ASSUME_NONNULL_BEGIN

MTR_AVAILABLE(ios(26.2), macos(26.2), watchos(26.2), tvos(26.2))
@interface MTRCommissioningOperation : NSObject
/**
 * Initialized via initWithParameters:setupPayload:delegate:queue:
 */
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Prepare to commission a device with the given parameters and the given setup
 * payload (QR code, manual pairing code, etc).  Returns nil if the payload is
 * not valid.
 *
 * The deviceAttestationDelegate property of MTRCommissioningParameters will be
 * ignored. Device attestation notifications will be delivered to the
 * MTRCommissioningDelegate instead.  The failSafeTimeout property of
 * MTRCommissioningParameters will be respected.
 *
 * The provided delegate will be notified about various things as commissioning
 * proceeds.  The calls into the delegate will happen on the provided queue.
 *
 * Modifying the parameters after this call will have no effect on the behavior
 * of the MTRCommissioningOperation.
 */
- (nullable instancetype)initWithParameters:(MTRCommissioningParameters *)parameters
                               setupPayload:(NSString *)payload
                                   delegate:(id<MTRCommissioningDelegate>)delegate
                                      queue:(dispatch_queue_t)queue;

/**
 * Start commissioning with the given controller (which identifies the fabric
 * the commissionee should be commissioned into).  The delegate will be notified
 * if there are any failures.
 */
- (void)startWithController:(MTRDeviceController *)controller;

/**
 * Stop commissioning.  This will typically result in
 * commissioning:failedWithError: callbacks to delegates.
 *
 * Returns YES if this commissioning was still in-progress and has now been
 * stopped; returns NO if this commissioning wasn't in-progress.
 *
 * Note that this can return NO while there are still pending async calls to
 * delegate callbacks for the end of the commissioning.
 */
- (BOOL)stop;

/**
 * If not nil, the payload (from possibly multiple payloads represented by the
 * provided setupPayload) that represents the commissionee we successfully
 * established PASE with.  This will only be non-nil after successful PASE
 * establishment.
 */
@property (nonatomic, readonly, strong, nullable) MTRSetupPayload * matchedPayload;

@end

NS_ASSUME_NONNULL_END
