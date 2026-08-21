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
#import <os/lock.h>

#import "MTRCommissioningDelegate_Internal.h"
#import "MTRCommissioningOperation.h"
#import "MTRCommissioningOperation_Internal.h"
#import "MTRCommissioningOperation_Test.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceControllerDelegate_Internal.h"
#import "MTRDeviceController_Concrete.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRMetricKeys.h"
#import "MTRMetricsCollector.h"
#import "MTRMetrics_Internal.h"

#include <math.h>
#include <stdatomic.h>

#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/core/CHIPError.h>
#include <setup_payload/SetupPayload.h>

using namespace chip;
using namespace chip::Tracing::DarwinFramework;

// Production interval after which the post-PASE watchdog fires.  Five
// minutes is comfortably longer than the commissionee fail-safe (60-180
// seconds) but short enough that the user gets unblocked well within a
// single sit-down troubleshooting session.  Exposed via
// MTRCommissioningOperation_Test.h so tests can assert against the same
// value production uses.
extern const NSTimeInterval kMTRPostPASEWatchdogInterval = 5 * 60;

// Test-only override for kMTRPostPASEWatchdogInterval.  Non-zero values
// take effect for all subsequently armed watchdogs in the current
// process; production reads kMTRPostPASEWatchdogInterval directly.
//
// Declared as a C11 atomic so the test setter (which can run on any
// thread) and the production read in -_armPostPASEWatchdog (which runs
// on _delegateQueue) are not a formal data race under the
// ObjC/C11 memory model.  Relaxed ordering is sufficient: the override
// is monotonic from the perspective of any individual armed watchdog
// (test setters are sequenced before the commissioning flow they want
// to observe), and there are no accompanying memory operations whose
// visibility we need to fence on this load/store.
static _Atomic(NSTimeInterval) sMTRPostPASEWatchdogIntervalForTesting = 0;

@interface MTRCommissioningOperationDeviceAttestationDelegate : NSObject <MTRDeviceAttestationDelegate>
@property (nonatomic, weak) MTRCommissioningOperation * commissioningOperation;
@end

@interface MTRCommissioningOperation () <MTRDeviceControllerDelegate_Internal>
@end

@implementation MTRCommissioningOperation {
    MTRCommissioningParameters * _parameters;
    NSNumber * _commissioningID;
    id<MTRCommissioningDelegate> __weak _delegate;
    dispatch_queue_t _delegateQueue;
    MTRDeviceController_Concrete * __weak _controller;
    // Watchdog timer armed once PASE has been established to bound how long we
    // hold the controller in the "in-progress commissioning" state if the
    // client drops the flow without ever calling commissionNodeWithID: or
    // cancelCommissioningForNodeID:.  Without this watchdog,
    // _currentInternalCommissioning is never cleared and every subsequent
    // commissioning attempt hits CHIP_ERROR_BUSY (0xDB) until the process
    // restarts.
    //
    // _postPASEWatchdog is read/written under _stateLock so the helpers
    // are safe to invoke from any queue.  The timer source's
    // event_handler is delivered on _delegateQueue.
    dispatch_source_t _postPASEWatchdog;
    // Backing ivar for the isWaitingAfterPASEEstablished property.  Reads and
    // writes are serialized by _stateLock so callers from any queue see a
    // consistent value immediately after the setter returns; the watchdog
    // teardown that runs as a side effect of clearing the flag is bounced
    // onto _delegateQueue (which is the only queue allowed to touch
    // _postPASEWatchdog).
    BOOL _isWaitingAfterPASEEstablished;
    os_unfair_lock _stateLock;
    // Test-only one-shot fault injection.  When YES, the next call to
    // _armPostPASEWatchdog will simulate a dispatch_source_create failure
    // (return NO without arming the timer) and clear the flag.  Reads and
    // writes are serialized by _stateLock so that the test setter can run
    // synchronously from any queue and still publish the flag write before
    // returning -- without it, tests had to rely on FIFO ordering on
    // _delegateQueue and explicit dispatch_sync flushes to observe the
    // injected fault.
    BOOL _forceNextArmFailureForTesting;
}

@synthesize commissioningID = _commissioningID;

- (BOOL)isWaitingAfterPASEEstablished
{
    os_unfair_lock_lock(&_stateLock);
    BOOL value = _isWaitingAfterPASEEstablished;
    os_unfair_lock_unlock(&_stateLock);
    return value;
}

