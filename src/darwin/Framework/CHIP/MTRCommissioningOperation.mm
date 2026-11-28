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
#import <Matter/MTRDeviceAttestationDelegate.h>

#import "MTRCommissioningDelegate_Internal.h"
#import "MTRCommissioningOperation.h"
#import "MTRCommissioningOperation_Internal.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceControllerDelegate_Internal.h"
#import "MTRDeviceController_Concrete.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRMetrics_Internal.h"
#import "MTRUnfairLock.h"

#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/core/CHIPError.h>
#include <setup_payload/SetupPayload.h>

using namespace chip;
using namespace chip::Tracing::DarwinFramework;

@interface MTRCommissioningOperationDeviceAttestationDelegate : NSObject <MTRDeviceAttestationDelegate>
@property (nonatomic, weak) MTRCommissioningOperation * commissioningOperation;
@end

@interface MTRCommissioningOperation () <MTRDeviceControllerDelegate_Internal>
@end

#ifdef DEBUG
// Declaring the selector so the compiler doesn't complain about testing for and calling it.
@protocol MTRCommissioningOperationUnitTestDelegate <MTRCommissioningDelegate>
- (void)unitTestCommissioningOperationWillEndWithError:(MTRCommissioningOperation *)commissioning;
@end
#endif

@implementation MTRCommissioningOperation {
    MTRCommissioningParameters * _parameters;
    NSNumber * _commissioningID;
    id<MTRCommissioningDelegate> __weak _delegate;
    dispatch_queue_t _delegateQueue;
    MTRDeviceController_Concrete * __weak _controller;

    // Protects _delegate and _ended.
    os_unfair_lock _lock;
    // Set by the first failure or success, so the delegate gets exactly one of them.
    BOOL _ended;
}

- (instancetype)initWithParameters:(MTRCommissioningParameters *)parameters
                      setupPayload:(NSString *)payload
                          delegate:(id<MTRCommissioningDelegate>)delegate
                             queue:(dispatch_queue_t)queue
{
    using namespace chip::Controller;

    uint64_t commissioningID;
    CHIP_ERROR err = ExampleOperationalCredentialsIssuer::GetRandomOperationalNodeId(&commissioningID);
    if (err != CHIP_NO_ERROR) {
        // Should basically never happen, so no good reason to propagate err out.
        // Not logging "self" because we have not done super init yet.
        MTR_LOG_ERROR("Unable to generate a commissioning identifier: %" CHIP_ERROR_FORMAT, err.Format());
        return nil;
    }

    return [self initWithParameters:parameters
                       setupPayload:payload
                    commissioningID:@(commissioningID)
                isInternallyCreated:NO
                           delegate:delegate
                              queue:queue];
}

- (instancetype)initWithParameters:(MTRCommissioningParameters *)parameters
                      setupPayload:(NSString *)payload
                   commissioningID:(NSNumber *)commissioningID
               isInternallyCreated:(BOOL)isInternallyCreated
                          delegate:(id<MTRCommissioningDelegate>)delegate
                             queue:(dispatch_queue_t)queue
{
    if (!(self = [super init])) {
        return nil;
    }

    if (!parameters || !payload || !commissioningID || !queue) {
        MTR_LOG_ERROR("%@ Invalid nil argument to initWithParameters", self);
        return nil;
    }

    _parameters = [parameters copy];
    _setupPayload = payload;
    _commissioningID = commissioningID;
    _isInternallyCreated = isInternallyCreated;
    _lock = OS_UNFAIR_LOCK_INIT;
    _delegate = delegate;
    _delegateQueue = queue;

    // Don't hold on to the provided attestation delegate, which we never use.
    _parameters.deviceAttestationDelegate = nil;

    return self;
}

