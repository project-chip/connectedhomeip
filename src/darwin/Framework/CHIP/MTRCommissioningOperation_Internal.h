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
#import <Matter/MTRCommissioningOperation.h>
#import <Matter/MTRDeviceControllerDelegate.h>
#import <Matter/MTRSetupPayload.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRCommissioningOperation () <MTRDeviceControllerDelegate>

/*
 * If isInternallyCreated is YES, that also means that commissioning metrics
 * initial setup has already happened.
 */
- (instancetype)initWithParameters:(MTRCommissioningParameters *)parameters
                      setupPayload:(NSString *)payload
                   commissioningID:(NSNumber *)commissioningID
               isInternallyCreated:(BOOL)isInternallyCreated
                          delegate:(id<MTRCommissioningDelegate>)delegate
                             queue:(dispatch_queue_t)queue;

@property (nonatomic, readonly, copy) NSString * setupPayload;

@property (nonatomic, readonly, assign) BOOL isInternallyCreated;

// The commissioning identifier this operation was constructed with (the
// random "future node ID" we use to track the commissioning flow before the
// commissionee has been assigned a real node ID).  Exposed so callers like
// MTRDeviceController_Concrete that need to drive cancellation against this
// commissioning have something to pass to StopPairing without reaching into
// private ivars.
@property (nonatomic, readonly, copy) NSNumber * commissioningID;

// True if the commissioning is waiting to resume after PASE has been
// established and the delegate chose to be notified about that.
//
// This is currently only true if isInternallyCreated, and is readwrite because
// MTRDeviceController_Concrete helps maintain this state.
//
// Threading: reads and writes go through the property, which is internally
// guarded by an os_unfair_lock so the value the setter just wrote is
// observable to a synchronous reader on any queue.  The paired post-PASE
// watchdog timer (an internal implementation detail) is owned by the
// operation's _delegateQueue; the setter clears the flag synchronously and
// bounces the watchdog teardown side effect onto _delegateQueue.  The
// watchdog event_handler consults isWaitingAfterPASEEstablished as a
// late-fire guard to avoid a spurious CHIP_ERROR_TIMEOUT when the client
// has legitimately advanced past the post-PASE waiting state but a timer
// block was already enqueued ahead of the cancel.
@property (nonatomic, readwrite, assign) BOOL isWaitingAfterPASEEstablished;

// Synchronously tear down the post-PASE watchdog timer.  Safe to call from
// any queue (the helper is internally lock-guarded).  Exposed for
// MTRDeviceController_Concrete's implicit-cancel path in
// -setupCommissioningSessionWithPayload:newNodeID:error:, which must guarantee
// that no leftover watchdog timer block can fire on _delegateQueue AFTER the
// new attempt's MATTER_LOG_METRIC_BEGIN(kMetricDeviceCommissioning) -- the
// timer's event_handler routes through -_dispatchCommissioningError, which
// would otherwise end the NEW attempt's metric span and corrupt telemetry.
//
// The setter on isWaitingAfterPASEEstablished bounces the watchdog teardown
// onto _delegateQueue via dispatch_async, so simply flipping the flag does
// not provide a synchronous guarantee.  This helper does.
- (void)_syncCancelPostPASEWatchdog;

@end

NS_ASSUME_NONNULL_END