- (void)setIsWaitingAfterPASEEstablished:(BOOL)isWaitingAfterPASEEstablished
{
    // Write the flag synchronously under _stateLock so that any caller that
    // immediately reads back isWaitingAfterPASEEstablished observes the
    // updated value (callers like MTRDeviceController_Concrete depend on
    // this caller-visible ordering).  Only the watchdog teardown side
    // effect is bounced onto _delegateQueue, since _postPASEWatchdog itself
    // is _delegateQueue-owned.
    BOOL changedToNo = NO;
    os_unfair_lock_lock(&_stateLock);
    if (_isWaitingAfterPASEEstablished != isWaitingAfterPASEEstablished) {
        _isWaitingAfterPASEEstablished = isWaitingAfterPASEEstablished;
        changedToNo = !isWaitingAfterPASEEstablished;
    }
    os_unfair_lock_unlock(&_stateLock);

    if (changedToNo) {
        // The client has either started commissioning (via
        // commissionNodeWithID:) or moved on; the watchdog no longer needs
        // to fire.  The cancel must happen on _delegateQueue because that
        // is where _postPASEWatchdog is owned and where the timer's
        // event_handler is delivered.
        dispatch_async(_delegateQueue, ^{
            [self _cancelPostPASEWatchdog];
        });
    }
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
    // Honor the `copy` semantic on the @property declaration: a caller passing
    // an NSMutableString must not be able to mutate our stored payload after
    // construction.  The implicit-cancel path in setupCommissioningSessionWithPayload:
    // compares the stored payload against the caller's freshly-derived
    // pairingCode via -isEqualToString:, so any post-construction drift here
    // would silently corrupt that decision.
    _setupPayload = [payload copy];
    _commissioningID = [commissioningID copy];
    _isInternallyCreated = isInternallyCreated;
    _delegate = delegate;
    _delegateQueue = queue;
    _stateLock = OS_UNFAIR_LOCK_INIT;

    // Don't hold on to the provided attestation delegate, which we never use.
    _parameters.deviceAttestationDelegate = nil;

    return self;
}

- (void)dealloc
{
    // Defensive: ensure the watchdog timer doesn't outlive us.  In all
    // non-dealloc code paths, _postPASEWatchdog is read/written under
    // _stateLock.  In dealloc itself the timer source's event_handler
    // cannot still be in flight (the handler retains self via mtr_strongify
    // for its duration), and no other strong references can exist (or we
    // wouldn't be deallocating), so a lock-free synchronous cancel here is
    // safe.
    if (_postPASEWatchdog) {
        dispatch_source_cancel(_postPASEWatchdog);
        _postPASEWatchdog = nil;
    }
}

