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
#import <Matter/MTRCommissioneeInfo.h>
#import <Matter/MTRDeviceAttestationDelegate.h> // For MTRDeviceAttestationDeviceInfo
#import <Matter/MTRMetrics.h>

@class MTRCommissioningOperation; // Can't be imported, since it needs to reference us.
@class MTRThreadScanResult; // TODO: Needs to be defined
@class MTRWiFiScanResult; // TODO: Needs to be defined

NS_ASSUME_NONNULL_BEGIN

MTR_UNSTABLE_API
@protocol MTRCommissioningDelegate <NSObject>
@optional
/**
 * Callback that gets called after various information (product identity,
 * optionally endpoint structure information, optionally other attributes that
 * were requested) has been read from the commissionee.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
 readCommissioneeInfo:(MTRCommissioneeInfo *)info;

/**
 * Notification that device attestation has completed.
 *
 * Commissioning will pause, regardless of whether attestation succeeded or
 * failed, until the completion is invoked.
 */
// TODO: Need to sort out
// https://github.com/project-chip/connectedhomeip/issues/26695 here because
// MTRDeviceAttestationDeviceInfo is not really the right thing here, probably.
- (void)commissioning:(MTRCommissioningOperation *)commissioning
completedDeviceAttestation:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                error:(nullable NSError *)error
           completion:(dispatch_block_t)completion;

/**
 * Callback that gets called for a commissionee that supports Wi-Fi if
 * scanNetworks was set to true on MTRCommissioningOperation and network
 * commissioning is required.
 *
 * Commissioning will pause, including in cases when the network scan failed,
 * until the completion is invoked with the network and credentials the
 * commissionee should use.  If multiple delegates invoke the completion, all
 * but the first call will be ignored.
 *
 * Exactly one of networks and error will be non-nil.
 */
- (void)comnissioning:(MTRCommissioningOperation *)commissioning
  scannedWiFiNetworks:(nullable NSArray<MTRWiFiScanResult *> *)networks
                error:(nullable NSError *)error
           completion:(void (^)(NSData * ssid, NSData * credentials))completion;

/**
 * Callback that gets called for a commissionee that supports Thread if
 * scanNetworks was set to true on MTRCommissioningOperation and network
 * commissioning is required.
 *
 * Commissioning will pause, including in cases when the network scan failed,
 * until the completion is invoked with the operational dataset the commissionee
 * should use.  If multiple delegates invoke the completion, all but the first
 * call will be ignored.
 *
 * Exactly one of networks and error will be non-nil.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
scannedThreadNetworks:(nullable NSArray<MTRThreadScanResult *> *)networks
                error:(nullable NSError *)error
           completion:(void (^)(NSData * operationalDataset))completion;

/**
 * Notification that network credentials have been successfully communicated
 * to the commissionee and it's going to try to join that network.  Note that
 * for commissionees that are already on-network this notification will not
 * happen.
 */
- (void)commissioningProvisionedNetworkCredentials:(MTRCommissioningOperation *)commissioning;

/**
 * Notification that commissioning has failed.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
      failedWithError:(NSError *)error
              metrics:(MTRMetrics *)metrics;

/**
 * Notification that commissioning has succeeded.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
   succeededForNodeID:(NSNumber *)nodeID
              metrics:(MTRMetrics *)metrics;
@end

NS_ASSUME_NONNULL_END
