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
#import <Matter/MTRCommissioningOperation.h>
#import <Matter/MTRCommissioningParameters.h>
#import <Matter/MTRDeviceController.h>
#import <Matter/MTRDeviceControllerDelegate.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MTRCommissioningDelegate_Internal <MTRCommissioningDelegate>
@required

/**
 * Internal-only callback that gets called on PASE session establishment
 * completion.  If this is implemented, then it's the delegate's job to keep
 * commissioning as needed.  Otherwise, if PASE was established succcessfully,
 * MTRCommissioningProcess will kick off the actual commissioning.
 *
 * NOTE: If we ever expose this as public API, we will want to re-work it to
 * take a completion block that takes an MTRCommissioningParameters as an
 * argument, or something along those lines.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
    paseSessionEstablishmentComplete:(NSError * _Nullable)error;

/**
 * Callback that allows implementation of the controller:statusUpdate:
 * notification.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
         statusUpdate:(MTRCommissioningStatus)status;

/**
 * Callback that allows piping the nodeID through to
 * commissioneeHasReceivedNetworkCredentials
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning provisionedNetworkCredentialsForDeviceID:(NSNumber *)deviceID;

/**
 * Callback that allows piping the nodeID through to
 * commissioningComplete
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
      failedWithError:(NSError *)error
          forDeviceID:(NSNumber *)deviceID
              metrics:(MTRMetrics *)metrics;

@end

NS_ASSUME_NONNULL_END