- (BOOL)_armPostPASEWatchdog
{
    // _postPASEWatchdog is guarded by _stateLock so this helper is safe
    // to call from any queue (production callers run from _delegateQueue;
    // tests sometimes invoke it directly from the test thread).

    // Test-only one-shot fault injection: simulate a
    // dispatch_source_create failure so callers can exercise the
    // "watchdog could not be armed -> bail out" path without having to
    // actually exhaust dispatch sources.  Read-and-clear under _stateLock
    // so a setter on any queue is synchronously visible here without
    // requiring tests to flush _delegateQueue first.
    BOOL forceFailure;
    os_unfair_lock_lock(&_stateLock);
    forceFailure = _forceNextArmFailureForTesting;
    if (forceFailure) {
        _forceNextArmFailureForTesting = NO;
    }
    os_unfair_lock_unlock(&_stateLock);
    if (forceFailure) {
        MTR_LOG_ERROR("%@ post-PASE watchdog arm forced to fail (testing)", self);
        return NO;
    }

    // Production interval lives in kMTRPostPASEWatchdogInterval; tests
    // can shorten it via setPostPASEWatchdogIntervalForTesting:.  The
    // override is loaded atomically so that concurrent test setters on
    // any thread do not race with this read under TSan.
    NSTimeInterval intervalOverride = atomic_load_explicit(&sMTRPostPASEWatchdogIntervalForTesting, memory_order_relaxed);
    NSTimeInterval intervalSeconds = (intervalOverride > 0)
        ? intervalOverride
        : kMTRPostPASEWatchdogInterval;

    // Check the double-arm guard under _stateLock without creating the
    // source yet -- we want the source to be fully configured and
    // dispatch_resume'd before we publish it via _postPASEWatchdog,
    // because a concurrent _cancelPostPASEWatchdog that grabbed a
    // suspended source would crash when ARC released its last strong
    // ref (libdispatch aborts on releasing a suspended source).
    os_unfair_lock_lock(&_stateLock);
    if (_postPASEWatchdog) {
        // Already armed; should not happen but defend against double-arm.
        os_unfair_lock_unlock(&_stateLock);
        return YES;
    }
    os_unfair_lock_unlock(&_stateLock);

    dispatch_source_t newTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _delegateQueue);
    if (!newTimer) {
        MTR_LOG_ERROR("%@ failed to create post-PASE watchdog timer", self);
        return NO;
    }

    dispatch_source_set_timer(newTimer,
        dispatch_time(DISPATCH_TIME_NOW, (int64_t) (intervalSeconds * NSEC_PER_SEC)),
        DISPATCH_TIME_FOREVER,
        1 * NSEC_PER_SEC);

    mtr_weakify(self);
    dispatch_source_set_event_handler(newTimer, ^{
        mtr_strongify(self);
        if (!self) {
            return;
        }
        // Late-fire guard: if the client has already advanced past the
        // post-PASE waiting state (e.g. the client's commissionNodeWithID:
        // succeeded and the controller flipped isWaitingAfterPASEEstablished
        // back to NO) but the timer block had already been posted to the
        // queue ahead of the cancel, we must NOT route a spurious
        // CHIP_ERROR_TIMEOUT through the failure path -- doing so would
        // tear down a commissioning that is now legitimately in flight.
        if (!self.isWaitingAfterPASEEstablished) {
            return;
        }
        MTR_LOG_ERROR("%@ post-PASE watchdog fired -- client never advanced past paseSessionEstablishmentComplete; cancelling commissioning to release controller", self);
        [self _firePostPASEWatchdog];
    });

    dispatch_resume(newTimer);

    // Publish under _stateLock.  A racing _cancelPostPASEWatchdog that
    // ran during the unlocked window above will simply have observed
    // _postPASEWatchdog == nil and become a no-op; we then arm here and
    // the next cancel call will properly tear this one down.  If that
    // racing cancel ran AFTER our nil-check window (i.e. after we got
    // here), it will pick up newTimer below and cancel a resumed source,
    // which is safe.
    dispatch_source_t toCancelIfRaceLost = nil;
    os_unfair_lock_lock(&_stateLock);
    if (_postPASEWatchdog) {
        // Lost a race with another _armPostPASEWatchdog call.  Tear down
        // our just-created timer and keep the winner.
        toCancelIfRaceLost = newTimer;
    } else {
        _postPASEWatchdog = newTimer;
    }
    os_unfair_lock_unlock(&_stateLock);

    if (toCancelIfRaceLost) {
        dispatch_source_cancel(toCancelIfRaceLost);
    }
    return YES;
}

- (void)_cancelPostPASEWatchdog
{
    // _postPASEWatchdog is guarded by _stateLock so this helper is safe
    // to call from any queue.  dispatch_source_cancel is invoked outside
    // the locked region to keep the critical section minimal and avoid
    // any reentrancy risk if the source's cleanup ever touched our lock.
    dispatch_source_t toCancel = nil;
    os_unfair_lock_lock(&_stateLock);
    toCancel = _postPASEWatchdog;
    _postPASEWatchdog = nil;
    os_unfair_lock_unlock(&_stateLock);

    if (toCancel) {
        dispatch_source_cancel(toCancel);
    }
}

- (void)_syncCancelPostPASEWatchdog
{
    // Public-to-controller alias for -_cancelPostPASEWatchdog, which is
    // already synchronous (lock-guarded read+clear of _postPASEWatchdog
    // followed by an immediate dispatch_source_cancel; no dispatch_async).
    //
    // The distinction matters at the call site: the controller's
    // implicit-cancel path in -setupCommissioningSessionWithPayload: needs
    // a guarantee that, by the time this method returns, the watchdog
    // timer has been cancelled and no future timer block will fire on
    // _delegateQueue.  Going through -setIsWaitingAfterPASEEstablished:NO
    // would only enqueue the teardown via dispatch_async, leaving a window
    // where a timer block already on _delegateQueue could still run after
    // the controller proceeded to MATTER_LOG_METRIC_BEGIN for the new
    // attempt -- corrupting the new attempt's metric span.
    [self _cancelPostPASEWatchdog];
}

