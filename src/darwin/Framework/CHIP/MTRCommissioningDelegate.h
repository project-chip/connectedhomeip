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
#import <Matter/MTRStructsObjc.h>

@class MTRCommissioningOperation;

NS_ASSUME_NONNULL_BEGIN

MTR_AVAILABLE(ios(26.2), macos(26.2), watchos(26.2), tvos(26.2))
@protocol MTRCommissioningDelegate <NSObject>
@optional
/**
 * Callback that gets called after various information (product identity,
 * optionally endpoint structure information, optionally other attributes that
 * were requested) has been read from the commissionee.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
    readCommissioneeInfo:(MTRCommissioneeInfo *)info;

@required
/**
 * Notification that device attestation has completed.
 *
 * Commissioning will pause, regardless of whether attestation succeeded or
 * failed, until the completion is invoked (indicating that commissioning should
 * proceed, even if attestation failed), or commissioning is stopped.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
    completedDeviceAttestation:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                         error:(nullable NSError *)error
                    completion:(dispatch_block_t)completion;

@optional
/**
 * Callback that gets called for a commissionee that supports Wi-Fi if
 * Wi-Fi network commissioning is required and Wi-Fi credentials were not
 * provided in MTRCommissioningParameters.
 *
 * Commissioning will pause, including in cases when the network scan failed,
 * until the completion is invoked with the network and credentials the
 * commissionee should use, or until commissioning is stopped.  The provided
 * credentials are allowed to be nil for an open Wi-Fi network.
 *
 * * error will be non-nil if a scan was attempted and failed.
 * * networks will be non-nil if a scan was attempted and succeeded.
 * * Both error and networks will be nil if no scan was performed.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
    needsWiFiCredentialsWithScanResults:(nullable NSArray<MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct *> *)networks
                                  error:(nullable NSError *)error
                             completion:(void (^)(NSData * ssid, NSData * _Nullable credentials))completion;

/**
 * Callback that gets called for a commissionee that supports Thread if Thread
 * network commissioning is required and an operational dataset was not provided
 * in MTRCommissioningParameters.
 *
 * Commissioning will pause, including in cases when the network scan failed,
 * until the completion is invoked with the operational dataset the commissionee
 * should use, or until commissioning is stopped.
 *
 * * error will be non-nil if a scan was attempted and failed.
 * * networks will be non-nil if a scan was attempted and succeeded.
 * * Both error and networks will be nil if no scan was performed.
 */
- (void)commissioning:(MTRCommissioningOperation *)commissioning
    needsThreadCredentialsWithScanResults:(nullable NSArray<MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct *> *)networks
                                    error:(nullable NSError *)error
                               completion:(void (^)(NSData * operationalDataset))completion;

/**
 * Notification that a network scan is starting.  This will only happen if a
 * network scan is performed during commissioning.
 */
- (void)commissioningStartingNetworkScan:(MTRCommissioningOperation *)commissioning;

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