static inline void emitMetricForSetupPayload(NSString * payload)
{
    std::vector<SetupPayload> payloads;
    CHIP_ERROR err = SetupPayload::FromStringRepresentation(payload.UTF8String, payloads);
    if (err != CHIP_NO_ERROR) {
        // Not much we can do here; can't get a VID/PID from an invalid payload.
        MTR_LOG_ERROR("Unable to parse setup payload to extract VID/PID");
        return;
    }

    if (payloads.empty()) {
        // Pretty odd that the parse succeeded!
        MTR_LOG_ERROR("Setup payload parsing succeeded but somehow did not create any payloads we can get a VID/PID from");
        return;
    }

    // Just log the first VID/PID we have; that's the best we can do.
    MATTER_LOG_METRIC(kMetricDeviceVendorID, payloads[0].vendorID);
    MATTER_LOG_METRIC(kMetricDeviceProductID, payloads[0].productID);
    uint32_t capabilities = payloads[0].rendezvousInformation.HasValue() ? payloads[0].rendezvousInformation.Value().Raw() : 0;
    MATTER_LOG_METRIC(kMetricDeviceDiscoveryCapabilities, capabilities);
}

- (void)startWithController:(MTRDeviceController *)controller
{
    auto * concreteController = MTR_SAFE_CAST(controller, MTRDeviceController_Concrete);
    if (!concreteController) {
        MTR_LOG_ERROR("%@ Cannot start commissioning with a non-concrete controller: %@", self, controller);
        [self _earlyFailCommissioning:CHIP_ERROR_INVALID_ARGUMENT];
        return;
    }

    _controller = concreteController;

    if (controller.suspended) {
        MTR_LOG_ERROR("%@ suspended: can't start commissioning %@", controller, self);
        [self _earlyFailCommissioning:CHIP_ERROR_INCORRECT_STATE];
        return;
    }

    if (concreteController.currentCommissioning) {
        MTR_LOG_ERROR("%@ Cannot start commissioning because commissioning %@ already in progress", self, concreteController.currentCommissioning);
        [self _earlyFailCommissioning:CHIP_ERROR_BUSY];
        return;
    }

    MTR_LOG("%@ starting commissioning with controller %@", self, _controller);

    if (!_isInternallyCreated) {
        [[MTRMetricsCollector sharedInstance] resetMetrics];

        // Track overall commissioning
        MATTER_LOG_METRIC_BEGIN(kMetricDeviceCommissioning);
        emitMetricForSetupPayload(_setupPayload);
    }

    CHIP_ERROR err = [concreteController startCommissioning:self withCommissioningID:_commissioningID];
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("%@ failed to start commissioning with controller %@", self, _controller);
        [self _dispatchCommissioningCHIPError:err];
    }
}

- (BOOL)stop
{
    MTRDeviceController_Concrete * strongController = _controller;
    if (!strongController) {
        // Nothing to do; controller is gone, so we are stopped no matter what.
        return NO;
    }

    return [strongController stopCommissioning:self forCommissioningID:_commissioningID];
}

- (void)_earlyFailCommissioning:(CHIP_ERROR)error
{
    // This handles failures before we have cleared the way for doing a commissioning at all, and in
    // particular we might have an existing commissioning when we land here.  Don't mess with the
    // "normal" metrics here, because we might stomp on the metrics for that ongoing commissioning.
    auto * emptyMetrics = [[MTRMetrics alloc] initWithCapacity:0];
    [self _dispatchCommissioningError:[MTRError errorForCHIPErrorCode:error] withMetrics:emptyMetrics];
}

- (void)_dispatchCommissioningCHIPError:(CHIP_ERROR)error
{
    MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, error);
    MTRMetrics * metrics = [[MTRMetricsCollector sharedInstance] metricSnapshotForCommissioning:YES];
    [self _dispatchCommissioningError:[MTRError errorForCHIPErrorCode:error] withMetrics:metrics];
}

- (void)_dispatchCommissioningError:(NSError *)error
{
    MATTER_LOG_METRIC_END(kMetricDeviceCommissioning, [MTRError errorToCHIPErrorCode:error]);
    MTRMetrics * metrics = [[MTRMetricsCollector sharedInstance] metricSnapshotForCommissioning:YES];
    [self _dispatchCommissioningError:error withMetrics:metrics];
}

- (void)_dispatchCommissioningError:(NSError *)error withMetrics:(MTRMetrics *)metrics
{
    [self _dispatchCommissioningError:error forCommissioningID:_commissioningID withMetrics:metrics];
}