- (void)_firePostPASEWatchdog
{
    // The handler is delivered on _delegateQueue.  Self-cancel by asking the
    // controller to stop commissioning for our commissioningID (which routes
    // through StopPairing in both internal and non-internal flows), then
    // notify the delegate via the standard error path so any UI / client
    // state can settle back to "no commissioning in flight".
    //
    // Use the atomic helper -stopCommissioningAtomically:forCommissioningID:
    // so the "are we still the current commissioning?" gate check and the
    // StopPairing call happen under a single controller lock.  The previous
    // approach read currentCommissioning, computed `replaced`, then called
    // stopCommissioning:forCommissioningID: as two independent reads of
    // currentCommissioning, which had a TOCTOU window: a concurrent
    // setupCommissioningSessionWithPayload: on another thread could swap
    // currentCommissioning between the two reads and we would either
    // dispatch a spurious CHIP_ERROR_TIMEOUT to a now-detached delegate or
    // fail to release a still-current C++ commissioning slot.

    [self _cancelPostPASEWatchdog];

    MTRDeviceController_Concrete * strongController = _controller;
    MTR_LOG_ERROR("%@ post-PASE watchdog fired for commissioning %@ -- client never advanced past paseSessionEstablishmentComplete; cancelling commissioning to release controller", self, _commissioningID);

    if (!strongController) {
        // Controller is gone -- the very wedge we wanted to prevent is moot
        // (no controller state to wedge), but we still need to drive our own
        // terminal error so the delegate state settles.
        [self _dispatchCommissioningError:[MTRError errorForCHIPErrorCode:CHIP_ERROR_TIMEOUT]];
        return;
    }

    // Clear the waiting-after-PASE flag BEFORE asking the controller to stop.
    // Otherwise -_cancelCommissioning would observe the flag still YES and
    // synthesize an OnCommissioningComplete(CHIP_ERROR_CANCELLED) via the
    // delegate bridge, which would (a) end kMetricDeviceCommissioning a
    // second time (we end it ourselves below via _dispatchCommissioningError)
    // and (b) enqueue a spurious CANCELLED notification onto _delegateQueue
    // ahead of the TIMEOUT we want to deliver.  By the time the watchdog
    // fires we are already certain the C++ side has no notifications coming
    // (the wedge IS that no notifications arrived), so suppressing the
    // synthetic CANCELLED is correct.
    os_unfair_lock_lock(&_stateLock);
    _isWaitingAfterPASEEstablished = NO;
    os_unfair_lock_unlock(&_stateLock);

    MTRStopCommissioningOutcome outcome = [strongController stopCommissioningAtomically:self forCommissioningID:_commissioningID];
    switch (outcome) {
    case MTRStopCommissioningOutcomeReplaced:
        // The commissioning slot has already been taken by a successor
        // MTRCommissioningOperation.  The replacement path already cleared
        // our delegate via commissioningDone:, so we must NOT dispatch a
        // terminal error -- doing so would tear down the unrelated
        // successor's delegate state.
        MTR_LOG("%@ post-PASE watchdog fired but commissioning %@ was already replaced; suppressing spurious timeout", self, _commissioningID);
        return;
    case MTRStopCommissioningOutcomeStopped:
    case MTRStopCommissioningOutcomeFailedStillCurrent:
        // Stopped: the timeout-while-still-current case -- escalate the
        // timeout to the client so it can settle back to "no commissioning
        // in flight".
        // FailedStillCurrent: StopPairing failed at the CHIP layer (e.g.
        // the commissionee fail-safe (60-180s) has already expired before
        // the 5-minute watchdog fires).  The wedge persists, so we MUST
        // still escalate the timeout to the client -- otherwise the very
        // wedge the watchdog exists to break is left in place silently.
        [self _dispatchCommissioningError:[MTRError errorForCHIPErrorCode:CHIP_ERROR_TIMEOUT]];
        return;
    }
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
    // Cancel the watchdog synchronously here.  _cancelPostPASEWatchdog is
    // queue-agnostic (lock-guarded) so it is safe to call on whatever
    // queue invoked -stop.  The flag-clear under _stateLock further down
    // provides a belt-and-suspenders late-fire guard for any
    // event_handler block already enqueued on _delegateQueue ahead of
    // the cancel: that block will observe isWaitingAfterPASEEstablished
    // == NO and bail out instead of routing a spurious CHIP_ERROR_TIMEOUT
    // through the failure path.
    //
    // Doing the cancel synchronously (rather than bouncing it onto
    // _delegateQueue via dispatch_async as the previous implementation
    // did) ensures that by the time -stop returns to its caller, the
    // dispatch_source has been cancelled and a subsequent test or caller
    // observing the operation's internal state will not see a still-live
    // watchdog timer.
    [self _cancelPostPASEWatchdog];

    MTRDeviceController_Concrete * strongController = _controller;
    BOOL stopResult;
    if (!strongController) {
        // Nothing to do; controller is gone, so we are stopped no matter what.
        stopResult = NO;
    } else {
        stopResult = [strongController stopCommissioning:self forCommissioningID:_commissioningID];
    }

    // Clear the waiting-after-PASE flag synchronously AFTER the controller
    // transition.  If we cleared it before stopCommissioning:, the watchdog's
    // late-fire guard could observe (flag=NO, currentCommissioning=self) and
    // suppress a fire that legitimately needed to drive cleanup.  By clearing
    // after the controller transition, either:
    //   - the watchdog fires before our clear: it sees flag=YES and runs its
    //     normal fire path, which is fine because stopCommissioning above is
    //     idempotent (a duplicate stopCommissioning: returns NO and the
    //     watchdog suppresses the timeout via the "replaced" check), OR
    //   - the watchdog fires after our clear: the late-fire guard suppresses
    //     it, also fine, since we have already done the controller transition.
    os_unfair_lock_lock(&_stateLock);
    _isWaitingAfterPASEEstablished = NO;
    os_unfair_lock_unlock(&_stateLock);

    return stopResult;
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
    // Any terminal error path implies the watchdog (if armed) no longer needs
    // to fire.  Clear the waiting-after-PASE flag synchronously so that any
    // already-enqueued watchdog event_handler is suppressed by the late-fire
    // guard, then bounce the dispatch_source cancel itself onto _delegateQueue
    // (this method may be invoked on the Matter / CHIP thread).
    os_unfair_lock_lock(&_stateLock);
    _isWaitingAfterPASEEstablished = NO;
    os_unfair_lock_unlock(&_stateLock);
    dispatch_async(_delegateQueue, ^{
        [self _cancelPostPASEWatchdog];
    });

    MTRDeviceController_Concrete * strongController = _controller;

    MTR_LOG("%@ Device commissioning failed with controller %@ metrics %@", self, strongController, metrics);

    id<MTRCommissioningDelegate_Internal> strongDelegate = [self _internalDelegate];

    // Null out the delegate, so we don't notify based on any internal cleanups
    // we happen to do.
    _delegate = nil;

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

- (id<MTRCommissioningDelegate_Internal>)_internalDelegate
{
    return static_cast<id<MTRCommissioningDelegate_Internal>>(_delegate);
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
            // Arm the watchdog inside this block so that the arm and the
            // paired ivar write below are both serialized on _delegateQueue
            // with all subsequent _postPASEWatchdog mutations.  A client
            // that handles paseSessionEstablishmentComplete: but then
            // loses interest -- without ever calling commissionNodeWithID:
            // or cancelCommissioningForNodeID: -- would otherwise
            // permanently wedge the controller's commissioning state.
            BOOL armed = [self _armPostPASEWatchdog];
            if (!armed) {
                // Failing to arm the watchdog means we cannot bound the
                // post-PASE wait at all -- that is the very wedge this
                // logic exists to prevent.  Surface a no-memory error so
                // the client tears down rather than going into the
                // unbounded wait state.
                MTR_LOG_ERROR("%@ failed to arm post-PASE watchdog; aborting commissioning to avoid an unbounded wait", self);
                [self _dispatchCommissioningError:[MTRError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY]];
                return;
            }
            // We are already on _delegateQueue, so write the backing ivar
            // directly under _stateLock rather than going through the
            // setter (which would re-enter the cancel path on a NO->YES
            // transition only -- but using the lock here keeps the
            // reader/writer contract uniform).
            os_unfair_lock_lock(&self->_stateLock);
            self->_isWaitingAfterPASEEstablished = YES;
            os_unfair_lock_unlock(&self->_stateLock);
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

    // Defensive: by the time we land here on a success path the controller
    // should already have flipped isWaitingAfterPASEEstablished back to NO
    // (from commissionNodeWithID:) which fires the watchdog teardown via
    // the setter side effect.  But this method is the terminal success
    // notification for the operation, so don't depend on that upstream
    // invariant -- if a future code path ever reaches success without
    // having cleared the flag, an armed watchdog would otherwise survive
    // to fire after we have nilled out _delegate and finished commissioning,
    // taking the late-fire guard's "replaced" branch but still doing the
    // dispatch_source bookkeeping uselessly.  Clear the flag synchronously
    // and bounce the dispatch_source cancel itself onto _delegateQueue.
    os_unfair_lock_lock(&_stateLock);
    _isWaitingAfterPASEEstablished = NO;
    os_unfair_lock_unlock(&_stateLock);
    dispatch_async(_delegateQueue, ^{
        [self _cancelPostPASEWatchdog];
    });

    id<MTRCommissioningDelegate> strongDelegate = _delegate;
    MTRDeviceController_Concrete * strongController = _controller;

    // Null out the delegate, so we don't notify based on any internal cleanups
    // we happened to do.
    _delegate = nil;

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
    id<MTRCommissioningDelegate> strongDelegate = _delegate;
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
    id<MTRCommissioningDelegate> strongDelegate = _delegate;
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
    id<MTRCommissioningDelegate> strongDelegate = _delegate;
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
    id<MTRCommissioningDelegate> strongDelegate = _delegate;
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
    id<MTRCommissioningDelegate> strongDelegate = _delegate;
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

#pragma mark - PostPASEWatchdogTesting

@implementation MTRCommissioningOperation (PostPASEWatchdogTesting)

+ (void)setPostPASEWatchdogIntervalForTesting:(NSTimeInterval)interval
{
    // Negative, NaN, and non-finite values are nonsensical; clamp to 0
    // (== "use production interval").  Non-finite (INFINITY / NaN) inputs
    // would multiply with NSEC_PER_SEC and cast to int64_t in
    // -_armPostPASEWatchdog, which is undefined behavior when the result
    // is not representable as int64_t (UBSan trips, dispatch_time
    // misbehaves).  isfinite() rejects both ±INFINITY and NaN; the > 0
    // check still covers negatives and zero.  The override is read on
    // _delegateQueue when arming a new watchdog; tests are expected to
    // set this before kicking off the commissioning flow they want to
    // observe.  Stored atomically so concurrent setters on different
    // threads cannot race with the read in -_armPostPASEWatchdog under
    // TSan.
    NSTimeInterval clamped = (isfinite(interval) && interval > 0) ? interval : 0;
    atomic_store_explicit(&sMTRPostPASEWatchdogIntervalForTesting, clamped, memory_order_relaxed);
}

- (void)setForceNextArmFailureForTesting:(BOOL)force
{
    // _forceNextArmFailureForTesting is protected by _stateLock so we can
    // publish the write synchronously from any queue (including the test's
    // own queue, or _delegateQueue itself).  The previous implementation
    // bounced onto _delegateQueue via dispatch_async, which required every
    // test that wanted to observe the injected fault to manually flush
    // _delegateQueue (dispatch_sync(queue, ^{})) before exercising
    // _armPostPASEWatchdog -- a footgun a future test would inevitably
    // forget.
    os_unfair_lock_lock(&_stateLock);
    _forceNextArmFailureForTesting = force;
    os_unfair_lock_unlock(&_stateLock);
}

- (nullable dispatch_source_t)postPASEWatchdogForTesting
{
    // Read _postPASEWatchdog under _stateLock so tests observe a consistent
    // value immediately after -stop returns (the cancel in -stop is now
    // synchronous; tests assert this returns nil right after -stop).
    os_unfair_lock_lock(&_stateLock);
    dispatch_source_t source = _postPASEWatchdog;
    os_unfair_lock_unlock(&_stateLock);
    return source;
}

@end