- (void)_dispatchCommissioningError:(NSError *)error forCommissioningID:(NSNumber *)commissioningID withMetrics:(MTRMetrics *)metrics
{
    MTRDeviceController_Concrete * strongController = _controller;

    MTR_LOG("%@ Device commissioning failed with controller %@ metrics %@", self, strongController, metrics);

#ifdef DEBUG
    id testDelegate = [self _internalDelegate];
    if ([testDelegate respondsToSelector:@selector(unitTestCommissioningOperationWillEndWithError:)]) {
        [testDelegate unitTestCommissioningOperationWillEndWithError:self];
    }
#endif

    // Take the delegate and null it out, so we don't notify based on any
    // internal cleanups we happen to do. Only the first failure or success
    // gets it: a stop can race with a continue that fails, and each would
    // otherwise report a failure.
    id<MTRCommissioningDelegate> delegate;
    if (![self _endCommissioningReturningDelegate:&delegate]) {
        MTR_LOG("%@ Ignoring commissioning failure %@: commissioning has already ended", self, error);
        return;
    }
    auto * strongDelegate = static_cast<id<MTRCommissioningDelegate_Internal>>(delegate);

    // No matter what, notify our controller that we are done. Do this before
    // we dispath our notifications, so that in cases when the controller is not
    // our delegate its state has been updated before our delegate tries to
    // start a new commissioning, which it might want to do from the failure
    // callback. If the controller _is_ our delegate, it will ignore this call
    // until it gets the delegate callbacks.
    [strongController commissioningDone:self];

    dispatch_async(_delegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(commissioning:failedWithError:forDeviceID:metrics:)]) {
            [strongDelegate commissioning:self failedWithError:error forDeviceID:commissioningID metrics:metrics];
        } else if ([strongDelegate respondsToSelector:@selector(commissioning:failedWithError:metrics:)]) {
            [strongDelegate commissioning:self failedWithError:error metrics:metrics];
        }
    });
}

- (BOOL)_endCommissioningReturningDelegate:(id<MTRCommissioningDelegate> __strong *)delegate
{
    std::lock_guard lock(_lock);
    if (_ended) {
        *delegate = nil;
        return NO;
    }
    _ended = YES;
    *delegate = _delegate;
    _delegate = nil;
    return YES;
}

- (id<MTRCommissioningDelegate>)_currentDelegate
{
    std::lock_guard lock(_lock);
    return _delegate;
}

- (id<MTRCommissioningDelegate_Internal>)_internalDelegate
{
    return static_cast<id<MTRCommissioningDelegate_Internal>>([self _currentDelegate]);
}

#pragma mark - MTRDeviceControllerDelegate implementation

- (void)controller:(MTRDeviceController *)controller statusUpdate:(MTRCommissioningStatus)status
{
    id<MTRCommissioningDelegate_Internal> strongDelegate = [self _internalDelegate];
    dispatch_async(_delegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(commissioning:statusUpdate:)]) {
            [strongDelegate commissioning:self statusUpdate:status];
        }
    });
}

- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error forPayload:(MTRSetupPayload * _Nullable)payload
{
    if (!error && payload) {
        _matchedPayload = payload;
    }

    id<MTRCommissioningDelegate_Internal> strongDelegate = [self _internalDelegate];
    // NOTE: Doing respondsToSelector check before dispatch, so we can kick off
    // commissioning ourselves if not.
    if ([strongDelegate respondsToSelector:@selector(commissioning:paseSessionEstablishmentComplete:)]) {
        dispatch_async(_delegateQueue, ^{
            self.isWaitingAfterPASEEstablished = YES;
            [strongDelegate commissioning:self paseSessionEstablishmentComplete:error];
        });

        return;
    }

    MTRDeviceController_Concrete * strongController = _controller;
    if (!strongController) {
        // Nothing to do; controller is gone, so no point trying to go on.
        return;
    }

    // We want to use ourselves as an attestation delegate on the parameters,
    // but to avoid a reference loop, don't set ourselves directly.
    auto * attestationDelegate = [[MTRCommissioningOperationDeviceAttestationDelegate alloc] init];
    attestationDelegate.commissioningOperation = self;
    _parameters.deviceAttestationDelegate = attestationDelegate;

    NSError * commissionError;
    BOOL ok = [strongController commission:self withCommissioningID:_commissioningID commissioningParams:_parameters error:&commissionError];
    if (!ok) {
        MTR_LOG_ERROR("%@ attempt to start commissioning with controller %@ and parameters %@ failed: %@", self, strongController, _parameters, commissionError);
        // _dispatchCommissioningError first, so we don't trigger any
        // notifications from "stop".
        [self _dispatchCommissioningError:commissionError];
        [self stop];
    }
}

- (void)controller:(MTRDeviceController *)controller
    commissioningComplete:(NSError * _Nullable)error
                   nodeID:(NSNumber * _Nullable)nodeID
                  metrics:(MTRMetrics *)metrics
{
    if (error) {
        [self _dispatchCommissioningError:error forCommissioningID:nodeID withMetrics:metrics];
        return;
    }

    // Take the delegate and null it out, so we don't notify based on any
    // internal cleanups we happened to do. A failure that ended the
    // commissioning first has already been reported.
    id<MTRCommissioningDelegate> strongDelegate;
    if (![self _endCommissioningReturningDelegate:&strongDelegate]) {
        MTR_LOG("%@ Ignoring commissioning success for node %@: commissioning has already ended", self, nodeID);
        return;
    }
    MTRDeviceController_Concrete * strongController = _controller;

    // No matter what, notify our controller that we are done. Do this before
    // we dispath our notifications, so that in cases when the controller is not
    // our delegate its state has been updated before our delegate tries to
    // start a new commissioning, which it might want to do from the success
    // callback. If the controller _is_ our delegate, it will ignore this call
    // until it gets the delegate callbacks.
    [strongController commissioningDone:self];

    dispatch_async(_delegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(commissioning:succeededForNodeID:metrics:)]) {
            [strongDelegate commissioning:self succeededForNodeID:nodeID metrics:metrics];
        }
    });
}

// No need to implement deprecated or less-information versions of
// commissioningComplete: if we implement the one we do.

- (void)controller:(MTRDeviceController *)controller
    readCommissioneeInfo:(MTRCommissioneeInfo *)info
{
    id<MTRCommissioningDelegate> strongDelegate = [self _currentDelegate];
    dispatch_async(_delegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(commissioning:readCommissioneeInfo:)]) {
            [strongDelegate commissioning:self readCommissioneeInfo:info];
        }
    });
}

// No need to implement controller:readCommissioningInfo: if we implement controller:readCommissioneeInfo:

// No need to implement controller:suspendedChangedTo: for commissioning purposes.

// No need to implement controller:devicesChangedForController: for commissioning purposes.

- (void)controller:(MTRDeviceController *)controller commissioneeHasReceivedNetworkCredentials:(NSNumber *)nodeID
{
    id<MTRCommissioningDelegate_Internal> strongDelegate = [self _internalDelegate];
    dispatch_async(_delegateQueue, ^{
        if ([strongDelegate respondsToSelector:@selector(commissioning:provisionedNetworkCredentialsForDeviceID:)]) {
            [strongDelegate commissioning:self provisionedNetworkCredentialsForDeviceID:nodeID];
        } else if ([strongDelegate respondsToSelector:@selector(commissioningProvisionedNetworkCredentials:)]) {
            [strongDelegate commissioningProvisionedNetworkCredentials:self];
        }
    });
}

#pragma mark - MTRDeviceControllerDelegate_Internal implementatation

- (void)controller:(MTRDeviceController *)controller
    needsWiFiCredentialsWithScanResults:(nullable NSArray<MTRNetworkCommissioningClusterWiFiInterfaceScanResultStruct *> *)networks
                                  error:(nullable NSError *)error
{
    id<MTRCommissioningDelegate> strongDelegate = [self _currentDelegate];
    dispatch_async(_delegateQueue, ^{
        mtr_weakify(self);
        [strongDelegate commissioning:self needsWiFiCredentialsWithScanResults:networks error:error completion:^(NSData * ssid, NSData * _Nullable credentials) {
            mtr_strongify(self);

            if (!self) {
                MTR_LOG_ERROR("MTRCommissioningOperation deallocated while waiting to continue after Wi-Fi network scan");
                return;
            }

            MTRDeviceController_Concrete * strongController = self->_controller;
            if (!strongController) {
                // Controller is gone, nothing to do here.
                return;
            }

            NSError * continueError;
            BOOL ok = [strongController continueCommissioning:self
                                                 withWiFiSSID:ssid
                                                  credentials:credentials
                                                        error:&continueError];

            if (!ok) {
                MTR_LOG_ERROR("%@ attempt to continue commissioning with Wi-Fi credentials with controller %@ failed: %@", self, strongController, continueError);
                // _dispatchCommissioningError first, so we don't trigger any
                // notifications from "stop".
                [self _dispatchCommissioningError:continueError];
                [self stop];
            }
        }];
    });
}

- (void)controller:(MTRDeviceController *)controller
    needsThreadCredentialsWithScanResults:(nullable NSArray<MTRNetworkCommissioningClusterThreadInterfaceScanResultStruct *> *)networks
                                    error:(nullable NSError *)error
{
    id<MTRCommissioningDelegate> strongDelegate = [self _currentDelegate];
    dispatch_async(_delegateQueue, ^{
        mtr_weakify(self);
        [strongDelegate commissioning:self needsThreadCredentialsWithScanResults:networks error:error completion:^(NSData * operationalDataset) {
            mtr_strongify(self);

            if (!self) {
                MTR_LOG_ERROR("MTRCommissioningOperation deallocated while waiting to continue after Thread network scan");
                return;
            }

            MTRDeviceController_Concrete * strongController = self->_controller;
            if (!strongController) {
                // Controller is gone, nothing to do here.
                return;
            }

            NSError * continueError;
            BOOL ok = [strongController continueCommissioning:self withOperationalDataset:operationalDataset error:&continueError];

            if (!ok) {
                MTR_LOG_ERROR("%@ attempt to continue commissioning with Thread credentials with controller %@ failed: %@", self, strongController, continueError);
                // _dispatchCommissioningError first, so we don't trigger any
                // notifications from "stop".
                [self _dispatchCommissioningError:continueError];
                [self stop];
            }
        }];
    });
}

- (void)controllerStartingNetworkScan:(MTRDeviceController *)controller
{
    id<MTRCommissioningDelegate> strongDelegate = [self _currentDelegate];
    if ([strongDelegate respondsToSelector:@selector(commissioningStartingNetworkScan:)]) {
        dispatch_async(_delegateQueue, ^{
            [strongDelegate commissioningStartingNetworkScan:self];
        });
    }
}

#pragma mark - MTRDeviceAttestationDelegate implementation

- (void)deviceAttestationCompletedForController:(MTRDeviceController *)controller
                             opaqueDeviceHandle:(void *)opaqueDeviceHandle
                          attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                                          error:(NSError * _Nullable)error
{
    id<MTRCommissioningDelegate> strongDelegate = [self _currentDelegate];
    dispatch_async(_delegateQueue, ^{
        mtr_weakify(self);
        [strongDelegate commissioning:self completedDeviceAttestation:attestationDeviceInfo error:error completion:^{
            mtr_strongify(self);

            if (!self) {
                MTR_LOG_ERROR("MTRCommissioningOperation deallocated while waiting to continue after device attestation");
                return;
            }

            MTRDeviceController_Concrete * strongController = self->_controller;
            if (!strongController) {
                // Controller is gone, nothing to do here.
                return;
            }

            NSError * continueError;
            BOOL ok = [strongController continueCommissioningAfterAttestation:self forOpaqueHandle:opaqueDeviceHandle error:&continueError];

            if (!ok) {
                MTR_LOG_ERROR("%@ attempt to continue commissioning after device attestation with controller %@ failed: %@", self, strongController, continueError);
                // _dispatchCommissioningError first, so we don't trigger any
                // notifications from "stop".
                [self _dispatchCommissioningError:continueError];
                [self stop];
            }
        }];
    });
}

@end

@implementation MTRCommissioningOperationDeviceAttestationDelegate

- (void)deviceAttestationCompletedForController:(MTRDeviceController *)controller
                             opaqueDeviceHandle:(void *)opaqueDeviceHandle
                          attestationDeviceInfo:(MTRDeviceAttestationDeviceInfo *)attestationDeviceInfo
                                          error:(NSError * _Nullable)error
{
    MTRCommissioningOperation * strongOperation = self.commissioningOperation;
    if (!strongOperation) {
        // We are gone, nothing to do.
        return;
    }

    [strongOperation deviceAttestationCompletedForController:controller
                                          opaqueDeviceHandle:opaqueDeviceHandle
                                       attestationDeviceInfo:attestationDeviceInfo
                                                       error:error];
}

@end
